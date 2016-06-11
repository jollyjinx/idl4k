

#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/arp.h>
#include <net/ip.h>
#include <linux/kthread.h>
#include <linux/bpa2.h>
#include "ivo_eth.h"
#include "descs.h"

#define RTP_HEADER
//#define STMMAC_CHECKSUM_INSERTION

#define MAX_CLIENTS			32
#define MAX_ETH_ITEMS		(4*1024)
#define MAX_TO_PURGE		(1024)

#define DEF_PAYLOAD_SIZE	(7*188)

struct net_device *net_eth0;

static unsigned int _eth_item_idx;
static struct page* _client_header[MAX_CLIENTS];
static struct page* _eth_item[MAX_ETH_ITEMS];
static unsigned int _to_purge_item;
static struct page* _page_to_purge[MAX_TO_PURGE];

static struct page* tspage;
static u16 rtp_seq_no;
static u32 rtp_timestamp;
static u32 ssrc_id;

struct rtp
{
	u32 flags;
	u32 timestamp;
	u32 ssrc_id;
} __attribute__((packed));

struct netheader
{
	struct ethhdr ethh;
	struct iphdr  iph;
	struct udphdr udph;
#ifdef RTP_HEADER
	struct rtp    rtph;
#endif
} __attribute__((packed));


struct stats_t
{
	unsigned long num_packets;
	unsigned long e_nomem;
	unsigned long e_qstopped;
	unsigned long e_nomem1;
};


struct queue_item
{
    unsigned int    tim_tick_to_send;
    void*           pa_hdr;
    u32             hdr_size;
    void*           pa_payload;
    u32             payload_size;
};


unsigned int queue_head;
unsigned int queue_tail;
struct queue_item queue[MAX_ETH_ITEMS];


static unsigned int last_tim_tick_to_send;

struct stats_t stats;
struct task_struct* thread_ptr;

u32 _test_bitrate = 80000000;
static struct queue_item *_qitem;


extern struct netdev_queue *dev_pick_tx(struct net_device *dev, struct sk_buff *skb);
extern netdev_tx_t ivomac_xmit(void* pa_hdr, u32 hdr_size, void* pa_payload, u32 payload_size, u32 tdes0, struct net_device *dev);
netdev_tx_t ivomac_xmit_1s(void* pa_hdr, u32 hdr_size, void* pa_payload, u32 payload_size, struct net_device *dev);
netdev_tx_t ivomac_xmit_1mb(void* pa_hdr, u32 hdr_size, void* pa_payload, u32 payload_size, struct net_device *dev, unsigned int *timestamp);

static unsigned added_to_queue;
static unsigned from_queue_to_xmit;

#define DEFAULT_TIMER_FREQ 10000 /*Hz*/

#if !defined(CONFIG_STMMAC_TMU_TIMER)
#include <linux/sh_timer.h>

static struct sh_timer_callb *timer;
static unsigned int timer_tick;


static void ivo_eth_timer_irq(void *data)
{
	netdev_tx_t netdev_tx;
	struct queue_item* qitem;

	timer_tick += 1;

	if ( _qitem != NULL )
	{
		qitem = _qitem;
		ivomac_xmit(qitem->pa_hdr, qitem->hdr_size, qitem->pa_payload, qitem->payload_size, 0, net_eth0);
	}

	while ( queue_tail != queue_head )
	{
		qitem = &queue[queue_tail % MAX_ETH_ITEMS];
		if ( qitem->tim_tick_to_send > timer_tick )
			break;
		netdev_tx = ivomac_xmit(qitem->pa_hdr, qitem->hdr_size, qitem->pa_payload, qitem->payload_size, 0, net_eth0);
		if ( netdev_tx != NETDEV_TX_OK )
			break;
		from_queue_to_xmit++;
		queue_tail++;
	}
}

static void ivo_eth_timer_start(void)
{
	if ( timer == NULL )
	{
		timer = sh_timer_register(ivo_eth_timer_irq,  (void *)0);
		if ( timer == NULL )
		{
			printk(KERN_ERR "ivo_eth_timer_start: no available TMU timer, start failed!\n");
			return;
		}
	}

	printk(KERN_NOTICE "ivo_eth_timer_start: settings TMU timer at %d Hz\n", DEFAULT_TIMER_FREQ);

	timer->set_rate(timer->tmu_priv, DEFAULT_TIMER_FREQ);
	timer->timer_start(timer->tmu_priv);
}

static void ivo_eth_timer_stop(void)
{
	if (timer != NULL)
	{
		timer->timer_stop(timer->tmu_priv);
		sh_timer_unregister(timer->tmu_priv);
		timer = NULL;
	}
}
#endif


#ifndef STMMAC_CHECKSUM_INSERTION
// Function for checksum calculation. From the RFC,
// the checksum algorithm is:
//  "The checksum field is the 16 bit one's complement of the one's
//  complement sum of all 16 bit words in the header.  For purposes of
//  computing the checksum, the value of the checksum field is zero."
static unsigned short csum(unsigned short *buf, int nwords)
{   	//
	unsigned long sum;
	for(sum=0; nwords>0; nwords--)
		sum += *buf++;
	sum = (sum >> 16) + (sum &0xffff);
	sum += (sum >> 16);
	return (unsigned short)(~sum);
}

static inline void add_checksum(struct netheader *nh, void *payload, unsigned int payload_size)
{
  int nwords;
  unsigned long sum = 0;
  unsigned short *payload_p = payload;

  if (nh)
  {
    sum += (nh->iph.saddr & 0xffff);
    sum += (nh->iph.saddr >> 16);
    sum += (nh->iph.daddr & 0xffff);
    sum += (nh->iph.daddr >> 16);
    sum += (nh->iph.protocol << 8);
    sum += nh->udph.len;
    sum += nh->udph.source;
    sum += nh->udph.dest;
    sum += nh->udph.len;

#ifdef RTP_HEADER
    sum += (nh->rtph.flags & 0xffff);
    sum += (nh->rtph.flags >> 16);
    sum += (nh->rtph.timestamp & 0xffff);
    sum += (nh->rtph.timestamp >> 16);
    sum += (nh->rtph.ssrc_id & 0xffff);
    sum += (nh->rtph.ssrc_id >> 16);
#endif

    nwords = ((payload_size + 1) & ~1) / sizeof(unsigned short);
    for (; nwords > 0; nwords--) {
      sum += *payload_p++;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    nh->udph.check = (unsigned short)(~sum);
    if (nh->udph.check == 0) {
      nh->udph.check = ~0;
    }
  }
}
#endif

static void clean_up_pages(struct page** pages, u32 num_pages)
{
	while ( num_pages )
	{
		struct page* page_p = pages[--num_pages];
		mb();
		if (page_p != NULL)
		{
			pages[num_pages] = NULL;
			put_page(page_p);
			__free_page(page_p);
		}
	}
}

extern struct rtp *get_rtp_header(unsigned int client_id)
{
#ifdef RTP_HEADER
  struct page* eth_hdr;
  struct netheader* nh;

  if ( client_id < MAX_CLIENTS && (eth_hdr = _client_header[client_id]) != NULL )
  {
    nh = (struct netheader *)page_address(eth_hdr);
    return &nh->rtph;
  }
#endif

  return 0;
}

EXPORT_SYMBOL(get_rtp_header);

extern netdev_tx_t xmit_payload( unsigned int client_id, void* payload, unsigned int payload_size )
{
	netdev_tx_t netdev_tx = NETDEV_TX_BUSY;
	const struct net_device_ops* ops;
	struct page* eth_hdr;

	BUG_ON(net_eth0 == 0);
	//BUG_ON(payload_size == 0);

	if ( client_id < MAX_CLIENTS && (eth_hdr = _client_header[client_id]) != NULL )
	{
		struct page *eth_item;
		unsigned char* va_eth_hdr;
		unsigned char* va_eth_item;
		unsigned char* pa_eth_item;
		struct netheader* nh;

		if (netif_queue_stopped(net_eth0))
		{
			stats.e_qstopped++;
			return NETDEV_TX_BUSY;
		}

		ops = net_eth0->netdev_ops;
		BUG_ON(ops->ndo_start_xmit == NULL);

		/*if(unlikely(ops->ndo_start_xmit_ivo == NULL))
		{
			return NETDEV_TX_LOCKED;
		}*/

		eth_item = _eth_item[_eth_item_idx++ % MAX_ETH_ITEMS];
		BUG_ON(eth_item == 0);
		va_eth_hdr = page_address(eth_hdr);
		va_eth_item = page_address(eth_item);
		//memcpy(va_eth_item, va_eth_hdr, 54); // Copy ETH,IPH,UDP,RTP
		*((u32*)(va_eth_item)+0x00) = *((u32*)(va_eth_hdr)+0x00);
		*((u32*)(va_eth_item)+0x01) = *((u32*)(va_eth_hdr)+0x01);
		*((u32*)(va_eth_item)+0x02) = *((u32*)(va_eth_hdr)+0x02);
		*((u32*)(va_eth_item)+0x03) = *((u32*)(va_eth_hdr)+0x03);
		*((u32*)(va_eth_item)+0x04) = *((u32*)(va_eth_hdr)+0x04);
		*((u32*)(va_eth_item)+0x05) = *((u32*)(va_eth_hdr)+0x05);
		*((u32*)(va_eth_item)+0x06) = *((u32*)(va_eth_hdr)+0x06);
		*((u32*)(va_eth_item)+0x07) = *((u32*)(va_eth_hdr)+0x07);
		*((u32*)(va_eth_item)+0x08) = *((u32*)(va_eth_hdr)+0x08);
		*((u32*)(va_eth_item)+0x09) = *((u32*)(va_eth_hdr)+0x09);
		*((u32*)(va_eth_item)+0x0A) = *((u32*)(va_eth_hdr)+0x0A);
#ifdef RTP_HEADER
		*((u32*)(va_eth_item)+0x0B) = *((u32*)(va_eth_hdr)+0x0B);
		*((u32*)(va_eth_item)+0x0C) = *((u32*)(va_eth_hdr)+0x0C);
		*((u32*)(va_eth_item)+0x0D) = *((u32*)(va_eth_hdr)+0x0D);
#endif

		// Prepare RTP part
		nh = (struct netheader*)va_eth_item;

    if (payload_size != DEF_PAYLOAD_SIZE)
    {
#ifdef RTP_HEADER
      nh->iph.tot_len = htons(sizeof(nh->iph) + sizeof(nh->udph) + sizeof(nh->rtph) + payload_size);
      nh->udph.len = htons(sizeof(nh->udph) + sizeof(nh->rtph) + payload_size);
#else
      nh->iph.tot_len = htons(sizeof(nh->iph) + sizeof(nh->udph) + payload_size);
      nh->udph.len = htons(sizeof(nh->udph) + payload_size);
#endif
      nh->iph.check = 0;
#ifndef STMMAC_CHECKSUM_INSERTION
      nh->iph.check = csum((unsigned short*)&nh->iph, sizeof(nh->iph) / sizeof(unsigned short));
#endif
    }

#ifdef RTP_HEADER
    nh->rtph.flags |= htonl((1<<31)|((33&0x7f)<<16));
#endif

#ifdef STMMAC_CHECKSUM_INSERTION
    nh->udph.check = 0;
#else
    add_checksum(nh, payload, payload_size);
#endif

		/*CONFIG_CACHE_WRITEBACK=y
		# CONFIG_CACHE_WRITETHROUGH is not set
		# CONFIG_CACHE_OFF is not set
		CONFIG_STM_L2_CACHE=y
		# CONFIG_STM_L2_CACHE_BYPASSED is not set
		# CONFIG_STM_L2_CACHE_WRITETHROUGH is not set
		CONFIG_STM_L2_CACHE_WRITEBACK=y*/

		pa_eth_item = (unsigned char*)virt_to_phys(va_eth_item);
		wmb();
#if 1
		// Writeback and Invalidate Cache lines.
		//dma_cache_sync(NULL, va_eth_item, sizeof(struct netheader), DMA_BIDIRECTIONAL);
		__ocbp(va_eth_item);
		__ocbp(va_eth_item + L1_CACHE_BYTES);
#if defined(CONFIG_STM_L2_CACHE) && !defined(CONFIG_STM_L2_CACHE_BYPASSED)
		stm_l2_flush_purge((unsigned long)pa_eth_item, sizeof(struct netheader), 1);
#endif
#else
		// Writeback Cache lines.
		//dma_cache_sync(NULL, va_eth_item, sizeof(struct netheader), DMA_TO_DEVICE);
		__ocbwb(va_eth_item);
		__ocbwb(va_eth_item + L1_CACHE_BYTES);
#if defined(CONFIG_STM_L2_CACHE) && !defined(CONFIG_STM_L2_CACHE_BYPASSED)
		stm_l2_flush_wback((unsigned long)pa_eth_item, sizeof(struct netheader), 1);
#endif
#endif

		//txq = dev_pick_tx(net_eth0, skb);
		//printk("stmmac_ivo_xmit skb %p	<===========\n", skb);
		//HARD_TX_LOCK(net_eth0, txq, smp_processor_id());
		//netdev_tx = ops->ndo_start_xmit_ivo((void*)virt_to_phys(va_eth_item), 54, (void*)virt_to_phys(payload), payload_size, net_eth0);
		netdev_tx = ivomac_xmit((void*)virt_to_phys(va_eth_item), sizeof(struct netheader), (void*)virt_to_phys(payload), payload_size, 0, net_eth0);
		//if (netdev_tx == NETDEV_TX_OK)
		//	txq_trans_update(txq);
		//HARD_TX_UNLOCK(net_eth0, txq);
	}

	return netdev_tx;
}

EXPORT_SYMBOL(xmit_payload);

static int queue_xmit_payload( unsigned int client_id, void* payload, unsigned int payload_size, struct queue_item* qitem )
{
	const struct net_device_ops* ops;
	struct page* eth_hdr;

	BUG_ON(net_eth0 == 0);
	BUG_ON(payload_size == 0);

	if ( client_id < MAX_CLIENTS && (eth_hdr = _client_header[client_id]) != NULL )
	{
		struct page *eth_item;
		unsigned char* va_eth_hdr;
		unsigned char* va_eth_item;
		unsigned char* pa_eth_item;
		struct netheader* nh;

		if (netif_queue_stopped(net_eth0))
		{
			stats.e_qstopped++;
			return NETDEV_TX_BUSY;
		}

		ops = net_eth0->netdev_ops;
		BUG_ON(ops->ndo_start_xmit == NULL);

		/*if(unlikely(ops->ndo_start_xmit_ivo == NULL))
		{
			return NETDEV_TX_LOCKED;
		}*/

		eth_item = _eth_item[_eth_item_idx++ % MAX_ETH_ITEMS];
		BUG_ON(eth_item == 0);
		va_eth_hdr = page_address(eth_hdr);
		va_eth_item = page_address(eth_item);
		//memcpy(va_eth_item, va_eth_hdr, 54); // Copy ETH,IPH,UDP,RTP
		*((u32*)(va_eth_item)+0x00) = *((u32*)(va_eth_hdr)+0x00);
		*((u32*)(va_eth_item)+0x01) = *((u32*)(va_eth_hdr)+0x01);
		*((u32*)(va_eth_item)+0x02) = *((u32*)(va_eth_hdr)+0x02);
		*((u32*)(va_eth_item)+0x03) = *((u32*)(va_eth_hdr)+0x03);
		*((u32*)(va_eth_item)+0x04) = *((u32*)(va_eth_hdr)+0x04);
		*((u32*)(va_eth_item)+0x05) = *((u32*)(va_eth_hdr)+0x05);
		*((u32*)(va_eth_item)+0x06) = *((u32*)(va_eth_hdr)+0x06);
		*((u32*)(va_eth_item)+0x07) = *((u32*)(va_eth_hdr)+0x07);
		*((u32*)(va_eth_item)+0x08) = *((u32*)(va_eth_hdr)+0x08);
		*((u32*)(va_eth_item)+0x09) = *((u32*)(va_eth_hdr)+0x09);
		*((u32*)(va_eth_item)+0x0A) = *((u32*)(va_eth_hdr)+0x0A);
		*((u32*)(va_eth_item)+0x0B) = *((u32*)(va_eth_hdr)+0x0B);
		*((u32*)(va_eth_item)+0x0C) = *((u32*)(va_eth_hdr)+0x0C);
		*((u32*)(va_eth_item)+0x0D) = *((u32*)(va_eth_hdr)+0x0D);

		// Prepare RTP part
		nh = (struct netheader*)va_eth_item;

    if (payload_size != DEF_PAYLOAD_SIZE)
    {
#ifdef RTP_HEADER
      nh->iph.tot_len = htons(sizeof(nh->iph) + sizeof(nh->udph) + sizeof(nh->rtph) + payload_size);
      nh->udph.len = htons(sizeof(nh->udph) + sizeof(nh->rtph) + payload_size);
#else
      nh->iph.tot_len = htons(sizeof(nh->iph) + sizeof(nh->udph) + payload_size);
      nh->udph.len = htons(sizeof(nh->udph) + payload_size);
#endif
#ifdef STMMAC_CHECKSUM_INSERTION
      nh->iph.check = 0;
#else
      nh->iph.check = csum((unsigned short*)&nh->iph, sizeof(nh->iph) / sizeof(unsigned short));
#endif
    }

#ifdef RTP_HEADER
    nh->rtph.flags = htonl((1<<31)|++rtp_seq_no);
    nh->rtph.timestamp = htonl(rtp_timestamp++);
    nh->rtph.ssrc_id = htonl((unsigned)&ssrc_id);
#endif

#ifdef STMMAC_CHECKSUM_INSERTION
    nh->udph.check = 0;
#else
    add_checksum(nh, payload, payload_size);
#endif

		/*CONFIG_CACHE_WRITEBACK=y
		# CONFIG_CACHE_WRITETHROUGH is not set
		# CONFIG_CACHE_OFF is not set
		CONFIG_STM_L2_CACHE=y
		# CONFIG_STM_L2_CACHE_BYPASSED is not set
		# CONFIG_STM_L2_CACHE_WRITETHROUGH is not set
		CONFIG_STM_L2_CACHE_WRITEBACK=y*/

		pa_eth_item = (unsigned char*)virt_to_phys(va_eth_item);
		wmb();
#if 1
		// Writeback and Invalidate Cache lines.
		//dma_cache_sync(NULL, va_eth_item, sizeof(struct netheader), DMA_BIDIRECTIONAL);
		__ocbp(va_eth_item);
		__ocbp(va_eth_item + L1_CACHE_BYTES);
#if defined(CONFIG_STM_L2_CACHE) && !defined(CONFIG_STM_L2_CACHE_BYPASSED)
		stm_l2_flush_purge((unsigned long)pa_eth_item, sizeof(struct netheader), 1);
#endif
#else
		// Writeback Cache lines.
		//dma_cache_sync(NULL, va_eth_item, sizeof(struct netheader), DMA_TO_DEVICE);
		__ocbwb(va_eth_item);
		__ocbwb(va_eth_item + L1_CACHE_BYTES);
#if defined(CONFIG_STM_L2_CACHE) && !defined(CONFIG_STM_L2_CACHE_BYPASSED)
		stm_l2_flush_wback((unsigned long)pa_eth_item, sizeof(struct netheader), 1);
#endif
#endif

		qitem->pa_hdr = (void*)virt_to_phys(va_eth_item);
		qitem->hdr_size = sizeof(struct netheader);
		qitem->pa_payload = (void*)virt_to_phys(payload);
		qitem->payload_size = payload_size;
		return 0;
	}

	return -1;
}


static int _ivo_eth_thread( void* param )
{
	struct timeval tvs, tve;
	int res = 0, i;
	unsigned id;
	unsigned not_handled_clients;

	memset(&stats, 0, sizeof(stats));
	do_gettimeofday(&tvs);

	id = -1;
	not_handled_clients = 0;
	for ( ; ; )
	{
		struct page* page_p;
		unsigned long elapsed;
		if ( kthread_should_stop() || res < 0 )
			break;

		/*do_gettimeofday(&tve);
		elapsed = (tve.tv_sec - tvs.tv_sec) * 1000 + (tve.tv_usec - tvs.tv_usec) / 1000;
		if (unlikely(elapsed >= 10000))
			break;*/

		id = (id + 1) % MAX_CLIENTS;

		page_p = _client_header[id];
		if (page_p == NULL)
		{
			if (++not_handled_clients == MAX_CLIENTS)
				schedule();
			else if (not_handled_clients >= 2*MAX_CLIENTS)
			{
				not_handled_clients = 0;
				msleep(1);
			}
			continue;
		}
		not_handled_clients = 0;

		i = 0;
		while (_client_header[id] != NULL)
		{
			if (xmit_payload(id, ((u8*)page_address(tspage))+i*DEF_PAYLOAD_SIZE, DEF_PAYLOAD_SIZE) != NETDEV_TX_OK)
			{
				msleep(1);
				break;
			}
			i = (i + 1) % 3;
			/*if ( i == 2 )
			{
				do_gettimeofday(&tve);
				elapsed = (tve.tv_sec - tvs.tv_sec) * 1000 + (tve.tv_usec - tvs.tv_usec) / 1000;
				if (unlikely(elapsed >= 10000))
					goto time_elapsed;
			}*/
		}
	}

time_elapsed:
	if ( res )
		res = -ERESTARTSYS;

	printk( "@@@@@@ [ivo_udp_thread] finished thread..., res=%d\n", res );
	printk("stats: num_packets %lu, e_nomem %lu, e_nomem1 %lu, e_qstopped %lu\n", stats.num_packets, stats.e_nomem, stats.e_nomem1, stats.e_qstopped);
	thread_ptr = 0;
	return res;
}


static int _ivo_eth_thread1( void* param )
{
	struct timeval tvs, tve;
	int res = 0, i, j=0;
	unsigned id;
	unsigned not_handled_clients;
	unsigned eth_frames_per_second = _test_bitrate / (DEF_PAYLOAD_SIZE * 8);
	unsigned eth_frames_period_m100 = (DEFAULT_TIMER_FREQ * 100) / eth_frames_per_second;
	unsigned eth_frames_generated = 0;
	unsigned int base_timer_tick;

	printk("_ivo_eth_thread1 started: _test_bitrate %u eth_frames_per_second %u eth_frames_period_m100 %u base_timer_tick %u\n",_test_bitrate,eth_frames_per_second,eth_frames_period_m100,base_timer_tick);

	added_to_queue = 0;
	from_queue_to_xmit = 0;

	memset(&stats, 0, sizeof(stats));
	do_gettimeofday(&tvs);

	last_tim_tick_to_send = timer_tick + DEFAULT_TIMER_FREQ; // Delay first transmission by 1 second.

	id = -1;
	not_handled_clients = 0;
	for ( ; ; )
	{
		struct page* page_p;
		unsigned long elapsed;
		if ( kthread_should_stop() || res < 0 )
			break;

		if ( eth_frames_generated >= eth_frames_per_second * 10 )
			break;

		/*do_gettimeofday(&tve);
		elapsed = (tve.tv_sec - tvs.tv_sec) * 1000 + (tve.tv_usec - tvs.tv_usec) / 1000;
		if (unlikely(elapsed >= 10000))
			break;*/

		id = (id + 1) % MAX_CLIENTS;

		page_p = _client_header[id];
		if (page_p == NULL)
		{
			if (++not_handled_clients == MAX_CLIENTS)
				schedule();
			else if (not_handled_clients >= 2*MAX_CLIENTS)
			{
				not_handled_clients = 0;
				msleep(1);
			}
			continue;
		}
		not_handled_clients = 0;

		i = 0;
        base_timer_tick = (last_tim_tick_to_send > timer_tick ? last_tim_tick_to_send : timer_tick);
		while (_client_header[id] != NULL)
		{
			if ( ((queue_head + 1) ^ queue_tail) % MAX_ETH_ITEMS )
			{
				struct queue_item* qitem = &queue[queue_head % MAX_ETH_ITEMS];
				if ( !queue_xmit_payload(id, ((u8*)page_address(tspage))+(j++%3)*DEF_PAYLOAD_SIZE, DEF_PAYLOAD_SIZE, qitem) )
				{
					last_tim_tick_to_send = qitem->tim_tick_to_send = base_timer_tick + (eth_frames_period_m100 * ++i) / 100;
					//printk("%u ",qitem->tim_tick_to_send);
					queue_head++;
					added_to_queue++;
					eth_frames_generated++;
				}
			}
			else
			{
				msleep(1);
				break;
			}

			if ( eth_frames_generated >= eth_frames_per_second * 10 )
				break;
		}
	}

	printk("\n_ivo_eth_thread1 end: eth_frames_generated %u\n",eth_frames_generated);

time_elapsed:
	if ( res )
		res = -ERESTARTSYS;

	printk( "@@@@@@ [ivo_udp_thread] finished thread..., res=%d\n", res );
	printk("stats: num_packets %lu, e_nomem %lu, e_nomem1 %lu, e_qstopped %lu\n", stats.num_packets, stats.e_nomem, stats.e_nomem1, stats.e_qstopped);
	thread_ptr = 0;
	return res;
}


static netdev_tx_t xmit_1sec( unsigned int client_id, void* payload, unsigned int payload_size )
{
	netdev_tx_t netdev_tx = NETDEV_TX_BUSY;
	const struct net_device_ops* ops;
	struct page* eth_hdr;

	BUG_ON(net_eth0 == 0);
	BUG_ON(payload_size == 0);

	if ( client_id < MAX_CLIENTS && (eth_hdr = _client_header[client_id]) != NULL )
	{
		struct page *eth_item;
		unsigned char* va_eth_hdr;
		unsigned char* va_eth_item;
		unsigned char* pa_eth_item;
		struct netheader* nh;

		if (netif_queue_stopped(net_eth0))
		{
			stats.e_qstopped++;
			return NETDEV_TX_BUSY;
		}

		ops = net_eth0->netdev_ops;
		BUG_ON(ops->ndo_start_xmit == NULL);

		/*if(unlikely(ops->ndo_start_xmit_ivo == NULL))
		{
			return NETDEV_TX_LOCKED;
		}*/

		eth_item = _eth_item[_eth_item_idx++ % MAX_ETH_ITEMS];
		BUG_ON(eth_item == 0);
		va_eth_hdr = page_address(eth_hdr);
		va_eth_item = page_address(eth_item);
		//memcpy(va_eth_item, va_eth_hdr, 54); // Copy ETH,IPH,UDP,RTP
		*((u32*)(va_eth_item)+0x00) = *((u32*)(va_eth_hdr)+0x00);
		*((u32*)(va_eth_item)+0x01) = *((u32*)(va_eth_hdr)+0x01);
		*((u32*)(va_eth_item)+0x02) = *((u32*)(va_eth_hdr)+0x02);
		*((u32*)(va_eth_item)+0x03) = *((u32*)(va_eth_hdr)+0x03);
		*((u32*)(va_eth_item)+0x04) = *((u32*)(va_eth_hdr)+0x04);
		*((u32*)(va_eth_item)+0x05) = *((u32*)(va_eth_hdr)+0x05);
		*((u32*)(va_eth_item)+0x06) = *((u32*)(va_eth_hdr)+0x06);
		*((u32*)(va_eth_item)+0x07) = *((u32*)(va_eth_hdr)+0x07);
		*((u32*)(va_eth_item)+0x08) = *((u32*)(va_eth_hdr)+0x08);
		*((u32*)(va_eth_item)+0x09) = *((u32*)(va_eth_hdr)+0x09);
		*((u32*)(va_eth_item)+0x0A) = *((u32*)(va_eth_hdr)+0x0A);
		*((u32*)(va_eth_item)+0x0B) = *((u32*)(va_eth_hdr)+0x0B);
		*((u32*)(va_eth_item)+0x0C) = *((u32*)(va_eth_hdr)+0x0C);
		*((u32*)(va_eth_item)+0x0D) = *((u32*)(va_eth_hdr)+0x0D);

		// Prepare RTP part
		nh = (struct netheader*)va_eth_item;

    if (payload_size != DEF_PAYLOAD_SIZE)
    {
#ifdef RTP_HEADER
      nh->iph.tot_len = htons(sizeof(nh->iph) + sizeof(nh->udph) + sizeof(nh->rtph) + payload_size);
      nh->udph.len = htons(sizeof(nh->udph) + sizeof(nh->rtph) + payload_size);
#else
      nh->iph.tot_len = htons(sizeof(nh->iph) + sizeof(nh->udph) + payload_size);
      nh->udph.len = htons(sizeof(nh->udph) + payload_size);
#endif
#ifdef STMMAC_CHECKSUM_INSERTION
      nh->iph.check = 0;
#else
      nh->iph.check = csum((unsigned short*)&nh->iph, sizeof(nh->iph) / sizeof(unsigned short));
#endif
    }

#ifdef RTP_HEADER
    nh->rtph.flags = htonl((1<<31)|++rtp_seq_no);
    nh->rtph.timestamp = htonl(rtp_timestamp++);
    nh->rtph.ssrc_id = htonl((unsigned)&ssrc_id);
#endif

#ifdef STMMAC_CHECKSUM_INSERTION
    nh->udph.check = 0;
#else
    add_checksum(nh, payload, payload_size);
#endif

		/*CONFIG_CACHE_WRITEBACK=y
		# CONFIG_CACHE_WRITETHROUGH is not set
		# CONFIG_CACHE_OFF is not set
		CONFIG_STM_L2_CACHE=y
		# CONFIG_STM_L2_CACHE_BYPASSED is not set
		# CONFIG_STM_L2_CACHE_WRITETHROUGH is not set
		CONFIG_STM_L2_CACHE_WRITEBACK=y*/

		pa_eth_item = (unsigned char*)virt_to_phys(va_eth_item);
		wmb();
#if 1
		// Writeback and Invalidate Cache lines.
		//dma_cache_sync(NULL, va_eth_item, sizeof(struct netheader), DMA_BIDIRECTIONAL);
		__ocbp(va_eth_item);
		__ocbp(va_eth_item + L1_CACHE_BYTES);
#if defined(CONFIG_STM_L2_CACHE) && !defined(CONFIG_STM_L2_CACHE_BYPASSED)
		stm_l2_flush_purge((unsigned long)pa_eth_item, sizeof(struct netheader), 1);
#endif
#else
		// Writeback Cache lines.
		//dma_cache_sync(NULL, va_eth_item, sizeof(struct netheader), DMA_TO_DEVICE);
		__ocbwb(va_eth_item);
		__ocbwb(va_eth_item + L1_CACHE_BYTES);
#if defined(CONFIG_STM_L2_CACHE) && !defined(CONFIG_STM_L2_CACHE_BYPASSED)
		stm_l2_flush_wback((unsigned long)pa_eth_item, sizeof(struct netheader), 1);
#endif
#endif

		//txq = dev_pick_tx(net_eth0, skb);
		//printk("stmmac_ivo_xmit skb %p	<===========\n", skb);
		//HARD_TX_LOCK(net_eth0, txq, smp_processor_id());
		//netdev_tx = ops->ndo_start_xmit_ivo((void*)virt_to_phys(va_eth_item), 54, (void*)virt_to_phys(payload), payload_size, net_eth0);
		netdev_tx = ivomac_xmit_1s((void*)virt_to_phys(va_eth_item), sizeof(struct netheader), (void*)virt_to_phys(payload), payload_size, net_eth0);
		//if (netdev_tx == NETDEV_TX_OK)
		//	txq_trans_update(txq);
		//HARD_TX_UNLOCK(net_eth0, txq);
	}

	return netdev_tx;
}


static netdev_tx_t xmit_1mb( unsigned int client_id, void* payload, unsigned int payload_size )
{
  netdev_tx_t netdev_tx = NETDEV_TX_BUSY;
  const struct net_device_ops* ops;
  struct page* eth_hdr;

  BUG_ON(net_eth0 == 0);
  BUG_ON(payload_size == 0);

  if ( client_id < MAX_CLIENTS && (eth_hdr = _client_header[client_id]) != NULL )
  {
    struct page *eth_item;
    unsigned char* va_eth_hdr;
    unsigned char* va_eth_item;
    unsigned char* pa_eth_item;
    struct netheader* nh;

    if (netif_queue_stopped(net_eth0))
    {
      stats.e_qstopped++;
      return NETDEV_TX_BUSY;
    }

    ops = net_eth0->netdev_ops;
    BUG_ON(ops->ndo_start_xmit == NULL);

    /*if(unlikely(ops->ndo_start_xmit_ivo == NULL))
    {
      return NETDEV_TX_LOCKED;
    }*/

    eth_item = _eth_item[_eth_item_idx++ % MAX_ETH_ITEMS];
    BUG_ON(eth_item == 0);
    va_eth_hdr = page_address(eth_hdr);
    va_eth_item = page_address(eth_item);
    //memcpy(va_eth_item, va_eth_hdr, 54); // Copy ETH,IPH,UDP,RTP
    *((u32*)(va_eth_item)+0x00) = *((u32*)(va_eth_hdr)+0x00);
    *((u32*)(va_eth_item)+0x01) = *((u32*)(va_eth_hdr)+0x01);
    *((u32*)(va_eth_item)+0x02) = *((u32*)(va_eth_hdr)+0x02);
    *((u32*)(va_eth_item)+0x03) = *((u32*)(va_eth_hdr)+0x03);
    *((u32*)(va_eth_item)+0x04) = *((u32*)(va_eth_hdr)+0x04);
    *((u32*)(va_eth_item)+0x05) = *((u32*)(va_eth_hdr)+0x05);
    *((u32*)(va_eth_item)+0x06) = *((u32*)(va_eth_hdr)+0x06);
    *((u32*)(va_eth_item)+0x07) = *((u32*)(va_eth_hdr)+0x07);
    *((u32*)(va_eth_item)+0x08) = *((u32*)(va_eth_hdr)+0x08);
    *((u32*)(va_eth_item)+0x09) = *((u32*)(va_eth_hdr)+0x09);
    *((u32*)(va_eth_item)+0x0A) = *((u32*)(va_eth_hdr)+0x0A);
    *((u32*)(va_eth_item)+0x0B) = *((u32*)(va_eth_hdr)+0x0B);
    *((u32*)(va_eth_item)+0x0C) = *((u32*)(va_eth_hdr)+0x0C);
    *((u32*)(va_eth_item)+0x0D) = *((u32*)(va_eth_hdr)+0x0D);

    // Prepare RTP part
    nh = (struct netheader*)va_eth_item;

    if (payload_size != DEF_PAYLOAD_SIZE)
    {
#ifdef RTP_HEADER
      nh->iph.tot_len = htons(sizeof(nh->iph) + sizeof(nh->udph) + sizeof(nh->rtph) + payload_size);
      nh->udph.len = htons(sizeof(nh->udph) + sizeof(nh->rtph) + payload_size);
#else
      nh->iph.tot_len = htons(sizeof(nh->iph) + sizeof(nh->udph) + payload_size);
      nh->udph.len = htons(sizeof(nh->udph) + payload_size);
#endif
#ifdef STMMAC_CHECKSUM_INSERTION
      nh->iph.check = 0;
#else
      nh->iph.check = csum((unsigned short*)&nh->iph, sizeof(nh->iph) / sizeof(unsigned short));
#endif
    }

#ifdef RTP_HEADER
    nh->rtph.flags = htonl((1<<31)|++rtp_seq_no);
    nh->rtph.timestamp = htonl(rtp_timestamp++);
    nh->rtph.ssrc_id = htonl((unsigned)&ssrc_id);
#endif

#ifdef STMMAC_CHECKSUM_INSERTION
    nh->udph.check = 0;
#else
    add_checksum(nh, payload, payload_size);
#endif

    /*CONFIG_CACHE_WRITEBACK=y
    # CONFIG_CACHE_WRITETHROUGH is not set
    # CONFIG_CACHE_OFF is not set
    CONFIG_STM_L2_CACHE=y
    # CONFIG_STM_L2_CACHE_BYPASSED is not set
    # CONFIG_STM_L2_CACHE_WRITETHROUGH is not set
    CONFIG_STM_L2_CACHE_WRITEBACK=y*/

    pa_eth_item = (unsigned char*)virt_to_phys(va_eth_item);
    wmb();
#if 1
    // Writeback and Invalidate Cache lines.
    //dma_cache_sync(NULL, va_eth_item, sizeof(struct netheader), DMA_BIDIRECTIONAL);
    __ocbp(va_eth_item);
    __ocbp(va_eth_item + L1_CACHE_BYTES);
#if defined(CONFIG_STM_L2_CACHE) && !defined(CONFIG_STM_L2_CACHE_BYPASSED)
    stm_l2_flush_purge((unsigned long)pa_eth_item, sizeof(struct netheader), 1);
#endif
#else
    // Writeback Cache lines.
    //dma_cache_sync(NULL, va_eth_item, sizeof(struct netheader), DMA_TO_DEVICE);
    __ocbwb(va_eth_item);
    __ocbwb(va_eth_item + L1_CACHE_BYTES);
#if defined(CONFIG_STM_L2_CACHE) && !defined(CONFIG_STM_L2_CACHE_BYPASSED)
    stm_l2_flush_wback((unsigned long)pa_eth_item, sizeof(struct netheader), 1);
#endif
#endif

    //txq = dev_pick_tx(net_eth0, skb);
    //printk("stmmac_ivo_xmit skb %p  <===========\n", skb);
    //HARD_TX_LOCK(net_eth0, txq, smp_processor_id());
    //netdev_tx = ops->ndo_start_xmit_ivo((void*)virt_to_phys(va_eth_item), 54, (void*)virt_to_phys(payload), payload_size, net_eth0);
    netdev_tx = ivomac_xmit_1mb((void*)virt_to_phys(va_eth_item), sizeof(struct netheader), (void*)virt_to_phys(payload), payload_size, net_eth0, 0);
    //if (netdev_tx == NETDEV_TX_OK)
    //  txq_trans_update(txq);
    //HARD_TX_UNLOCK(net_eth0, txq);
  }

  return netdev_tx;
}


static int _start_thread(int test_thread_no)
{
	if (thread_ptr)
		printk("Thread already started!!!\n");
	else
	{
		switch (test_thread_no)
		{
		default:
			thread_ptr = kthread_run(_ivo_eth_thread, 0, "ivo_udp_%d", 0);
			break;
		case 1:
			thread_ptr = kthread_run(_ivo_eth_thread1, 0, "ivo_udp_%d", 0);
			break;
		}
        
		if ( IS_ERR( thread_ptr ) )
		{
			int res = PTR_ERR( thread_ptr );
			printk( "[fe] #error# failed to start kthread (%d)\n", res );
			return res;
		}
	}

	return 0;
}

static void _stop_thread(void)
{
	if (thread_ptr != 0)
	{
		printk("%s\n",__FUNCTION__);
		kthread_stop( thread_ptr );
		thread_ptr = 0;
	}
}


static struct proc_dir_entry* _proc_ptr;
static int _read_proc( char* page, char** start, off_t off, int count, int* eof, void* data_unused )
{
	return 0;
}


static int _write_proc_ivo( struct file* file, const char __user* buffer, unsigned long count, void* data )
{
	char writebuf[32];

	if (count >= sizeof(writebuf))
		count = sizeof(writebuf) - 1;
	if (copy_from_user(writebuf, buffer, count))
		return -EFAULT;
	writebuf[count + 1] = 0; // End of string.

	BUG_ON(net_eth0 == 0);

	if (!tspage)
	{
		unsigned char *va_page;
		int i = 0;

		tspage = alloc_page(GFP_KERNEL|GFP_DMA);
		if (!tspage)
		{
			//ret = -ENOMEM;
			printk("%s: couldn't allocate TS memory page\n",__FUNCTION__);
			return count;
		}
		va_page = (unsigned char*)page_address(tspage);
		do
		{
			memset(va_page + i * 188, i + 1, 188);
			*(va_page + i * 188) = 0x47;
			*(va_page + i * 188 + 187) = ~0x47;
		}
		while (++i < PAGE_SIZE / 188);
		//flush_dcache_page(tspage);
		dma_cache_sync(NULL, va_page, PAGE_SIZE, DMA_BIDIRECTIONAL); // Sync cache.
	}

	if (!strncmp(writebuf, "do_one_tx", 9))
	{
		if (tspage)
		{
			u32 client_id = 0;
			for ( ;client_id < MAX_CLIENTS; ++client_id)
				if (_client_header[client_id])
				{
					xmit_payload(client_id, page_address(tspage), DEF_PAYLOAD_SIZE);
					break;
				}
		}
	}
	else if (!strncmp(writebuf, "start_test_thr", sizeof("start_test_thr")-1))
	{
		_start_thread(0);
	}
	else if (!strncmp(writebuf, "st_test_thr1", sizeof("st_test_thr1")-1))
	{
		u32 bitrate = 0;
		sscanf( writebuf + sizeof("st_test_thr1")-1, "%u", &bitrate );
		if ( bitrate > 0 && bitrate < 800000000 )
		{
			_test_bitrate = bitrate;
		}
		_start_thread(1);
	}
	else if (!strncmp(writebuf, "stop_test_thr", sizeof("stop_test_thr")-1))
	{
		_stop_thread();
	}
	else if (!strncmp(writebuf, "test_1s_burst", sizeof("test_1s_burst")-1))
	{
		if (tspage)
		{
			u32 client_id = 0;
			for ( ;client_id < MAX_CLIENTS; ++client_id)
				if (_client_header[client_id])
				{
					xmit_1sec(client_id, page_address(tspage), 1500-((sizeof(struct netheader))-offsetof(struct netheader,iph)));
					break;
				}

			if (client_id >= MAX_CLIENTS)
			{
				printk("This test can't be performed because no client has been used yet\n");
			}
		}
	}
	else if (!strncmp(writebuf, "test_1mb_burst", sizeof("test_1mb_burst")-1))
	{
	  if (tspage)
	  {
	    u32 client_id = 0;
	    for ( ;client_id < MAX_CLIENTS; ++client_id)
	      if (_client_header[client_id])
	      {
	        xmit_1mb(client_id, page_address(tspage), 1500-((sizeof(struct netheader))-offsetof(struct netheader,iph)));
	        break;
	      }

	    if (client_id >= MAX_CLIENTS)
	    {
	      printk("This test can't be performed because no client has been used yet\n");
	    }
	  }
	}
	else if ((!strncmp(writebuf, "test_100us_transmit", sizeof("test_100us_transmit") - 1)) )
	{
		static struct queue_item qitem;
		if (_client_header[0])
		{
			queue_xmit_payload(0, ((u8*)page_address(tspage))+1*DEF_PAYLOAD_SIZE, DEF_PAYLOAD_SIZE, &qitem);
			printk("qitem to send in 100us periods has been prepared\n");
			_qitem = &qitem;
		}
	}
	else if (!strncmp(writebuf, "print_timer",sizeof("print_timer")-1))
	{
		printk("timer_tick %u\n",timer_tick);
		printk("added_to_queue %u from_queue_to_xmit %u\n",added_to_queue,from_queue_to_xmit);
	}

	return count;
}

static int ivoupd_start_transferring( struct ivo_udp_start_params_t* ivo_udp_start_params )
{
	int ret = -1;
	unsigned char *h_addr;
	unsigned char hw_addr[ALIGN(MAX_ADDR_LEN, sizeof(unsigned long))];
	BUG_ON(net_eth0 == 0);

	if ( ivo_udp_start_params->client_id < MAX_CLIENTS )
	{
	unsigned id = ivo_udp_start_params->client_id;
	struct page* page_p;
	struct neighbour *neigh;
	struct netheader* nh;
	void* page_start;
	__be32 src_ip; 
	__be32 dst_ip;

	page_p = alloc_page(GFP_KERNEL|GFP_DMA);
	mb();
	if (unlikely(page_p == NULL))
	{
		printk("ivoupd_start_transferring: couldn't allocate for client params");
		ret = -ENOMEM;
		goto exit;
	}
	get_page(page_p);

	src_ip = htonl(ivo_udp_start_params->src_ip); 
	dst_ip = htonl(ivo_udp_start_params->dst_ip);

	neigh = neigh_lookup(&arp_tbl, &dst_ip, net_eth0);
	if (unlikely(neigh == NULL))
	{
	  if (ipv4_is_multicast(dst_ip))
	  {
	    hw_addr[0] = 0x01;
	    hw_addr[1] = 0x00;
	    hw_addr[2] = 0x5e;
	    hw_addr[3] = (dst_ip>>8 ) & 0x7f;
	    hw_addr[4] = (dst_ip>>16) & 0xff;
	    hw_addr[5] = (dst_ip>>24) & 0xff;
	  }
	  else
	  {
	    printk("ivoudp: can't obtain destination mac address for IP %lu.%lu.%lu.%lu\n",
	       (ivo_udp_start_params->dst_ip>>24)&0x0ff, (ivo_udp_start_params->dst_ip>>16)&0x0ff,
	       (ivo_udp_start_params->dst_ip>>8)&0x0ff, ivo_udp_start_params->dst_ip&0x0ff);
	    ret = -ENODEV;
	    goto exit;
	  }
	}
	else
	{
	  memcpy(hw_addr, neigh->ha, sizeof(hw_addr));
	}

	printk("ivoupd_start_transferring: destination mac address for IP %lu.%lu.%lu.%lu is %02X:%02X:%02X:%02X:%02X:%02X\n",
		   (ivo_udp_start_params->dst_ip>>24)&0x0ff, (ivo_udp_start_params->dst_ip>>16)&0x0ff,
		   (ivo_udp_start_params->dst_ip>>8)&0x0ff, ivo_udp_start_params->dst_ip&0x0ff,
		   hw_addr[0],hw_addr[1],hw_addr[2],hw_addr[3],hw_addr[4],hw_addr[5]);

	h_addr = (unsigned char *)net_eth0->dev_addr;
	printk("ivoupd_start_transferring: source mac address is %02X:%02X:%02X:%02X:%02X:%02X\n", h_addr[0], h_addr[1], h_addr[2], h_addr[3], h_addr[4], h_addr[5]);
	page_start = page_address(page_p);
	//printk("client page 0x%08X\n", (u32)page_start);
	BUG_ON(page_start == NULL);
	WARN_ON((u32)page_start < P1SEG || (u32)page_start >= P2SEG);
	memset(page_start, 0, PAGE_SIZE);

	nh = (struct netheader*)page_start;
	nh->ethh.h_proto = htons(ETH_P_IP);
	memcpy(nh->ethh.h_source, net_eth0->dev_addr, ETH_ALEN);
	memcpy(nh->ethh.h_dest, hw_addr, ETH_ALEN);

	nh->iph.ihl = 5;
	nh->iph.version = 4; //IPv4
	nh->iph.tos = 16; // Low delay
#ifdef RTP_HEADER
	nh->iph.tot_len = htons(sizeof(nh->iph) + sizeof(nh->udph) + sizeof(nh->rtph) + DEF_PAYLOAD_SIZE);
#else
	nh->iph.tot_len = htons(sizeof(nh->iph) + sizeof(nh->udph) + DEF_PAYLOAD_SIZE);
#endif
	nh->iph.id = 0;
	nh->iph.frag_off = htons(IP_DF);
	nh->iph.ttl = 64; // hops
	nh->iph.protocol = IPPROTO_UDP; // UDP
	nh->iph.check = 0;
	// Source IP address, can use spoofed address here!!!
	nh->iph.saddr = src_ip;
	// The destination IP address
	nh->iph.daddr = dst_ip;
	nh->iph.check = 0;
#ifndef STMMAC_CHECKSUM_INSERTION
	nh->iph.check = csum((unsigned short *)&nh->iph, sizeof(nh->iph) / sizeof(unsigned short));
#endif

	nh->udph.source = htons(ivo_udp_start_params->src_port);
	nh->udph.dest = htons(ivo_udp_start_params->dst_port);
#ifdef RTP_HEADER
	nh->udph.len = htons(sizeof(nh->udph) + sizeof(nh->rtph) + DEF_PAYLOAD_SIZE);
#else
	nh->udph.len = htons(sizeof(nh->udph) + DEF_PAYLOAD_SIZE);
#endif

	printk("ivoupd_start_transferring: source/dest port: %hu/%hu\n", ivo_udp_start_params->src_port, ivo_udp_start_params->dst_port);

	mb();
	dma_cache_sync(NULL, page_start, PAGE_SIZE, DMA_BIDIRECTIONAL); // Sync cache.

	{
	struct page* old_page_p = _client_header[id];
	mb();
	_client_header[id] = page_p;
	if ( old_page_p )
	{
		unsigned int to_purge_item = _to_purge_item++ % MAX_TO_PURGE;
		if (_page_to_purge[to_purge_item])
		{
			put_page(_page_to_purge[to_purge_item]);
			__free_page(_page_to_purge[to_purge_item]);
		}
		_page_to_purge[to_purge_item] = old_page_p;
	}
	}
	ret = 0;

	}
exit:
	return ret;
}

static int ivoupd_stop_transferring( struct ivo_udp_stop_params_t* ivo_udp_stop_params )
{
	unsigned id = ivo_udp_stop_params->client_id;

	if (id < MAX_CLIENTS )
	{
		struct page* old_page_p = _client_header[id];
		mb();
		_client_header[id] = NULL;
		if ( old_page_p )
		{
			unsigned int to_purge_item = _to_purge_item++ % MAX_TO_PURGE;
			if (_page_to_purge[to_purge_item])
			{
				put_page(_page_to_purge[to_purge_item]);
				__free_page(_page_to_purge[to_purge_item]);

			}
			_page_to_purge[to_purge_item] = old_page_p; 
			mdelay(MAX_UDELAY_MS);
		}
	}

	return 0;
}


static int ivo_eth_open(struct inode *inode_ptr, struct file *file_ptr)
{
	return 0;
}

static int ivo_eth_release(struct inode *inode_ptr, struct file *file_ptr)
{
	return 0;
}


static int ivo_eth_ioctl(struct inode *inode_ptr, struct file *file_ptr, unsigned int cmd, unsigned long arg)
{
	int res = -EPERM;

	switch ( cmd )
	{
	default:
		break;
	case IVO_UDP_TRANSFER_START:
		{
		struct ivo_udp_start_params_t ivo_udp_start;
		copy_from_user(&ivo_udp_start, (void* __user)arg, _IOC_SIZE( cmd ));
		res = ivoupd_start_transferring(&ivo_udp_start);
		}
		break;
	case IVO_UDP_TRANSFER_STOP:
		{
		struct ivo_udp_stop_params_t ivo_udp_stop;
		copy_from_user(&ivo_udp_stop, (void* __user)arg, _IOC_SIZE( cmd ));
		res = ivoupd_stop_transferring(&ivo_udp_stop);
		}
		break;
	}

	return res;
}

/***********************************************************/

struct ivoeth_dev_t
{
	unsigned	major;
	struct cdev	cdev;
};


struct ivoeth_dev_t ivoeth_dev;

struct file_operations ivoupd_fops = {
	.owner   = THIS_MODULE,
	.open    = ivo_eth_open,
	.release = ivo_eth_release,
	.ioctl   = ivo_eth_ioctl,
};

/***********************************************************/

int __init stmmac_init_module(void);
void __exit stmmac_cleanup_module(void);

static void __exit ivo_eth_exit(void)
{
    ivo_eth_timer_stop();

	_stop_thread();

	dev_put( net_eth0 );

	remove_proc_entry( "ivo_eth", 0 );
	cdev_del( &ivoeth_dev.cdev );
	unregister_chrdev_region( MKDEV( ivoeth_dev.major, 0 ), 1 );

	stmmac_cleanup_module();

	clean_up_pages(_client_header, MAX_CLIENTS);
	clean_up_pages(_eth_item, MAX_ETH_ITEMS);
	clean_up_pages(_page_to_purge, MAX_TO_PURGE);
}


static int __init ivo_eth_init(void)
{
	int ret;
	int i;
	dev_t dev;
	dev_t devno;

	ret = stmmac_init_module();
	if (ret)
		return ret;

	net_eth0 = dev_get_by_name(&init_net, "eth0");
	if ( !net_eth0 )
	{
		return -ENXIO;
	}

    ivo_eth_timer_start();

	_proc_ptr = create_proc_entry( "ivo_eth", 0600, 0 );
	if (_proc_ptr)
	{
		_proc_ptr->read_proc = _read_proc;
		_proc_ptr->write_proc = _write_proc_ivo;
	}

	for (i=0; i<MAX_ETH_ITEMS; ++i)
	{
		struct page* page_p = alloc_page(GFP_KERNEL | GFP_DMA);
		if ( !page_p )
		{
			ret = -ENOMEM;
			goto exit;
		}

		get_page(page_p);
		_eth_item[i] = page_p;
		WARN_ON((u32)page_address(page_p) < P1SEG || (u32)page_address(page_p) >= P2SEG);
	}

	if ( alloc_chrdev_region( &dev, 0, 1, "ivo_eth" ) )
	{
		printk( "[fta_fe] #error# register_chrdev_region failed\n");
		ret = -EBUSY;
		goto exit;
	}

	ivoeth_dev.major = MAJOR(dev);
	devno = MKDEV( ivoeth_dev.major, 0 );
	cdev_init( &ivoeth_dev.cdev, &ivoupd_fops );
	ivoeth_dev.cdev.owner = THIS_MODULE;
	ret = cdev_add( &ivoeth_dev.cdev, devno, 1 );
	printk("IVOETH init, major %d\n", ivoeth_dev.major);

	return 0;

exit:
	clean_up_pages(_eth_item, MAX_ETH_ITEMS);

	return ret;
}


module_init(ivo_eth_init);
module_exit(ivo_eth_exit);
MODULE_LICENSE("GPL");



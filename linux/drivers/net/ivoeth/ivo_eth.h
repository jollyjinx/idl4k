#ifndef _LINUX_IVO_UPD_H
#define _LINUX_IVO_UPD_H


struct ivo_udp_start_params_t
{
    unsigned int client_id;
    unsigned long  src_ip;
    unsigned short src_port;
    unsigned long  dst_ip;
    unsigned short dst_port;
};

struct ivo_udp_stop_params_t
{
    unsigned int client_id;
};

#define IVO_UDP_TRANSFER_START  _IOW('I', 0x00, struct ivo_udp_start_params_t)
#define IVO_UDP_TRANSFER_STOP   _IOW('I', 0x01, struct ivo_udp_stop_params_t)


#endif	/* _LINUX_IVO_UPD_H */

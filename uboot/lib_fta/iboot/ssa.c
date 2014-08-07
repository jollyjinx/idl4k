#define KEY_LEN		64

#include "iboot.h"
#include "sha1.h"

#define DBG_SSA(x)

static inline void printDigest(const char* desc, unsigned char digest[SHA1_SUM_LEN])
{
	int i;
	printf("%s :", desc);
	for(i=0; i<SHA1_SUM_LEN; i++)
	{
		printf("%.2X", digest[i]);
	}
	printf("\n");
}

void ssaCheck(unsigned char *binary)
{
	unsigned char digest[SHA1_SUM_LEN];

	ssa_sign_t *sign = NULL;
	int         size = 0;
	int         res  = 1;

	DBG_SSA(printf("[SSA] Check binary @0x%p\n", binary);)

	if((unsigned int)binary & 3) {
		DBG_SSA(printf("[SSA] Invalid Address\n");)
		goto leave;
	}

	size = swap32(*(unsigned int *)binary);
	sign = (ssa_sign_t *)&binary[size];
	DBG_SSA(printf("[SSA] binary size 0x%X\n", size);)

	size = (unsigned int)sign->digest - (unsigned int)binary;
	DBG_SSA(printf("[SSA] Checking 0x%X bytes\n", size);)

	if(sizeof(sign->digest) != SHA1_SUM_LEN) {
		DBG_SSA(printf("[SSA] Invalid Digest Length\n");)
		goto leave;
	}

	if(sizeof(sign->key) != KEY_LEN) {
		DBG_SSA(printf("[SSA] Invalid Key Length\n");)
		goto leave;
	}

	sha1_hmac(sign->key, 64, binary, size, digest);

	DBG_SSA(printDigest("[SSA] Signature", sign->digest);)
	DBG_SSA(printDigest("[SSA] Computed ", digest);)

	res = memcmp(digest, sign->digest, SHA1_SUM_LEN);

leave:
	DBG_SSA(printf("[SSA] %s\n", res ? "Failure" : "Success");)
	while(res);
}


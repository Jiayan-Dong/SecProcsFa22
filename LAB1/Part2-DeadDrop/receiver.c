
#include "util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

#define BUFF_SIZE (1 << 21)

int main(int argc, char **argv)
{
	// Put your covert channel setup code here
	void *buf = mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);

	if (buf == (void *)-1)
	{
		perror("mmap() error\n");
		exit(EXIT_FAILURE);
	}
	*((char *)buf) = 1; // dummy write to trigger page allocation

	volatile unsigned char tmp_chr;

	printf("Please press enter.\n");

	char text_buf[2];
	fgets(text_buf, sizeof(text_buf), stdin);

	printf("Receiver now listening.\n");

	bool listening = true;
	while (listening)
	{
		// Put your covert channel code here
		for (unsigned char i = 0; i < 256; i++)
		{
			// Prime
			for (int j = 0; j < 8; j++)
			{
				tmp_chr = ((char *)buf)[(i << 6) + (j << 15)];
			}

			uint64_t t0 = rdtscp64();
			while (rdtscp64() - t0 < 1000)
			{
				/* code */
			}
			
			// Probe
			for (int j = 0; j < 8; j++)
			{
				uint32_t t = measure_one_block_access_time((uint64_t) &((char *)buf)[(i << 6) + (j << 15)]);
				// if(t > 77)
				// 	printf("%u\n", i);
				printf("%d\n", t);
			}
		}
	}

	printf("Receiver finished.\n");

	return 0;
}

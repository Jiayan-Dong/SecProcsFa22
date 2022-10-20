
#include "util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

#define BUFF_SIZE (1 << 21)

int main(int argc, char **argv)
{
	volatile char tmp;

	for (size_t i = 0; i < 20000000; i++)
	{
		for (size_t j = 0; j < 8; j++)
		{
			tmp++;
		}
	}

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

	uint64_t *target_buffer = (uint64_t *)buf;
	CYCLES probe_time[8] = {0};
	double sum[256] = {0};

	while (listening)
	{
		// Put your covert channel code here
		for (int i = 0; i < 256; i++)
		{

			// Prime
			for (int j = 0; j < 8; j++)
			{
				for (size_t k = 0; k < 8; k++)
				{
					tmp_chr = *(unsigned char *)((uint64_t)buf + (i << 6) + (j << 15));
				}
			}


			double local_sum = 0;
			// Probe
			for (int j = 0; j < 8; j++)
			{
				local_sum += measure_one_block_access_time((uint64_t)buf + (i << 6) + (j << 15));
				sum[i] = sum[i] * 0.9999 + local_sum * 0.0001;
			}

			// if (sum[i] > 230)
			// {
			// 	printf("%d\n", i);
			// }
		}
		printf("%f\n", sum[0]);
	}

	printf("Receiver finished.\n");

	return 0;
}

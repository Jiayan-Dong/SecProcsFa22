// Name: Jiayan Dong
// Last Modified: 10/20/2022
// Prime and probe receiver for Ivy Bridge L1/L2, 32 KB - 256 KB - 8 MB

#include "util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

#define BUFF_SIZE (1 << 21)
#define SET_NUM 256
#define MISS_THRESHOLD 400
#define NOISE_THRESHOLD 500
#define SAMPLE_NUM 256
#define SAMPLE_THRESHOLD 150
#define WAIT_IN_BETWEEN 15000

int main(int argc, char **argv)
{
	// Warm up code, let cpu core run at stable frequency
	// to eliminate rdtsc noise
	volatile char tmp;
	for (size_t i = 0; i < 20000000; i++)
	{
		for (size_t j = 0; j < 8; j++)
		{
			tmp++;
		}
	}

	// Covert channel setup code below

	// Allocate huge page 2 MB
	void *buf = mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
	if (buf == (void *)-1)
	{
		perror("mmap() error\n");
		exit(EXIT_FAILURE);
	}
	*((char *)buf) = 1; // dummy write to trigger page allocation

	// Start receiver
	printf("Please press enter to start\n");
	char text_buf[2];
	fgets(text_buf, sizeof(text_buf), stdin);
	printf("Receiver now listening.\n");
	bool listening = true;

	double sum[SET_NUM] = {[0 ... 255] = 230}; // The cumulative average latency of the last ten times
	register uint64_t t0, t1;			   // Cycle used to Synchronize
	int received_num[SAMPLE_NUM];				   // The most recent sampled signals
	int cnt = 0;						   // Counter

	// Initialize the most recent sampled signal
	for (int i = 0; i < SAMPLE_NUM; i++)
	{
		received_num[i] = i;
	}

	while (listening)
	{
		// Covert channel code here
		for (int i = 0; i < SET_NUM; i++) // Takes about 4000000 cycles to run this loop
		{

			// Prime
			for (int j = 0; j < 8; j++)
			{
				for (size_t k = 0; k < 8; k++)
				{
					tmp = *(unsigned char *)((uint64_t)buf + (i << 6) + (j << 15));
				}
			}

			// Wait, sender send one signal in about 5000 cycles
			uint64_t t0 = rdtscp64();
			do
			{
				t1 = rdtscp64();
			} while (t1 - t0 < WAIT_IN_BETWEEN);

			// Probe
			double local_sum = 0;
			for (int j = 0; j < 8; j++)
			{
				local_sum += measure_one_block_access_time((uint64_t)buf + (i << 6) + (j << 15));
			}
			sum[i] = sum[i] * 0.9 + local_sum * 0.1; // Sample 10 most recent signal

			// Target set miss: 424 cycles, Target set hit: 230 - 280 cycles, Other sets miss: 313 cycles
			// Basically in line with our expectations, 8 lines L2 hit / miss means 8 * 18 = 144 cycles difference
			if (sum[i] > MISS_THRESHOLD && sum[i] < NOISE_THRESHOLD)
			{
				received_num[cnt % SET_NUM] = i;
				cnt++;
				// Check the number of times the signal appears in the last 256 signals
				int local_cnt = 0;
				for (int j = 0; j < SAMPLE_NUM; j++)
				{
					if (received_num[j] == i)
					{
						local_cnt++;
					}
				}
				// If it is greater than the threshold, the signal sent by the sender
				if (local_cnt > SAMPLE_THRESHOLD)
				{
					printf("%d\n", i);
					for (int j = 0; j < SAMPLE_NUM; j++)
					{
						received_num[j] = j; // Print out the received number
					}
				}
			}
		}
	}

	printf("Receiver finished.\n");

	return 0;
}

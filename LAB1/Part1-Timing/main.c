#include "utility.h"

// Uncomment the following lines and fill in the correct size
#define L1_SIZE 16 * 1024
#define L2_SIZE 256 * 1024
#define L3_SIZE 8 * 1024 * 1024

int main(int ac, char **av)
{

    // create 4 arrays to store the latency numbers
    // the arrays are initialized to 0
    uint64_t dram_latency[SAMPLES] = {0};
    uint64_t l1_latency[SAMPLES] = {0};
    uint64_t l2_latency[SAMPLES] = {0};
    uint64_t l3_latency[SAMPLES] = {0};

    // A temporary variable we can use to load addresses
    // The volatile keyword tells the compiler to not put this variable into a
    // register- it should always try to load from memory/ cache.

    // Warm up code, let cpu core run at stable frequency
    // to eliminate rdtsc noise
    volatile char tmp;
    for (size_t i = 0; i < 10000000; i++)
    {
        for (size_t j = 0; j < 8; j++)
        {
            tmp++;
        }
    }

    // Allocate a buffer of 1024 * 1024 * 8 Bytes (8 MB)
    // the size of an unsigned integer (uint64_t) is 8 Bytes
    // Therefore, we request 1024 * 1024 * 8 Bytes (8 MB)
    uint64_t *target_buffer = (uint64_t *)malloc((1024 * 1024 * 2) * sizeof(uint64_t));

    if (NULL == target_buffer)
    {
        perror("Unable to malloc");
        return EXIT_FAILURE;
    }

    // Example: Measure L1 access latency, store results in l1_latency array
    for (int i = 0; i < SAMPLES; i++)
    {
        // Step 1: bring the target cache line into L1 by simply accessing the line
        tmp = target_buffer[0];
        // Step 2: measure the access latency
        l1_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }

    // [1.4] TODO: Uncomment the following line to allocate a buffer of a size
    // of your chosing. This will help you measure the latencies at L2 and L3.

    // ======
    // [1.4] TODO: Measure DRAM Latency, store results in dram_latency array
    // ======
    for (int i = 0; i < SAMPLES; i++)
    {
        // Step 1: clflush the target cache line into DRAM by calling clflush
        clflush((void *)target_buffer);
        // Step 2: measure the access latency
        dram_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }

    // ======
    // [1.4] TODO: Measure L2 Latency, store results in l2_latency array
    // ======
    for (int i = 0; i < SAMPLES; i++)
    {
        // Step 1: bring the target cache line into L1
        tmp = target_buffer[0];
        // Step 2: bring the target cache line into L2 by access all lines in L2 except the target line
        for (int j = 1; j < 1024 * 8; j++) // 4 K lines
        {
            for (size_t k = 0; k < 4; k++)
            {
                tmp = target_buffer[j * 8];
            }
        }
        // Step 3: measure the access latency
        l2_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }

    // ======
    // [1.4] TODO: Measure L3 Latency, store results in l3_latency array
    // ======
    for (int i = 0; i < SAMPLES; i++)
    {
        // Step 1: bring the target cache line into L1
        tmp = target_buffer[0];
        // Step 2: bring the target cache line into L3 by access all lines in L3 except the target line
        for (int j = 1; j < 1024 * 256; j++) // 128 K lines
        {
            for (size_t k = 0; k < 4; k++)
            {
                tmp = target_buffer[j * 8];
            }
        }
        // Step 3: measure the access latency
        l3_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }

    // Print the results to the screen
    // [1.5] Change print_results to print_results_for_python so that your code will work
    // with the python plotter software
    print_results(dram_latency, l1_latency, l2_latency, l3_latency);
    // print_results_for_python(dram_latency, l1_latency, l2_latency, l3_latency);

    free(target_buffer);

    return 0;
}

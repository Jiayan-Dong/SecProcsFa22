// Name: Jiayan Dong
// Last Modified: 10/20/2022
// Prime and probe sender for Ivy Bridge L1/L2, 32 KB - 256 KB - 8 MB

#include "util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

#define BUFF_SIZE (1 << 21)

int main(int argc, char **argv)
{
  // Allocate a buffer using huge page
  // See the handout for details about hugepage management
  void *buf = mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);

  if (buf == (void *)-1)
  {
    perror("mmap() error\n");
    exit(EXIT_FAILURE);
  }
  // The first access to a page triggers overhead associated with
  // page allocation, TLB insertion, etc.
  // Thus, we use a dummy write here to trigger page allocation
  // so later access will not suffer from such overhead.
  *((char *)buf) = 1; // dummy write to trigger page allocation

  // Covert channel setup code here
  volatile unsigned char tmp_chr;

  register int num;

  register uint64_t t0, t1;

  printf("Please type a message.\n");

  bool sending = true;
  while (sending)
  {
    char text_buf[128];
    printf("< ");
    fgets(text_buf, sizeof(text_buf), stdin);
    num = atoi(text_buf);
    if (num < 0 || num > 255)
    {
      printf("Please enter 0 - 255 (8 bit number)\n");
      continue;
    }
    
    t0 = rdtscp64();
    do
    {
      for (int j = 32; j < 64; j++) // Takes < 4000 cycles
      {
        for (size_t k = 0; k < 8; k++)
        {
          tmp_chr = *(unsigned char *)((uint64_t)buf + (num << 6) + (j << 15));
        }
      }
      t1 = rdtscp64();
    } while (t1 - t0 < 800000000); // 200 signals to receiver
  }

  printf("Sender finished.\n");
  return 0;
}

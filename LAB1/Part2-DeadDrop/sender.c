
#include "util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

// TODO: define your own buffer size
#define BUFF_SIZE (1 << 21)
//#define BUFF_SIZE [TODO]

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
  printf("%p\n", buf);

  // TODO:
  // Put your covert channel setup code here
  volatile unsigned char tmp_chr;

  register int num;

  printf("Please type a message.\n");

  bool sending = true;
  while (sending)
  {
    char text_buf[128];
    printf("< ");
    fgets(text_buf, sizeof(text_buf), stdin);
    num = atoi(text_buf);

    for (size_t i = 0; i < 1000000;i)
    {
      for (int j = 32; j < 64; j++)
      {
        for (size_t k = 0; k < 8; k++)
        {
          tmp_chr = *(unsigned char *)((uint64_t)buf + (num << 6) + (j << 15));
        }
      }
    }
  }

  printf("Sender finished.\n");
  return 0;
}

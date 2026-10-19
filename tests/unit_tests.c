#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DOBBY_IMPLEMENTATION
#include "../dobby.h"
#undef DOBBY_IMPLEMENTATION

// NDEBUG disables assert, this will never be disabled
#define ALWAYS_ASSERT(expr) \
  ((expr) ? (void)0 : (fprintf(stderr, "Assertion failed at line %d: %s\n", __LINE__, #expr), exit(1)))

void test_double_buffer(void) {
  struct double_buffer dbuf;
  struct double_buffer_config cfg = {
    .elem_size = sizeof(int),
    .elem_align = 64,
    .elem_count = 10
  };

  double_buffer_init(&dbuf, cfg);

  int *write = double_buffer_write(&dbuf);
  const int *read = double_buffer_read(&dbuf);

  ALWAYS_ASSERT((uintptr_t)write % 64 == 0);
  ALWAYS_ASSERT((uintptr_t)read % 64 == 0);
  ALWAYS_ASSERT(write != read);

  for (int i = 0; i < 10; ++i)
    write[i] = i;

  double_buffer_swap(&dbuf);

  ALWAYS_ASSERT(double_buffer_read(&dbuf) == write);
  ALWAYS_ASSERT(double_buffer_write(&dbuf) == read);

  read = double_buffer_read(&dbuf);
  for (int i = 0; i < 10; ++i)
    ALWAYS_ASSERT(read[i] == i);

  double_buffer_swap(&dbuf);
  ALWAYS_ASSERT(double_buffer_write(&dbuf) == write);

  double_buffer_free(&dbuf);
}

void test_alignment(void) {
  const size_t alignments[] = {1, 2, 4, 8, 16, 32, 64, 128};

  for (size_t i = 0; i < sizeof(alignments) / sizeof(alignments[0]); ++i) {
    struct double_buffer dbuf;
    struct double_buffer_config cfg = {
      .elem_size = 1,
      .elem_align = alignments[i],
      .elem_count = 4097
    };

    double_buffer_init(&dbuf, cfg);

    ALWAYS_ASSERT((uintptr_t)double_buffer_read(&dbuf) % cfg.elem_align == 0);
    ALWAYS_ASSERT((uintptr_t)double_buffer_write(&dbuf) % cfg.elem_align == 0);

    double_buffer_swap(&dbuf);
    double_buffer_free(&dbuf);
  }
}

int main(void) {
  printf("Running tests...\n");

  test_double_buffer();
  test_alignment();

  printf("All tests passing...\n");
  return 0;
}

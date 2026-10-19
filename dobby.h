
/**
/// ________   ________ _________________________.___.
///  \______ \  \_____  \\______   \______   \__  |   |
///   |    |  \  /   |   \|    |  _/|    |  _//   |   |
///   |    `   \/    |    \    |   \|    |   \\____   |
///  /_______  /\_______  /______  /|______  // ______|
///          \/         \/       \/        \/ \/
**/
///
/// @title:        Dobby
///
/// @author:       Walker Rout
///
/// @contact:      walkerrout04@gmail.com
///
/// @date_written: 2026/10/18
///
/// @description:  Abstraction for single-threaded double buffering
///
/// @license:      GPL3
///

#ifndef _DOBBY_H
#define _DOBBY_H

#if !defined(__cplusplus) \
  && (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L)
#error "dobby needs C11"
#endif

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct double_buffer_inner {
  uint8_t *head_read;
  uint8_t *head_write;
};

// non owning api
void double_buffer_inner_init(
  struct double_buffer_inner *inner,
  uint8_t *head_read,
  uint8_t *head_write
);
const void *double_buffer_inner_read(const struct double_buffer_inner inner);
void *double_buffer_inner_write(const struct double_buffer_inner inner);
void double_buffer_inner_swap(struct double_buffer_inner *inner);

struct double_buffer_config {
  size_t elem_size;
  size_t elem_align;
  size_t elem_count;
};

struct double_buffer {
  struct double_buffer_inner inner;
};

// owning api
void double_buffer_init(
  struct double_buffer *dbuf,
  const struct double_buffer_config cfg
);
void double_buffer_free(const struct double_buffer *dbuf);
const void *double_buffer_read(const struct double_buffer *dbuf);
void *double_buffer_write(const struct double_buffer *dbuf);
void double_buffer_swap(struct double_buffer *dbuf);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _DOBBY_H

#ifdef DOBBY_IMPLEMENTATION

#include <assert.h>
#include <stdlib.h>

// i fucking hate windows
#ifdef _WIN32
#include <malloc.h>
#define _DOBBY_ALIGNED_ALLOC(align, size) _aligned_malloc((size), (align))
#define _DOBBY_ALIGNED_FREE(ptr) _aligned_free(ptr)
#else
#define _DOBBY_ALIGNED_ALLOC(align, size) aligned_alloc((align), (size))
#define _DOBBY_ALIGNED_FREE(ptr) free(ptr)
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void double_buffer_inner_init(
  struct double_buffer_inner *inner,
  uint8_t *head_read,
  uint8_t *head_write
) {
  assert(inner);
  inner->head_read = head_read;
  inner->head_write = head_write;
}

const void *double_buffer_inner_read(const struct double_buffer_inner inner) {
  return (const void *)inner.head_read;
}

void *double_buffer_inner_write(const struct double_buffer_inner inner) {
  return (void *)inner.head_write;
}

void double_buffer_inner_swap(struct double_buffer_inner *inner) {
  assert(inner);
  void *temp = inner->head_read;
  inner->head_read = inner->head_write;
  inner->head_write = temp;
}

void double_buffer_init(
  struct double_buffer *dbuf,
  const struct double_buffer_config cfg
) {
  assert(dbuf);
  assert(cfg.elem_size > 0);
  assert(cfg.elem_count > 0);
  assert(cfg.elem_align > 0);
  assert((cfg.elem_align & (cfg.elem_align - 1)) == 0);

  // round element size to alignment
  assert(cfg.elem_size <= SIZE_MAX - (cfg.elem_align - 1));
  const size_t elem_stride =
    (cfg.elem_size + cfg.elem_align - 1) & ~(cfg.elem_align - 1);

  // block size for one of the buffers (we need two of these)
  assert(elem_stride <= SIZE_MAX / cfg.elem_count);
  const size_t size = elem_stride * cfg.elem_count;

  // total size of both buffers
  assert(size <= SIZE_MAX / 2);
  const size_t total = 2 * size;

  // round to allocation alignment
  // clang-format off
  // - guy keeps putting my ternaries on two lines...
  const size_t align = cfg.elem_align < _Alignof(max_align_t)
    ? _Alignof(max_align_t)
    : cfg.elem_align;
  // clang-format on
  assert(total <= SIZE_MAX - (align - 1));
  const size_t alloc_size = (total + align - 1) & ~(align - 1);

  // we use aligned alloc to avoid storing cfg
  uint8_t *memory = _DOBBY_ALIGNED_ALLOC(align, alloc_size);
  assert(memory);

  double_buffer_inner_init(&dbuf->inner, memory, memory + size);
}

void double_buffer_free(const struct double_buffer *dbuf) {
  // lower pointer always starts as the allocation base
  // - we add a positive offset to the base to get the second buffer
  if (dbuf->inner.head_read < dbuf->inner.head_write) {
    // read comes before write, so read is start of allocation
    _DOBBY_ALIGNED_FREE(dbuf->inner.head_read);
  } else {
    // write comes before read, so write is start of allocation
    _DOBBY_ALIGNED_FREE(dbuf->inner.head_write);
  }
}

const void *double_buffer_read(const struct double_buffer *dbuf) {
  return double_buffer_inner_read(dbuf->inner);
}

void *double_buffer_write(const struct double_buffer *dbuf) {
  return double_buffer_inner_write(dbuf->inner);
}

void double_buffer_swap(struct double_buffer *dbuf) {
  double_buffer_inner_swap(&dbuf->inner);
}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DOBBY_IMPLEMENTATION

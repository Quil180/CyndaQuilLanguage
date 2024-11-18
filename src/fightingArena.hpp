#pragma once

#include <cstddef>
#include <cstdlib>

class ArenaAllocator {
public:
  inline explicit ArenaAllocator(size_t bytes) : mem_size(bytes) {
    mem_buffer = static_cast<std::byte *>(malloc(mem_size));
    mem_offset = mem_buffer;
  }

  template <typename T> inline T *alloc() {
    void *offset = mem_offset;
    mem_offset += sizeof(T);
    return static_cast<T*>(offset);
  }

  inline ArenaAllocator(const ArenaAllocator &other) = delete;
  inline ArenaAllocator operator=(const ArenaAllocator &other) = delete;
  inline ~ArenaAllocator() { free(mem_buffer); }

private:
  size_t mem_size;
  std::byte *mem_buffer;
  std::byte *mem_offset;
};

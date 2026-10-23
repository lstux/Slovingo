#include "save.h"

#include <stdio.h>
#include <string.h>

bool MemoryStorage::read(void* data, size_t size) {
  if (size_ == 0 && path_ && *path_) {
    FILE* f = fopen(path_, "rb");
    if (f) {
      size_ = fread(buf_, 1, sizeof buf_, f);
      fclose(f);
    }
  }
  if (size_ != size) return false;
  memcpy(data, buf_, size);
  return true;
}

bool MemoryStorage::write(const void* data, size_t size) {
  if (size > sizeof buf_) return false;
  memcpy(buf_, data, size);
  size_ = size;
  writes_++;
  if (path_ && *path_) {
    FILE* f = fopen(path_, "wb");
    if (!f) return false;
    fwrite(buf_, 1, size_, f);
    fclose(f);
  }
  return true;
}

void MemoryStorage::erase() {
  size_ = 0;
  if (path_ && *path_) remove(path_);
}

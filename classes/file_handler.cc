#include <string.h>
#include <stdio.h>

#include "include/file_handler.h"

bool FileHandler::Initialize() {
  f_stream_ = fopen(file_path_.c_str(), "rb");
  if (f_stream_ == nullptr) return true;

  chunk_ = new char[chunk_size_];
  end_chunk_ = chunk_ + chunk_size_;
  record_ = ptr_ = chunk_;

  ReadChunk();

  return false;
}

bool FileHandler::EndOfRecords() {
  while (record_ < ptr_ && *record_ == '\n') record_++;

  if (record_ >= end_chunk_) {
    record_ = ptr_ = chunk_;
    ReadChunk();
    return eof_ || EndOfRecords();
  }

  return record_ >= ptr_;
}

// TODO: What if the length of single value is bigger than the size of chunk.
void FileHandler::NextRecord(char **ptr, size_t *length, bool *end_of_records) {
  if (EndOfRecords()) {
    *end_of_records = true;
    return;
  }

  char *end_of_record = record_;
  while (end_of_record < end_chunk_ && *end_of_record != '\n') end_of_record++;

  if (end_of_record < end_chunk_ || eof_) {
    *ptr = record_;
    *length = end_of_record - record_;
    record_ = end_of_record + 1;
    *end_of_records = false;
  } else {
    strncpy(chunk_, record_, end_chunk_ - record_);
    ptr_ = chunk_ + (end_chunk_ - record_);
    record_ = chunk_;

    // Read the following data
    ReadChunk();

    NextRecord(ptr, length, end_of_records);
  }
}

void FileHandler::ReadChunk() {
  if (feof(f_stream_)) {
    eof_ = true;
    return;
  }

  size_t read_bytes = fread(ptr_, sizeof(char), chunk_size_ - (ptr_ - chunk_),
                            f_stream_);

  ptr_ += read_bytes;

  offset_ += read_bytes;
  fseeko(f_stream_, offset_, SEEK_SET);
}

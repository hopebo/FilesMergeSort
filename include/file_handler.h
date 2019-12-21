#ifndef FILE_HANDLER_H_
#define FILE_HANDLER_H_

#include <stdio.h>

#include <string>

class FileHandler {
 private:
  std::string file_path_;
  long long offset_;
  char *chunk_;
  char *end_chunk_;
  char *record_;
  char *ptr_;
  size_t chunk_size_;
  FILE *f_stream_;
  bool eof_;

  bool EndOfRecords();
  void ReadChunk();

 public:
  FileHandler(std::string file_path, size_t chunk_size) :
      file_path_(file_path),
      offset_(0),
      chunk_(nullptr),
      end_chunk_(nullptr),
      record_(nullptr),
      ptr_(nullptr),
      chunk_size_(chunk_size),
      f_stream_(nullptr),
      eof_(false) {}

  bool Initialize();
  void NextRecord(char **ptr, size_t *length, bool *end_of_records);
  ~FileHandler() {
    if (f_stream_) fclose(f_stream_);
    if (chunk_) delete []chunk_;
  }
};

#endif  // FILE_HANDLER_H_

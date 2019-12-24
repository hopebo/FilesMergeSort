#ifndef FILE_HANDLER_H_
#define FILE_HANDLER_H_

#include <stdio.h>

#include <string>

class FileHandler {
  struct Chunk {
    char *begin;
    char *end;

    char *content_begin;
    char *record;
    char *records_end;
    size_t header_size;
    size_t chunk_size;

    Chunk(size_t chunk, size_t header) :
        chunk_size(chunk),
        header_size(header) {
      begin = new char[chunk_size + header_size];
      end = begin + chunk_size + header_size;
      content_begin = begin + header_size;
    }
  };

 public:
  Chunk *input_;
  Chunk *prefetch_;
  bool back_chunk_ready_;

  FILE *f_stream_;
  long long offset_;
  bool eof_;

  std::string file_;

  FileHandler(std::string file, size_t chunk_size, size_t header_size);

  void Swap();
  void ReadChunk(Chunk *chunk);
  void NextRecord(Chunk *chunk, char **ptr, size_t *length,
                  bool *end_of_records, bool *need_next_chunk);
  ~FileHandler();
};

#endif  // FILE_HANDLER_H_

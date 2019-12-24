#ifndef COORDINATOR_H_
#define COORDINATOR_H_

#include <string>
#include <queue>
#include <vector>

#include "include/event.h"
#include "include/file_handler.h"

typedef unsigned int uint;

class Coordinator {
  struct String {
    char *ptr;
    size_t length;
    int file_num;
    String(char *pointer, size_t len, int index) : ptr(pointer), length(len),
                                                   file_num(index) {}
  };

  struct Cmp {
    bool operator() (String a, String b) {
      size_t len = std::min(a.length, b.length);
      for (size_t i = 0; i < len; i++) {
        if (a.ptr[i] == b.ptr[i]) continue;
        return a.ptr[i] > b.ptr[i];
      }

      return a.length > b.length;
    }
  };

 private:
  std::vector<std::string> files_;
  std::string output_file_;

  size_t chunk_size_;
  size_t header_size_;
  std::vector<FileHandler *> f_handlers_;

  std::queue<uint> p_indexes_;
  std::mutex indexes_mutex_;

  Event prefetch_;
  Event chunk_ready_;
  Event flush_;
  Event flush_buffer_empty_;

  std::vector<std::string> flush_datas_;
  std::vector<std::string> sort_datas_;
  size_t flush_size_;

  std::thread read_thread_;
  std::thread flush_thread_;

  bool finish_;

 public:
  Coordinator(size_t chunk_size, size_t header_size, std::string& output_file,
              std::vector<std::string>& data_files);
  void ReadThread();
  // When a thread finish current work, it will be assigned the next work.
  void Run();
  void ReadRecord(char **ptr, size_t *length, bool *end_of_records, int index);

  void FlushThread();
  ~Coordinator();
};

#endif  // COORDINATOR_H_

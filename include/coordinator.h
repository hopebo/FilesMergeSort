#ifndef COORDINATOR_H_
#define COORDINATOR_H_

#include <string>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>

class Coordinator {
  struct LevelFiles {
    std::queue<std::string> files;
    int k_merge_sort;
    int num_of_remained_files;
  };

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
  int max_threads_;
  int lowest_active_level_;
  size_t chunk_size_;
  std::string output_file_;
  std::mutex mutex_;

 public:
  std::vector<LevelFiles *> level_files_;

  Coordinator(int max_thread, int max_k_merge_sort, size_t chunk_size,
              std::string output_file, std::vector<std::string>& data_files);
  // When a thread finish current work, it will be assigned the next work.
  void Run();
  void ProcessLoop();
  void KMergeSort(std::vector<std::string>& input_files,
                  std::string& output_files, int next_level_index);
  ~Coordinator() {
    for (int i = 0; i < level_files_.size(); i++) {
      delete level_files_[i];
    }
  }
};

#endif  // COORDINATOR_H_

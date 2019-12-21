#include <cmath>
#include <thread>
#include <algorithm>

#include <assert.h>
#include <stdio.h>

#include "include/coordinator.h"
#include "include/file_handler.h"

static std::string GetFileName(std::string path) {
  int pos = path.find_last_of('/');
  return std::string(path.substr(pos + 1));
}

static void PrintLog(std::vector<std::string>& input_files,
                     std::string& output_file) {
  int len = input_files.size();
  int mid = (len - 1) / 2;
  for (int i = 0; i < len; i++) {
    std::string filename = GetFileName(input_files[i]);
    printf("%15s", filename.c_str());
    if (i == mid)
      printf("  ------->>  %s",  GetFileName(output_file).c_str());
    printf("\n");
  }
}

Coordinator::Coordinator(int max_thread, int max_k_merge_sort,
                         size_t chunk_size, std::string output_file,
                         std::vector<std::string>& data_files) :
    max_threads_(max_thread),
    chunk_size_(chunk_size),
    output_file_(output_file),
    lowest_active_level_(0) {
  LevelFiles *first_level = new LevelFiles;
  for (int i = 0; i < data_files.size(); i++)
    first_level->files.push(data_files[i]);

  first_level->num_of_remained_files = first_level->files.size();
  first_level->k_merge_sort = std::min(
      max_k_merge_sort, first_level->num_of_remained_files);

  level_files_.push_back(first_level);

  LevelFiles *last_level = first_level;
  while (last_level->num_of_remained_files > 1) {
    LevelFiles *next_level = new LevelFiles;
    next_level->num_of_remained_files = std::ceil(
        last_level->num_of_remained_files / (double)last_level->k_merge_sort);

    next_level->k_merge_sort = std::min(
        max_k_merge_sort, next_level->num_of_remained_files);

    level_files_.push_back(next_level);
    last_level = next_level;
  }
}

void Coordinator::Run() {
  std::unique_lock<std::mutex> mlock(mutex_);
  std::vector<std::thread> threads;
  for (int i = 0; i < std::ceil(level_files_[0]->num_of_remained_files /
                                (double)level_files_[0]->k_merge_sort); i++) {
    threads.push_back(std::thread(&Coordinator::ProcessLoop, this));
  }

  mlock.unlock();

  for (auto& thread : threads)
    thread.join();
}

void Coordinator::ProcessLoop() {
  int next_level_index = 0;
  std::string output_file;
  while (1) {
    std::unique_lock<std::mutex> mlock(mutex_);

    if (next_level_index > 0) {
      level_files_[next_level_index]->files.push(output_file);
    }

    if (lowest_active_level_ == (int)level_files_.size() - 1) break;

    LevelFiles *current_level = level_files_[lowest_active_level_];

    std::vector<std::string> process_files;
    int files_num = 0;
    if (current_level->files.size() >= current_level->k_merge_sort) {
      files_num = current_level->k_merge_sort;
    } else if (current_level->files.size() ==
               current_level->num_of_remained_files) {
      files_num = current_level->files.size();
    } else {
      break;
    }

    for (int i = 0; i < files_num; i++) {
      process_files.push_back(current_level->files.front());
      current_level->files.pop();
    }

    next_level_index = lowest_active_level_ + 1;

    current_level->num_of_remained_files -= files_num;
    if (current_level->num_of_remained_files == 0) lowest_active_level_++;

    if (next_level_index == (int)level_files_.size() - 1) {
      output_file = output_file_;
    } else {
      output_file = process_files[0] + "_" + std::to_string(next_level_index);
    }

    printf("-----Merge Sort Level %d-----\n", next_level_index - 1);
    printf("Thread #0x%x:\n", std::this_thread::get_id());
    PrintLog(process_files, output_file);
    printf("\n");

    mlock.unlock();

    KMergeSort(process_files, output_file, next_level_index);
  }
}

void Coordinator::KMergeSort(std::vector<std::string>& input_files,
                             std::string& output_file,
                             int next_level_index) {
  std::priority_queue<String, std::vector<String>, Cmp> pq_string;
  std::vector<FileHandler *> file_handlers;
  for (int i = 0; i < input_files.size(); i++) {
    FileHandler *file_handler = new FileHandler(input_files[i], chunk_size_);
    file_handlers.push_back(file_handler);

    bool status = file_handler->Initialize();
    assert(!status);

    char *ptr = nullptr;
    size_t length = 0;
    bool end_of_records = false;
    file_handler->NextRecord(&ptr, &length, &end_of_records);
    if (!end_of_records)
      pq_string.push(String(ptr, length, i));
  }

  FILE *f_stream = fopen(output_file.c_str(), "wb");

  assert(f_stream);

  char delimiter = '\n';
  while (!pq_string.empty()) {
    String string = pq_string.top();
    pq_string.pop();

    size_t bytes = fwrite(string.ptr, sizeof(char), string.length, f_stream);
    bytes += fwrite(&delimiter, sizeof(char), 1, f_stream);

    assert(bytes == string.length + 1);

    char *ptr = nullptr;
    size_t length = 0;
    bool end_of_records = false;
    file_handlers[string.file_num]->NextRecord(&ptr, &length, &end_of_records);
    if (!end_of_records) {
      pq_string.push(String(ptr, length, string.file_num));
    }
  }

  fclose(f_stream);

  if (next_level_index - 1 != 0) {
    for (auto& file : input_files)
      remove(file.c_str());
  }
}

#include <cmath>
#include <thread>
#include <algorithm>
#include <fstream>
#include <string>

#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "include/coordinator.h"

Coordinator::Coordinator(size_t chunk_size, size_t header_size,
                         std::string& output_file,
                         std::vector<std::string>& data_files) :
    chunk_size_(chunk_size),
    header_size_(header_size),
    output_file_(output_file),
    flush_size_(0),
    finish_(false) {
  for (auto& file : data_files) files_.push_back(file);
}

void Coordinator::ReadThread() {
  for (auto& handler : f_handlers_) {
    handler->ReadChunk(handler->prefetch_);
    chunk_ready_.Notify();
  }

  while (!finish_) {
    while (p_indexes_.empty() && !finish_) {
      prefetch_.Wait();
    }

    if (finish_) break;

    std::unique_lock<std::mutex> lock(indexes_mutex_);
    uint index = p_indexes_.front();
    p_indexes_.pop();
    lock.unlock();

    f_handlers_[index]->ReadChunk(f_handlers_[index]->prefetch_);
    chunk_ready_.Notify();
  }
}

void Coordinator::ReadRecord(char **ptr, size_t *length, bool *end_of_records,
                             int index) {
  assert(index >= 0);
  bool need_next_chunk = false;

  do {
    if (need_next_chunk) {
      while (!f_handlers_[index]->back_chunk_ready_)
      {
        chunk_ready_.Wait();
      }

      chunk_ready_.Lock();
      f_handlers_[index]->Swap();
      chunk_ready_.Unlock();

      {
        std::unique_lock<std::mutex> lock(indexes_mutex_);
        p_indexes_.push(index);
        prefetch_.Notify();
      }
    }

    f_handlers_[index]->NextRecord(f_handlers_[index]->input_, ptr, length,
                                   end_of_records, &need_next_chunk);
  } while (need_next_chunk && !(*end_of_records));
}

void Coordinator::Run() {
  for (auto& file : files_) {
    f_handlers_.push_back(new FileHandler(file, chunk_size_, header_size_));
  }

  read_thread_ = std::thread(&Coordinator::ReadThread, this);
  flush_thread_ = std::thread(&Coordinator::FlushThread, this);

  std::priority_queue<String, std::vector<String>, Cmp> pq_string;

  for (int i = 0; i < f_handlers_.size(); i++) {
    char *ptr = nullptr;
    size_t length = 0;
    bool end_of_records = false;
    bool need_next_chunk = false;

    ReadRecord(&ptr, &length, &end_of_records, i);

    if (!end_of_records)
      pq_string.push(String(ptr, length, i));
  }

  while (!pq_string.empty()) {
    String string = pq_string.top();
    pq_string.pop();

    sort_datas_.push_back(std::string(string.ptr, string.length));
    flush_size_ += string.length;

    char *ptr = nullptr;
    size_t length = 0;
    bool end_of_records = false;
    bool need_next_chunk = false;

    ReadRecord(&ptr, &length, &end_of_records, string.file_num);

    if (!end_of_records)
      pq_string.push(String(ptr, length, string.file_num));

    if (flush_size_ >= chunk_size_ || pq_string.empty()) {
      while (!flush_datas_.empty()) {
        flush_buffer_empty_.Wait();
      }

      swap(sort_datas_, flush_datas_);
      sort_datas_.clear();
      flush_size_ = 0;
      flush_.Notify();
    }
  }

  finish_ = true;

  prefetch_.Notify();
  flush_.Notify();
  read_thread_.join();
  flush_thread_.join();
}

void Coordinator::FlushThread() {
  FILE *f_stream = fopen(output_file_.c_str(), "wb");
  char delimiter = '\n';

  while (!finish_) {
    while (flush_datas_.empty() && !finish_) {
      flush_.Wait();
    }

    for (auto& string : flush_datas_) {
      fwrite(string.c_str(), sizeof(char), string.size(), f_stream);
      fwrite(&delimiter, sizeof(char), 1, f_stream);
    }

    flush_datas_.clear();
    flush_buffer_empty_.Notify();
  }

  fclose(f_stream);
}

Coordinator::~Coordinator() {
  for (int i = 0; i < f_handlers_.size(); i++)
    delete f_handlers_[i];
}

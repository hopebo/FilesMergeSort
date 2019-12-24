#include <string.h>
#include <stdio.h>

#include "include/file_handler.h"

FileHandler::FileHandler(std::string file, size_t chunk_size,
                         size_t header_size) :
    offset_(0),
    eof_(false),
    file_(file) {
  input_ = new Chunk(chunk_size, header_size);
  prefetch_ = new Chunk(chunk_size, header_size);
  f_stream_ = fopen(file.c_str(), "rb");
  ReadChunk(input_);
  input_->record = input_->content_begin;
}

void FileHandler::Swap() {
  if (input_->record < input_->records_end) {
    size_t size = input_->records_end - input_->record;
    prefetch_->record = prefetch_->content_begin - size;
    strncpy(prefetch_->record, input_->record, size);
  } else {
    prefetch_->record = prefetch_->content_begin;
  }

  Chunk *temp = prefetch_;
  prefetch_ = input_;
  input_ = temp;

  back_chunk_ready_ = false;
}

void FileHandler::ReadChunk(Chunk *chunk) {
  if (feof(f_stream_)) {
    eof_ = true;
    return;
  }

  size_t read_bytes = fread(chunk->content_begin, sizeof(char),
                            chunk->chunk_size, f_stream_);

  chunk->records_end = chunk->content_begin + read_bytes;

  if (read_bytes < chunk->chunk_size) {
    eof_ = true;
  } else {
    offset_ += read_bytes;
    fseeko(f_stream_, offset_, SEEK_SET);
  }
  if (chunk == prefetch_)
    back_chunk_ready_ = true;
}

void FileHandler::NextRecord(Chunk *chunk, char **ptr, size_t *length,
                             bool *end_of_records, bool *need_next_chunk) {
  *end_of_records = false;
  *need_next_chunk = false;

  while (chunk->record < chunk->records_end && *chunk->record == '\n')
    chunk->record++;

  if (chunk->record >= chunk->records_end) {
    if (eof_ && !back_chunk_ready_) {
      *end_of_records = true;
    } else {
      *need_next_chunk = true;
    }
    return;
  }

  char *end_of_record = chunk->record;
  while (end_of_record < chunk->records_end && *end_of_record != '\n')
    end_of_record++;

  if (end_of_record < chunk->records_end || (eof_ && !back_chunk_ready_)) {
    *ptr = chunk->record;
    *length = end_of_record - chunk->record;
    chunk->record = end_of_record + 1;
  } else {
    *need_next_chunk = true;
  }
}

FileHandler::~FileHandler() {
  delete []input_->begin;
  delete []prefetch_->begin;
}

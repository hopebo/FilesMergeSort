#include <fstream>

#include <gtest/gtest.h>

#include "include/file_handler.h"

const std::string file_path = "test/data_files/data_file_0";
const size_t chunk_size = 1 * 1024 * 1024;
const size_t header_size = 20;

TEST(FileHandler, ReadRecord) {
  FileHandler file_handler(file_path, chunk_size, 20);

  std::fstream f_stream;
  f_stream.open(file_path, std::ios_base::in);

  std::string value;
  int i = 0;
  int j = 0;
  if (f_stream.is_open()) {
    while (f_stream >> value) {
      i++;
      char *ptr = nullptr;
      size_t length = 0;
      bool end_of_records = false;
      bool need_next_chunk = false;
      do {
        if (need_next_chunk) {
          file_handler.ReadChunk(file_handler.prefetch_);
          file_handler.Swap();
          j++;
        }
        file_handler.NextRecord(file_handler.input_, &ptr, &length,
                                &end_of_records, &need_next_chunk);
      } while (need_next_chunk && !end_of_records);

      ASSERT_EQ(strncmp(ptr, value.c_str(), length), 0);
    }

    char *ptr = nullptr;
    size_t length = 0;
    bool end_of_records = false;
    bool need_next_chunk = false;
    file_handler.NextRecord(file_handler.input_, &ptr, &length, &end_of_records,
                            &need_next_chunk);
    ASSERT_TRUE(end_of_records);
  }
}

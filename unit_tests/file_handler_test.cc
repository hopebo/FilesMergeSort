#include <fstream>

#include <gtest/gtest.h>

#include "include/file_handler.h"

const std::string file_path = "test/data_files/data_file_0";
const size_t chunk_size = 10 * 1024;

TEST(FileHandler, ReadRecord) {
  FileHandler file_handler(file_path, chunk_size);
  bool is_failed = file_handler.Initialize();
  ASSERT_EQ(is_failed, false);

  std::fstream f_stream;
  f_stream.open(file_path, std::ios_base::in);

  std::string value;
  int i = 0;
  if (f_stream.is_open()) {
    while (f_stream >> value) {
      i++;
      char *ptr = nullptr;
      size_t length = 0;
      bool end_of_records = false;
      file_handler.NextRecord(&ptr, &length, &end_of_records);
      ASSERT_EQ(strncmp(ptr, value.c_str(), length), 0);
    }

    char *ptr = nullptr;
    size_t length = 0;
    bool end_of_records = false;
    file_handler.NextRecord(&ptr, &length, &end_of_records);
    ASSERT_TRUE(end_of_records);
  }
}

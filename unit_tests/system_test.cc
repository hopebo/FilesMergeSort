#include <fstream>

#include <stdio.h>
#include <dirent.h>

#include <gtest/gtest.h>

#include "include/coordinator.h"

size_t CountLines(const char *filename) {
  std::ifstream read_file;
  size_t n = 0;
  std::string tmp;
  read_file.open(filename, std::ios::in);

  if(read_file.fail()) {
    return 0;
  } else {
    while(getline(read_file, tmp ,'\n')) {
      n++;
    }

    read_file.close();
    return n;
  }
}

TEST(System, Run) {
  std::string input_path = "test/data_files";
  std::string output_file = "test/result/sorted_result";

  DIR *input_dir = nullptr;
  dirent *entry = nullptr;

  input_dir = opendir(input_path.c_str());

  int file_count = 0;
  std::vector<std::string> data_files;
  while ((entry = readdir(input_dir)) != nullptr) {
    if (entry->d_type != DT_REG) continue;

    file_count++;
    data_files.push_back(input_path + "/" + std::string(entry->d_name));
  }

  size_t original_count = 0;
  for (int i = 0; i < data_files.size(); i++) {
    int count = CountLines(data_files[i].c_str());
    original_count += count;
  }

  Coordinator coordinator(1 * 1024 * 1024, 20, output_file, data_files);

  coordinator.Run();

  size_t result_count = 0;

  std::ifstream read_file;
  std::string prev_row;
  std::string cur_row;

  read_file.open(output_file.c_str(), std::ios::in);

  ASSERT_FALSE(read_file.fail());

  while(getline(read_file, cur_row, '\n')) {
    ASSERT_FALSE(cur_row.empty());

    result_count++;

    if (!prev_row.empty()) {
      ASSERT_LE(prev_row, cur_row);
    }
  }

  read_file.close();

  ASSERT_EQ(original_count, result_count);
}

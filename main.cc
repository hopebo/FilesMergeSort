#include <string>
#include <vector>
#include <algorithm>

#include <string.h>
#include <stdio.h>
#include <dirent.h>

#include "include/coordinator.h"

const size_t MAX_K_MERGE_SORT = 4;
const size_t MAX_PARALLEL_DEGREE = 4;
// KB
const size_t CHUNK_SIZE_PER_FILE = 2 * 1024;
const size_t MEMORY_LIMIT = 15 * 1024 * 1024;

/**
   Command line parameters:
     -i  input data files' location
*/
int main(int argc, char **argv) {
  std::string input_path = "test/data_files";
  std::string output_file = "test/result/sorted_result";

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0) {
      input_path = std::string(argv[i + 1]);
    }
  }

  DIR *input_dir = nullptr;
  dirent *entry = nullptr;

  if ((input_dir = opendir(input_path.c_str())) == nullptr) {
    printf("Error: Open intput files' directory failed.");
    return 1;
  }

  int file_count = 0;
  std::vector<std::string> data_files;
  while ((entry = readdir(input_dir)) != nullptr) {
    if (entry->d_type != DT_REG) continue;

    file_count++;
    data_files.push_back(input_path + "/" + std::string(entry->d_name));
  }

  // K-way merge sort
  int k_merge_sort = std::min(MAX_K_MERGE_SORT, MEMORY_LIMIT /
                              CHUNK_SIZE_PER_FILE / MAX_PARALLEL_DEGREE);

  Coordinator coordinator(MAX_PARALLEL_DEGREE, k_merge_sort,
                          CHUNK_SIZE_PER_FILE * 1024, output_file, data_files);

  coordinator.Run();

  return 0;
}

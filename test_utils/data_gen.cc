#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <vector>
#include <algorithm>

std::string RandomString(const int length) {
  std::string str;
  for (int i = 0; i < length; i++) {
    switch (rand() % 3)
    {
      case 0:
        str.push_back('A' + rand() % 26);
        break;
      case 1:
        str.push_back('a' + rand() % 26);
        break;
      default:
        str.push_back('0' + rand() % 10);
        break;
    }
  }

  return str;
}

/**
   Command line parameters:
     -n  number of files to generate
     -d  directory to output the generated files
     -l  average length of the values
     -v  variation of the values' lengths
     -f  file size (MB)
*/
int main(int argc, char **argv) {
  int number_of_files = 1;
  std::string path = "data_files";
  int average_length = 64;
  int variation = 10;
  int file_size_mb = 10;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-n") == 0) {
      number_of_files = atoi(argv[i + 1]);
    } else if (strcmp(argv[i], "-d") == 0) {
      path = std::string(argv[i + 1]);
    } else if (strcmp(argv[i], "-l") == 0) {
      average_length = atoi(argv[i + 1]);
    } else if (strcmp(argv[i], "-v") == 0) {
      variation = atoi(argv[i + 1]);
    } else if (strcmp(argv[i], "-f") == 0) {
      file_size_mb = atoi(argv[i + 1]);
    }
  }

  int64_t file_size = file_size_mb * 1024 * 1024;

  if (access(path.c_str(), F_OK) == 0) {
    printf("Error: The target output directory has already existed.\n");
    return -1;
  }

  if (mkdir(path.c_str(), S_IRWXU) != 0) {
    printf("Error: Create output directory failed.\n");
    return -1;
  }

  srand(11040608);
  // Add one byte for seperate character
  int64_t number_of_values = file_size / (average_length + 1);

  for (int i = 0; i < number_of_files; i++) {
    std::vector<std::string> values;
    for (int j = 0; j < number_of_values; j++) {
      values.push_back(RandomString(average_length + (rand() % variation) *
                                    ((rand() % 2) == 0 ? 1 : -1)));
    }

    std::sort(values.begin(), values.end());

    std::string filename = path + "/data_file_" + std::to_string(i);
    FILE *f_stream = fopen(filename.c_str(), "wb");

    for (int j = 0; j < values.size(); j++) {
      fprintf(f_stream, "%s\n", values[j].c_str());
    }

    fclose(f_stream);
  }

  return 0;
}

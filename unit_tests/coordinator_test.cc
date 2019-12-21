#include <gtest/gtest.h>

#include "include/coordinator.h"

TEST(Coordinator, Initialize) {
  std::vector<std::string> mock_data_files;
  for (int i = 0; i < 28; i++)
    mock_data_files.push_back(std::string('a' + i, 1));

  int max_thread = 3, max_k_merge_sort = 3;
  Coordinator coordinator(max_thread, max_k_merge_sort, 5, "",  mock_data_files);

  ASSERT_EQ(coordinator.level_files_.size(), 5);

  ASSERT_EQ(coordinator.level_files_[0]->num_of_remained_files, 28);
  ASSERT_EQ(coordinator.level_files_[0]->k_merge_sort, 3);

  ASSERT_EQ(coordinator.level_files_[1]->num_of_remained_files, 10);
  ASSERT_EQ(coordinator.level_files_[1]->k_merge_sort, 3);

  ASSERT_EQ(coordinator.level_files_[2]->num_of_remained_files, 4);
  ASSERT_EQ(coordinator.level_files_[2]->k_merge_sort, 3);

  ASSERT_EQ(coordinator.level_files_[3]->num_of_remained_files, 2);
  ASSERT_EQ(coordinator.level_files_[3]->k_merge_sort, 2);

  ASSERT_EQ(coordinator.level_files_[4]->num_of_remained_files, 1);
  ASSERT_EQ(coordinator.level_files_[4]->k_merge_sort, 1);
}

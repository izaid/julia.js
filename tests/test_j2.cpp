#include <stdio.h>

#include "gtest/gtest.h"

namespace {

class J2Environment : public ::testing::Environment {
public:
  void SetUp() { // ...
  }

  void TearDown() {
    // ...
  }
};

} // anonymous namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new J2Environment());

  return RUN_ALL_TESTS();
}

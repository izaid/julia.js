#include <stdio.h>

#include "j2_assertions.h"

V8Environment *V8_ENVIRONMENT;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  V8_ENVIRONMENT = new V8Environment(argv[0]);
  ::testing::AddGlobalTestEnvironment(new JuliaEnvironment());
  ::testing::AddGlobalTestEnvironment(V8_ENVIRONMENT);

  return RUN_ALL_TESTS();
}

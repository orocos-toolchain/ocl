
GLOBAL_ADD_TEST(test1 test1.cpp)
PROGRAM_ADD_DEPS(test1 orocos-rtt)

IF (NOT APPLE)
  GLOBAL_ADD_TEST(test2 test2.cpp)
  PROGRAM_ADD_DEPS(test2 orocos-rtt)
ENDIF (NOT APPLE)

GLOBAL_ADD_TEST(test-free test-free.cpp)
PROGRAM_ADD_DEPS(test-free orocos-rtt)

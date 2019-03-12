cmake_minimum_required (VERSION 3.2 FATAL_ERROR)

project (akamai-mapper-radixtree VERSION 1.0 LANGUAGES CXX)

include(CMakeDependentOption)

option(AKAMAI_MAPPER_RADIXTREE_TEST "Build/run unit tests" OFF)
option(AKAMAI_MAPPER_RADIXTREE_DOCS "Build Doxygen documentation" OFF)
option(AKAMAI_MAPPER_RADIXTREE_EXAMPLES "Build RadixTree examples" OFF)
cmake_dependent_option(BUILD_TESTING "" ON "AKAMAI_MAPPER_RADIXTREE_TEST" OFF)

set(AKAMAI_MAPPER_INSTALL_INCLUDE "include/Akamai/Mapper")
set(AKAMAI_MAPPER_INSTALL_DOC "doc/Akamai/Mapper")

add_subdirectory(include)

if (AKAMAI_MAPPER_RADIXTREE_EXAMPLES)
  add_subdirectory(examples)
endif (AKAMAI_MAPPER_RADIXTREE_EXAMPLES)

if (AKAMAI_MAPPER_RADIXTREE_TEST)
  include(CTest)
  add_subdirectory(test)
endif (AKAMAI_MAPPER_RADIXTREE_TEST)

include(CPack)
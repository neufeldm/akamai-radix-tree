# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in "${CMAKE_BINARY_DIR}/googletest-download/CMakeLists.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

find_package(Threads REQUIRED)

add_executable(test_BinaryTestPath test_BinaryTestPath.cc RandomUtils.cc)
target_compile_options(test_BinaryTestPath PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(test_BinaryTestPath akamai-mapper-radixtree gtest gtest_main pthread)
add_test(NAME testBinaryTestPath COMMAND test_BinaryTestPath)

add_executable(test_TestPath test_TestPath.cc RandomUtils.cc)
target_compile_options(test_TestPath PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(test_TestPath akamai-mapper-radixtree gtest gtest_main pthread)
add_test(NAME testTestPath COMMAND test_TestPath)

add_executable(test_TreeTestUtils test_TreeTestUtils.cc)
target_compile_options(test_TreeTestUtils PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(test_TreeTestUtils akamai-mapper-radixtree gtest gtest_main pthread)
add_test(NAME testTreeTestUtils COMMAND test_TreeTestUtils)

add_executable(test_BinaryPath test_BinaryPath.cc RandomUtils.cc)
target_compile_options(test_BinaryPath PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(test_BinaryPath akamai-mapper-radixtree gtest gtest_main pthread)
add_test(NAME testBinaryPath COMMAND test_BinaryPath)

add_executable(test_SimplePath test_SimplePath.cc RandomUtils.cc)
target_compile_options(test_SimplePath PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(test_SimplePath akamai-mapper-radixtree gtest gtest_main pthread)
add_test(NAME testSimplePath COMMAND test_SimplePath)

add_executable(test_SimpleEdge test_SimpleEdge.cc RandomUtils.cc)
target_compile_options(test_SimpleEdge PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(test_SimpleEdge akamai-mapper-radixtree gtest gtest_main pthread)
add_test(NAME testSimpleEdge COMMAND test_SimpleEdge)

add_executable(test_BinaryWordEdge test_BinaryWordEdge.cc RandomUtils.cc)
target_compile_options(test_BinaryWordEdge PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(test_BinaryWordEdge akamai-mapper-radixtree gtest gtest_main pthread)
add_test(NAME testBinaryWordEdge COMMAND test_BinaryWordEdge)

add_executable(test_BinaryTree test_BinaryTree.cc RandomUtils.cc)
target_compile_options(test_BinaryTree PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(test_BinaryTree akamai-mapper-radixtree gtest gtest_main pthread)
add_test(NAME testBinaryTree COMMAND test_BinaryTree)

add_executable(test_BinaryWordTree test_BinaryWordTree.cc RandomUtils.cc)
target_compile_options(test_BinaryWordTree PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(test_BinaryWordTree akamai-mapper-radixtree gtest gtest_main pthread)
add_test(NAME testBinaryWordTree COMMAND test_BinaryWordTree)

add_executable(test_BinaryWORMTree test_BinaryWORMTree.cc RandomUtils.cc)
target_compile_options(test_BinaryWORMTree PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(test_BinaryWORMTree akamai-mapper-radixtree gtest gtest_main pthread)
add_test(NAME testBinaryWORMTree COMMAND test_BinaryWORMTree)

add_executable(test_SimpleTree test_SimpleTree.cc RandomUtils.cc)
target_compile_options(test_SimpleTree PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(test_SimpleTree akamai-mapper-radixtree gtest gtest_main pthread)
add_test(NAME testSimpleTree COMMAND test_SimpleTree)

add_executable(test_CompoundCursor test_CompoundCursor.cc RandomUtils.cc)
target_compile_options(test_CompoundCursor PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(test_CompoundCursor akamai-mapper-radixtree gtest gtest_main pthread)
add_test(NAME testCompoundCursor COMMAND test_CompoundCursor)

add_executable(test_CursorIterator test_CursorIterator.cc RandomUtils.cc)
target_compile_options(test_CursorIterator PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(test_CursorIterator akamai-mapper-radixtree gtest gtest_main pthread)
add_test(NAME testCursorIterator COMMAND test_CursorIterator)

#add_executable(test_RadixTreeFunctions test_RadixTreeFunctions.cc)
#target_link_libraries(test_RadixTreeFunctions akamai-mapper-radixtree gtest gtest_main pthread)
#add_test(NAME testRadixTreeFunctions COMMAND test_RadixTreeFunctions)

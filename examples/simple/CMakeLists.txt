add_executable(BinaryTrees BinaryTrees.cc)
target_compile_options(BinaryTrees PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(BinaryTrees akamai-mapper-radixtree)

add_executable(TerenaryTree TerenaryTree.cc)
target_compile_options(TerenaryTree PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(TerenaryTree akamai-mapper-radixtree)

add_executable(AlphabetTree AlphabetTree.cc)
target_compile_options(AlphabetTree PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(AlphabetTree akamai-mapper-radixtree)

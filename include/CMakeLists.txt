cmake_minimum_required (VERSION 3.2 FATAL_ERROR)

add_library(akamai-mapper-radixtree INTERFACE)

target_compile_features(akamai-mapper-radixtree
INTERFACE
  cxx_constexpr
  cxx_lambdas
  cxx_nullptr
  cxx_static_assert
  cxx_strong_enums
)

target_include_directories(akamai-mapper-radixtree
INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/RadixTree>
  $<INSTALL_INTERFACE:RadixTree>
)

target_sources(akamai-mapper-radixtree
INTERFACE
${CMAKE_CURRENT_LIST_DIR}/RadixTree/BinaryPath.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/BinaryRadixTree.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/BinaryWordEdge.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/BinaryWordNode.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/BinaryWORMCursorRO.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/BinaryWORMNode.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/BinaryWORMNodeHeader.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/BinaryWORMNodeHeaderBytes.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/BinaryWORMTree.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/BinaryWORMTreeBuilder.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/BinaryWORMTreeGeneric.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/BinaryWORMTreeUInt.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/BinaryWORMTreeUIntBuilder.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/BitPacking.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/CompoundCursor.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/Cursor.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/LookupCursor.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/WalkCursorRO.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/CursorMetaUtils.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/CursorOps.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/CursorTraversal.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/CursorIterator.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/MetaUtils.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/NodeAllocator.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/NodeInterface.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/NodeValue.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/RadixTree.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/RadixTreeUtils.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/SimpleEdge.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/SimpleNodeImpl.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/SimplePath.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/SimpleRadixTree.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/SimpleStack.h
${CMAKE_CURRENT_LIST_DIR}/RadixTree/WordBlockAllocator.h
)

install(
FILES
$<TARGET_PROPERTY:akamai-mapper-radixtree,INTERFACE_SOURCES>
DESTINATION ./${AKAMAI_MAPPER_INSTALL_INCLUDE}/RadixTree)

if (AKAMAI_MAPPER_RADIXTREE_DOCS)
  find_package(Doxygen)
  if (DOXYGEN_FOUND)
    set(AKAMAI_MAPPER_RADIXTREE_DOCS_DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(AKAMAI_MAPPER_RADIXTREE_DOCS_DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    set(AKAMAI_MAPPER_RADIXTREE_CMAKE_DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/doxygen-out)

    configure_file(${AKAMAI_MAPPER_RADIXTREE_DOCS_DOXYFILE_IN} ${AKAMAI_MAPPER_RADIXTREE_DOCS_DOXYFILE_OUT} @ONLY)
    message("Building Doxygen documentation")
    add_custom_command(COMMAND ${DOXYGEN_EXECUTABLE} ${AKAMAI_MAPPER_RADIXTREE_DOCS_DOXYFILE_OUT}
                       DEPENDS $<TARGET_PROPERTY:akamai-mapper-radixtree,INTERFACE_SOURCES> ${AKAMAI_MAPPER_RADIXTREE_DOXYFILE_OUT} ${CMAKE_CURRENT_LIST_DIR}/RadixTree/mainpage.dox
                       OUTPUT ${AKAMAI_MAPPER_RADIXTREE_CMAKE_DOXYGEN_OUT}/html/index.html
                       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                       COMMENT "Generating API documentation with Doxygen"
                       VERBATIM)
    add_custom_target(akamai-mapper-radixtree-doxygen ALL
                      DEPENDS ${AKAMAI_MAPPER_RADIXTREE_CMAKE_DOXYGEN_OUT}/html/index.html)
    install(DIRECTORY ${AKAMAI_MAPPER_RADIXTREE_CMAKE_DOXYGEN_OUT}/html DESTINATION ./${AKAMAI_MAPPER_INSTALL_DOC}/RadixTree)
  else (DOXYGEN_FOUND)
    message("Unable to build documention - Doxygen not found")
  endif(DOXYGEN_FOUND)
endif (AKAMAI_MAPPER_RADIXTREE_DOCS)

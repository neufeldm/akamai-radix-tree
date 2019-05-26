add_library(ipv6addrutils "")
target_compile_features(ipv6addrutils PUBLIC cxx_std_11)
target_compile_options(ipv6addrutils PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(ipv6addrutils akamai-mapper-radixtree)
target_sources(ipv6addrutils
PUBLIC
${CMAKE_CURRENT_LIST_DIR}/IPAddressUtils.h
${CMAKE_CURRENT_LIST_DIR}/IPAddressBlock.h
)
target_sources(ipv6addrutils
PRIVATE
IPAddressUtils.cc
IPAddressBlock.cc
)

add_executable(ParseIPCheck ParseIPCheck.cc)
target_compile_options(ParseIPCheck PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(ParseIPCheck ipv6addrutils)

add_executable(IPLookup IPLookup.cc BinaryTreeLookup.h IPAddrBlockLookup.h IPLookupUtils.h)
target_compile_options(IPLookup PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(IPLookup akamai-mapper-radixtree ipv6addrutils)

add_executable(IPLookupHop IPLookupHop.cc BinaryHopLookup.h IPAddrBlockLookup.h IPLookupUtils.h)
target_compile_options(IPLookupHop PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(IPLookupHop akamai-mapper-radixtree ipv6addrutils)

add_executable(IPLookupLeap IPLookupLeap.cc BinaryHopLookup.h BinaryLeapLookup.h IPAddrBlockLookup.h IPLookupUtils.h)
target_compile_options(IPLookupLeap PRIVATE ${AKAMAI_MAPPER_CXX_WARNING_FLAGS})
target_link_libraries(IPLookupLeap akamai-mapper-radixtree ipv6addrutils)

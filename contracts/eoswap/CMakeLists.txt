add_contract(eoswap eoswap ${CMAKE_CURRENT_SOURCE_DIR}/src/eoswap.cpp)

target_include_directories(eoswap
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(eoswap
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

# configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/eoswap.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/eoswap.contracts.md @ONLY )

# target_compile_options( eoswap PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )

add_contract(eodos eodos ${CMAKE_CURRENT_SOURCE_DIR}/src/eodos.cpp)

target_include_directories(eodos
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(eodos
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

# configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/eodos.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/eodos.contracts.md @ONLY )

# target_compile_options( eodos PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )

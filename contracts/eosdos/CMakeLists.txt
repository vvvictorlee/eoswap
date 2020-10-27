add_contract(eosdos eosdos ${CMAKE_CURRENT_SOURCE_DIR}/src/eosdos.cpp)

target_include_directories(eosdos
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(eosdos
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

# configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/eosdos.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/eosdos.contracts.md @ONLY )

# target_compile_options( eosdos PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )

add_executable(sail sail.cpp)

target_link_libraries(
  sail
  PRIVATE sail::sail_options
          sail::sail_warnings
          sample_library)

target_link_system_libraries(
  sail
  PRIVATE
          CLI11::CLI11
          fmt::fmt
          spdlog::spdlog)

target_include_directories(sail PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

add_executable(intro intro.cpp)

target_link_libraries(
  intro
  PRIVATE sail::sail_options
          sail::sail_warnings
          sample_library)

target_link_system_libraries(
  intro
  PRIVATE
          CLI11::CLI11
          fmt::fmt
          spdlog::spdlog)

target_include_directories(intro PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

cmake_minimum_required(VERSION 3.15)
project(integracja_systemow_projekt LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(is_app)

target_compile_features(is_app PRIVATE cxx_std_26)

target_compile_options(is_app PRIVATE
  -Wall
  -Wextra
  -Wpedantic
  -Wshadow
  -Wconversion
  -fdiagnostics-color
)

target_compile_definitions(is_app PRIVATE 
  SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/src/"
)

add_subdirectory(src)
add_subdirectory(cpp-httplib)

# Find Glaze library
find_package(glaze REQUIRED)

target_link_libraries(is_app PRIVATE 
  httplib::httplib
  sqlite3
  glaze::glaze
)

if (WIN32)
  target_link_libraries(is_app PRIVATE 
    stdc++exp
  )
endif()

#find_package(OpenSSL REQUIRED)

#target_compile_definitions(is_app PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
#target_link_libraries(is_app PRIVATE 
#    OpenSSL::SSL OpenSSL::Crypto
#)

cmake_minimum_required(VERSION 3.5)
project(in_xsf CXX)
enable_language(C)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(in_xsf_framework)
add_subdirectory(in_2sf)
add_subdirectory(in_gsf)
add_subdirectory(in_ncsf)
add_subdirectory(in_snsf)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "in_ncsf")

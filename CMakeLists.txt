cmake_minimum_required(VERSION 2.8...3.15)
project(rakhack VERSION 0.1.0 LANGUAGES C CXX)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
	source/rakclass.cpp
    	source/bitstream.cpp
)

target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                                      $<INSTALL_INTERFACE:include/${PROJECT_NAME}> ${CMAKE_CURRENT_SOURCE_DIR})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

cmake_minimum_required(VERSION 3.22.1)
project(dns_cache VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(GTest REQUIRED)
enable_testing()
find_package(Threads REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${CMAKE_PROJECT_NAME})
target_link_libraries(${CMAKE_PROJECT_NAME}   
)
target_sources(${CMAKE_PROJECT_NAME} PUBLIC   
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp       
  ${CMAKE_CURRENT_SOURCE_DIR}/dns_cache.cpp       
)

add_executable(${CMAKE_PROJECT_NAME}_test)
target_link_libraries(${CMAKE_PROJECT_NAME}_test  
  ${GTEST_BOTH_LIBRARIES} 
)
target_sources(${CMAKE_PROJECT_NAME}_test PUBLIC   
  ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp       
  ${CMAKE_CURRENT_SOURCE_DIR}/dns_cache.cpp       
)
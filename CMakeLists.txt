cmake_minimum_required(VERSION 3.14)

project(db_connection_pool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(
    ${PROJECT_SOURCE_DIR}
    /usr/local/Cellar/jsoncpp/1.9.6/include
    /usr/local/Cellar/mysql/9.2.0_3/include/mysql)

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)

link_directories(/usr/local/Cellar/jsoncpp/1.9.6/lib /usr/local/Cellar/mysql/9.2.0_3/lib)

add_executable(db_connection_pool ${SOURCES})

target_link_libraries(db_connection_pool mysqlclient jsoncpp)
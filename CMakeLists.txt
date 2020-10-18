cmake_minimum_required(VERSION 3.10)
project(TLSserver)

find_package(OpenSSL REQUIRED)
find_package(PostgreSQL REQUIRED)
list(APPEND LIB_LIST ${OPENSSL_LIBRARIES})
list(APPEND LIB_LIST ${PostgreSQL_LIBRARIES})


include_directories(${PROJECT_SOURCE_DIR}/include ${PostgreSQL_INCLUDE_DIRS})

add_library(cJSON_lib OBJECT ${PROJECT_SOURCE_DIR}/src/cJSON/cJSON.c)
add_library(utils_lib OBJECT ${PROJECT_SOURCE_DIR}/src/utils.c)

add_executable(server ${PROJECT_SOURCE_DIR}/src/server.c $<TARGET_OBJECTS:cJSON_lib> $<TARGET_OBJECTS:utils_lib>)
target_link_libraries(server OpenSSL::SSL ${PostgreSQL_LIBRARIES})

add_executable(client ${PROJECT_SOURCE_DIR}/src/client.c $<TARGET_OBJECTS:cJSON_lib> $<TARGET_OBJECTS:utils_lib>)
target_link_libraries(client OpenSSL::SSL ${PostgreSQL_LIBRARIES})

add_subdirectory(test)

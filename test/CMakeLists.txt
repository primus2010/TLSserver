add_executable(test_1 test_1.c $<TARGET_OBJECTS:utils_lib> $<TARGET_OBJECTS:cJSON_lib>)
target_link_libraries(test_1 ${PostgreSQL_LIBRARIES})

add_executable(test_2 test_2.c $<TARGET_OBJECTS:utils_lib> $<TARGET_OBJECTS:cJSON_lib>)
target_link_libraries(test_2 ${PostgreSQL_LIBRARIES})

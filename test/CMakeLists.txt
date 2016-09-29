add_executable(elix_test basic.cpp)
target_include_directories(elix_test PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(elix_test elix)

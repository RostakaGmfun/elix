add_executable(ecf_test basic.cpp)
target_include_directories(ecf_test PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(ecf_test ecf)

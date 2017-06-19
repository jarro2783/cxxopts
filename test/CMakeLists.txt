if (CXXOPTS_BUILD_TESTS)
    add_executable(options_test main.cpp options.cpp)
    target_link_libraries(options_test cxxopts)


    add_test(options options_test)
endif()


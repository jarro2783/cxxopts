if (CXXOPTS_BUILD_TESTS)
    add_executable(options_test main.cpp options.cpp)
    target_link_libraries(options_test cxxopts)

    if (MSVC)
        target_compile_options(options_test PUBLIC /W2)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "[Cc]lang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(options_test PUBLIC -std=c++11 -Wall)
    endif()

    add_test(options options_test)
endif()


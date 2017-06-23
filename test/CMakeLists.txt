if (CXXOPTS_BUILD_TESTS)
    add_executable(options_test main.cpp options.cpp)
    target_link_libraries(options_test cxxopts)
    add_test(options options_test)

    # test if the targets are findable from the build directory
    add_test(find-package-test ${CMAKE_CTEST_COMMAND}
        -C ${CMAKE_BUILD_TYPE}
        --build-and-test
        "${CMAKE_CURRENT_SOURCE_DIR}/find-package-test"
        "${CMAKE_CURRENT_BINARY_DIR}/find-package-test"
        --build-generator ${CMAKE_GENERATOR}
        --build-makeprogram ${CMAKE_MAKE_PROGRAM}
        --build-options
        "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-Dcxxopts_DIR=${PROJECT_BINARY_DIR}"
    )

    # test if the targets are findable when add_subdirectory is used
    add_test(add-subdirectory-test ${CMAKE_CTEST_COMMAND}
        -C ${CMAKE_BUILD_TYPE}
        --build-and-test
        "${CMAKE_CURRENT_SOURCE_DIR}/add-subdirectory-test"
        "${CMAKE_CURRENT_BINARY_DIR}/add-subdirectory-test"
        --build-generator ${CMAKE_GENERATOR}
        --build-makeprogram ${CMAKE_MAKE_PROGRAM}
        --build-options
        "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
    )
endif()

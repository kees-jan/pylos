# Automatically enable catch2 to generate ctest targets
if(CONAN_CATCH2_ROOT_DEBUG)
    include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else()
    include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif()

add_executable(pylos-tests test/test_move.cc)
target_link_libraries(pylos-tests PRIVATE project_warnings project_options catch_main CONAN_PKG::gsl_microsoft cppcoro)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
        pylos-tests
        TEST_PREFIX
        "pylos-tests."
        REPORTER
        xml
        OUTPUT_DIR
        .
        OUTPUT_PREFIX
        "pylos-tests."
        OUTPUT_SUFFIX
        .xml)

# CMake generated Testfile for 
# Source directory: /Users/timsiebert/Projects/add_sanitizers/ADOL-C
# Build directory: /Users/timsiebert/Projects/add_sanitizers/ADOL-C/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[boost-test-adolc]=] "/Users/timsiebert/Projects/add_sanitizers/ADOL-C/build/ADOL-C/boost-test/boost-test-adolc")
set_tests_properties([=[boost-test-adolc]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/timsiebert/Projects/add_sanitizers/ADOL-C/CMakeLists.txt;83;add_test;/Users/timsiebert/Projects/add_sanitizers/ADOL-C/CMakeLists.txt;0;")
subdirs("ADOL-C")
subdirs("ADOL-C/boost-test")

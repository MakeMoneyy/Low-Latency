# CMake generated Testfile for 
# Source directory: D:/Low-Latency
# Build directory: D:/Low-Latency/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(FinancialAppTests "D:/Low-Latency/build/FinancialAppTests.exe")
set_tests_properties(FinancialAppTests PROPERTIES  _BACKTRACE_TRIPLES "D:/Low-Latency/CMakeLists.txt;58;add_test;D:/Low-Latency/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")

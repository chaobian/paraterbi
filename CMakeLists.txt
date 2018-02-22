cmake_minimum_required (VERSION 2.8)
project (PARATERBI C CXX)
set(CG_ORIGIN "${PROJECT_SOURCE_DIR}")

# To compile with thread sanitizer, we need precompiled boost libraries (especially boost::lockfree) that
# have been instrumentalized with thread sanitizer (-fsanitize=thread).
# The following variable must be set to the boost root containing instrumented libraries of boost version 1.60
# Note that the program will only link against them if FSAN is set to 'thread'
set(BOOST160_TSAN_ROOT "${PROJECT_SOURCE_DIR}/boost-1.60-tsan")
set(BOOST160_ASAN_ROOT "${PROJECT_SOURCE_DIR}/boost-1.60-asan")

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message("Note: No build type specified, choosing debug instead of release")
  set(CMAKE_BUILD_TYPE "debug")
  # set default sanitizer to address
  if("${FSAN}" STREQUAL "")
	    message("No sanitizer specified, choosing default.")
    set(FSAN "none")
  endif()
endif()
message("Selecting sanitizer: ${FSAN}")

# set the flag
if("${FSAN}" STREQUAL "none")
  set(FSAN_OPTION   "")
else()
  set(FSAN_OPTION "-fsanitize=${FSAN}")
endif()




set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_CXX_STD c++17)
#add_definitions(-std=c++17)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -DWITHGPERFTOOLS") 
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wc++-compat -Wextra -O3 -mavx2 -march=native -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g ${FSAN_OPTION}")


# this makes the compiler not print out full pathnames on error messages (the filter must be manually adjusted to the path)
configure_file(
    "${PROJECT_SOURCE_DIR}/script/filter/gcc_filter.sh.in"
   "${PROJECT_BINARY_DIR}/gcc_filter.sh"
    @ONLY)
set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${PROJECT_BINARY_DIR}/gcc_filter.sh")

#run script to run all 3 executables and measure
configure_file(
    "${PROJECT_SOURCE_DIR}/script/run.sh"
   "${PROJECT_BINARY_DIR}/run.sh"
    @ONLY)


configure_file(
    "${PROJECT_SOURCE_DIR}/script/sum.py"
   "${PROJECT_BINARY_DIR}/sum.py"
    @ONLY)


configure_file(
    "${PROJECT_SOURCE_DIR}/data/sents"
   "${PROJECT_BINARY_DIR}/sents"
    @ONLY)



add_subdirectory(src)

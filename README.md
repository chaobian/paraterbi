* Paraterbi

Paraterbi is an attempt to implement the viterbi algorithm in a way that takes full advantage of modern CPU, i.e. SIMD and multiple cores. This repository currently contains three versions of the viterbi algorithm and a toy HMM which is used to test viterbi's speed during inference.
Consequently, compilation yields three executables

- viterbi_baseline -> a completely sequential version of the algorithm to compare against
- viterbi_simd -> viterbi using a different memory model and fully utilizing SIMD CPU features
- viterbi -> the fully parallelized version of the algorithm

A run.sh script is provided to automatically run and benchmark the three implementations.

* Installation 
** Requirements
*** C++14

The viterbi code as well as the Vc Library requires at least C++ 14 to work.
*** cmake

We use CMake as a build system.

*** boost

Requires boost version >= 1.60. If you want to use clang sanitizers, make sure to compile boost with the specific sanitizer instrumentation. Use
 $ cmake -DFSAN=<SANITIZER>
 to enable compilation with a specific <SANITIZER>, e.g. thread or memory. See the CMakeLists.txt for more details.
*** Vc

To greatly ease the use of SIMD instructions we use the Vc library. Get it from

http://github.com/vcdevel/vc

build and install the library. CMake takes care of the rest.
** Building

Use cmake like this:

 paraterbi/ $ mkdir build && cd build
 paraterbi/build/ $ cmake -DCMAKE_BUILD_TYPE=release .. 

Then to run/benchmark
paraterbi/build $ run.sh



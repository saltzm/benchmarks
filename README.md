
# About

I wrote these benchmarks for a presentation on "Performance Tips, Tricks, and
Gotchas". They contain benchmarks to compare several different ways of doing
the same thing that I think people either:
1) May not know will affect performance, or
2) Know will affect performance, but not know how much

# Installation

```bash
# Install conan. Used to fetch google benchmark.
sudo apt-get install python3-venv
python3 -m venv pyenv
source pyenv/bin/activate
pip install conan

# Configure conan.
conan profile new default --detect
conan profile update settings.compiler.libcxx=libstdc++11 default
mkdir build && cd build
# This will download google benchmark
conan install ..

# Configure cmake
cmake .. -G "Unix Makefiles" -DCMAKE\_BUILD\_TYPE=Release
# Build benchmark
cmake --build .
# Run benchmark
./bin/benchmarks
```

Output on my machine:

```
Running ./build/bin/benchmarks
Run on (16 X 3396.7 MHz CPU s)
CPU Caches:
  L1 Data 32K (x8)
  L1 Instruction 32K (x8)
  L2 Unified 1024K (x8)
  L3 Unified 25344K (x1)
Load Average: 0.00, 0.11, 0.27
----------------------------------------------------------------------------------------
Benchmark                                              Time             CPU   Iterations
----------------------------------------------------------------------------------------
BM_virtualFunctionCallsThroughPointerToParent       2.51 ns         2.51 ns    294239104
BM_virtualFunctionCallsThroughPointerToChild        1.61 ns         1.61 ns    433945778
BM_virtualFunctionCallsThroughInstanceOfChild      0.295 ns        0.295 ns   1000000000
BM_nonVirtualNonInlineFunctionCall                  3.24 ns         3.24 ns    215936970
BM_inlineFunctionCall                              0.295 ns        0.295 ns   1000000000
BM_noFunctionCall                                  0.295 ns        0.295 ns   1000000000
BM_stdFunctionCall                                  1.77 ns         1.77 ns    395824197
BM_lambdaFunctionCall                              0.295 ns        0.295 ns   1000000000
BM_stdFunctionPassedAsParameterFunctionCall         2.06 ns         2.06 ns    339063066
BM_lambdaPassedAsParameterFunctionCall             0.295 ns        0.295 ns   1000000000
BM_sequentialListAccess                             1367 ns         1367 ns       440074
BM_sequentialArrayAccess                             148 ns          148 ns      4704685
BM_sequentialArrayAccessSmallerThanL1               47.8 ns         47.8 ns     14637763
BM_randomArrayAccessSmallerThanL1                   99.9 ns         99.9 ns      7034840
BM_sequentialArrayAccessBiggerThanL1              158070 ns       158063 ns         4429
BM_randomArrayAccessBiggerThanL1                  727313 ns       727301 ns          839
BM_falseSharing/manual_time                      2841579 ns        41720 ns          246
BM_noFalseSharing/manual_time                    2144634 ns        39660 ns          326
BM_useMutex/manual_time                        116323172 ns        50073 ns            6
BM_useMutexNoContention/manual_time             15982798 ns        36639 ns           44
BM_useAtomic/manual_time                        28326920 ns        39443 ns           25
```

# Fuzzing BehaviorTree.CPP

You can build the existing harnesses either for libfuzzer or AFL++.
Building the fuzzers requires `clang` (libfuzzer) or an installed version
of [AFL++](https://github.com/AFLplusplus/AFLplusplus).

## libfuzzer

```bash
mkdir build_libfuzzer && cd build_libfuzzer
cmake -DENABLE_FUZZING=ON ..
```

## AFL++

```bash
export CC=afl-clang-fast
export CXX=afl-clang-fast++
mkdir build_afl && cd build_afl
cmake -DENABLE_FUZZING=ON -DUSE_AFLPLUSPLUS=ON ..
```

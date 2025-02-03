# Fuzzing BehaviorTree.CPP

You can build the existing harnesses either for libfuzzer or AFL++:

## libfuzzer

```bash
mkdir build_libfuzzer && cd build_libfuzzer
cmake -DENABLE_FUZZING ..
```

## AFL++

```bash
export CC=afl-clang-fast
export CXX=afl-clang-fast++
mkdir build_afl && cd build_afl
cmake -DENABLE_FUZZING -DUSE_AFLPLUSPLUS ..
```

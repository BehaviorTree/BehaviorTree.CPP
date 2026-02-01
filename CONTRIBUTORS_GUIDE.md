# Contributors Guide

Before submitting a Pull Request, please follow these instructions:

- Unless your code is self explaining, add comments.
- Consider if your proposed change introduces API, ABI, back-compatibility or behavioral changes.
- If your code is fixing a bug, please create a unit test to reproduce the bug, i.e. a test that fails before the fix and pass after the fix.
- You use [pre-commit](https://pre-commit.com/) to apply automatically all the required linting rules (clang-format in particular).
- You should also execute the script `./run_clang_tidy.sh` and correct all the warnings.

You will need to install the latest **clang-tidy-21** as follows:

```
  wget https://apt.llvm.org/llvm.sh
  chmod +x llvm.sh
  sudo ./llvm.sh 21
  sudo apt install clangd-21 clang-tidy-21
```

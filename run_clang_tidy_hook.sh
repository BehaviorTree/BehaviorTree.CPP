#!/bin/bash
# Pre-commit hook wrapper for clang-tidy.
# - Skips if clangd-21 is not installed
# - Skips if compile_commands.json not found
# - Runs only on files passed as arguments (modified files)
# - Fails if clang-tidy reports any errors

script_dir=${0%/*}
build_dir="${script_dir}/build"

if ! command -v clangd-21 &> /dev/null; then
  echo "Skipping clang-tidy: clangd-21 not installed"
  exit 0
fi

if [ ! -f "${build_dir}/compile_commands.json" ]; then
  echo "Skipping clang-tidy: compile_commands.json not found (run cmake first)"
  exit 0
fi

if [ $# -eq 0 ]; then
  echo "No files to check"
  exit 0
fi

# Filter to only .cpp, .hpp, .h files and exclude 3rdparty/contrib/scripting
files=()
for f in "$@"; do
  case "$f" in
    3rdparty/*|*/contrib/*|*/scripting/*|*_WIN.cpp) continue ;;
    *.cpp|*.hpp|*.h) files+=("$f") ;;
  esac
done

if [ ${#files[@]} -eq 0 ]; then
  exit 0
fi

echo "Running clang-tidy on ${#files[@]} file(s)..."

failed=0
for f in "${files[@]}"; do
  echo "  Checking: $f"
  output=$(clangd-21 \
      --log=error \
      --clang-tidy \
      --compile-commands-dir="$build_dir" \
      --check-locations=false \
      --check="$f" 2>&1)
  # Check for error lines (starts with E[)
  if echo "$output" | grep -q "^E\["; then
    echo "$output" | grep "^E\["
    failed=1
  fi
done

exit $failed

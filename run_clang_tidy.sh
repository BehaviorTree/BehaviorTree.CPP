#!/bin/bash -eu

script_dir=${0%/*}
ws_dir=$(realpath "$script_dir")

# Check if clangd-21 is available
if ! command -v clangd-21 &> /dev/null; then
  echo "Error: clangd-21 is not installed or not in PATH."
  echo ""
  echo "To install clangd-21 on Ubuntu/Debian, visit:"
  echo "  https://apt.llvm.org/"
  echo ""
  echo "Quick install instructions:"
  echo "  wget https://apt.llvm.org/llvm.sh"
  echo "  chmod +x llvm.sh"
  echo "  sudo ./llvm.sh 21"
  echo "  sudo apt install clangd-21 clang-tidy-21"
  exit 1
fi

# Display help message if --help is passed as an argument
if [[ "${1:-}" == "--help" ]]; then
  echo "Usage: $(basename "$0") [source_path] [build_path]"
  echo "Run clang-tidy on the specified paths."
  echo
  echo "Arguments:"
  echo "  build_path   Path to build directory containing compile_commands.json (default: build)"
  exit 0
fi


clang_tidy_paths="$ws_dir/src $ws_dir/include"
cmake_build_path="$ws_dir/${1:-build}"

skip_list=(
  "$ws_dir/3rdparty"
  "$ws_dir/include/behaviortree_cpp/contrib"
  "$ws_dir/include/behaviortree_cpp/scripting"
  "$ws_dir/include/behaviortree_cpp/flatbuffers"
)

skip_paths=()
for path in "${skip_list[@]}"; do
  skip_paths+=(-not -path "$path/*")
done

## check that the file compile_commands.json exists
if [ ! -f "$cmake_build_path/compile_commands.json" ]; then
  echo "Error: compile_commands.json not found in $cmake_build_path"
  echo "Please build the project first with CMake to generate compile_commands.json"
  exit 1
fi

echo "-----------------------------------------------------------"
echo "Running clang-tidy on $clang_tidy_paths"
echo " Skipping paths:"
for path in "${skip_list[@]}"; do
  echo "  $path"
done
echo "-----------------------------------------------------------"

find "$ws_dir/src" "$ws_dir/include" \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) -not -name '*_WIN.cpp' "${skip_paths[@]}" -print0 \
  | xargs -0 -n 1 -P $(nproc) bash -c '
    set -o pipefail
    echo "$@"
    cd "'"$ws_dir"'" && clangd-21 \
      --log=error \
      --clang-tidy \
      --compile-commands-dir="'"$cmake_build_path"'" \
      --check-locations=false \
      --check="$@" \
      2>&1 | sed "s/^/${1//\//\\/}: /"
    ' _

echo "-----------------------------------------------------------"
echo "Clang-tidy complete."
echo "-----------------------------------------------------------"

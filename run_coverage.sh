#!/bin/bash
set -e

# Code coverage script for BehaviorTree.CPP
# Based on .github/workflows/cmake_ubuntu.yml coverage job

BUILD_DIR="build/coverage"
COVERAGE_FILE="coverage.info"
COVERAGE_HTML_DIR="coverage_report"

echo "=== BehaviorTree.CPP Code Coverage ==="

# Check for lcov
if ! command -v lcov &> /dev/null; then
    echo "Error: lcov is not installed."
    echo "Install with: sudo apt-get install lcov"
    exit 1
fi

# Clean previous coverage data
echo "Cleaning previous coverage data..."
rm -rf "$BUILD_DIR" "$COVERAGE_FILE" "$COVERAGE_HTML_DIR"

# Configure CMake with coverage flags (plain CMake approach)
echo "Configuring CMake with coverage flags..."
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_FLAGS="--coverage -fprofile-update=atomic" \
    -DCMAKE_CXX_FLAGS="--coverage -fprofile-update=atomic"

# Build
echo "Building..."
cmake --build "$BUILD_DIR" --parallel

# Run tests
echo "Running tests..."
ctest --test-dir "$BUILD_DIR" --output-on-failure

# Collect coverage
echo "Collecting coverage data..."
lcov --capture --directory "$BUILD_DIR" \
     --output-file "$COVERAGE_FILE"

# Extract only project source files
echo "Filtering coverage data..."
lcov --extract "$COVERAGE_FILE" \
     "$(pwd)/include/*" \
     "$(pwd)/src/*" \
     --output-file "$COVERAGE_FILE" || true

# Remove contrib files
lcov --remove "$COVERAGE_FILE" \
     '*/contrib/*' \
     --output-file "$COVERAGE_FILE" || true

# Show summary
echo ""
echo "=== Coverage Summary ==="
lcov --list "$COVERAGE_FILE"

# Generate HTML report
if command -v genhtml &> /dev/null; then
    echo ""
    echo "Generating HTML report..."
    genhtml "$COVERAGE_FILE" --output-directory "$COVERAGE_HTML_DIR"
    echo "HTML report generated at: $COVERAGE_HTML_DIR/index.html"
fi

echo ""
echo "Coverage data saved to: $COVERAGE_FILE"

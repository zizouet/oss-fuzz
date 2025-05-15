#!/bin/bash
set -e  # Exit immediately if a command exits with a non-zero status
# Define cleanup function
cleanup() {
  echo "Cleaning up..."
  git reset --hard
}
# Register the cleanup function to run on EXIT signal
trap cleanup EXIT

PROJECT_NAME="freetype2"
FUZZER_NAME="ftfuzzer"
RUN_TIME=$((4 * 60 * 60))
CORPUS_NAME="build/out/run.w__ocorpus/"
FUZZER_ARGS="-rss_limit_mb=2560 -fork=$(nproc) -ignore_crashes=1"

mkdir -p "$CORPUS_NAME"

echo "Applying git diff"
git apply SUBMISSION/part1/oss-fuzz.diff || { echo "Patch failed"; exit 1; }
git apply SUBMISSION/part1/project.diff || { echo "Patch failed"; exit 1; }

echo "Building output folder"
mkdir "$CORPUS_NAME"

echo "Building image for $PROJECT_NAME"
python3 infra/helper.py build_image "$PROJECT_NAME"

echo "Building fuzzers for $PROJECT_NAME"
python3 infra/helper.py build_fuzzers "$PROJECT_NAME"

echo "Running $FUZZER_NAME for 4 hours"
python3 infra/helper.py run_fuzzer "$PROJECT_NAME" "$FUZZER_NAME" --corpus-dir "$CORPUS_NAME" --run_time="$RUN_TIME" -e FUZZER_ARGS="$FUZZER_ARGS"

echo "Creating coverage report"
python3 infra/helper.py build_fuzzers --sanitizer coverage "$PROJECT_NAME"
python3 infra/helper.py coverage "$PROJECT_NAME" --corpus-dir "$CORPUS_NAME" --fuzz-target ftfuzzer




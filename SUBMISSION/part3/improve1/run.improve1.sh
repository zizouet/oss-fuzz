#!/bin/bash
set -e  # Exit immediately if a command exits with a non-zero status

cleanup() {
  echo "Cleaning up oss-fuzz directory..."
  rm -rf oss-fuzz
}
trap cleanup EXIT

cp SUBMISSION/part3/improve1/ftfuzzer.cc ../../../projects/freetype2/
cp SUBMISSION/part3/improve1/Dockerfile ../../../projects/freetype2/


cd oss-fuzz || exit


PROJECT_NAME="freetype2"
FUZZER_NAME="ftfuzzer"
CORPUS_NAME="build/out/corpus"
RUN_TIME=$((4 * 60 * 60))  # 4 hours in seconds

FUZZER_ARGS="-rss_limit_mb=2560 -fork=$(nproc) -ignore_crashes=1"

echo "Building output folder"
mkdir -p "$CORPUS_NAME"

echo "Building image for $PROJECT_NAME"
python3 infra/helper.py build_image "$PROJECT_NAME"

echo "Building fuzzers for $PROJECT_NAME"
python3 infra/helper.py build_fuzzers "$PROJECT_NAME"

echo "Running $FUZZER_NAME for 4 hours"
python3 infra/helper.py run_fuzzer "$PROJECT_NAME" "$FUZZER_NAME" \
  --corpus-dir "$CORPUS_NAME" --run_time="$RUN_TIME" -e FUZZER_ARGS="$FUZZER_ARGS"

echo "Creating coverage report"
python3 infra/helper.py build_fuzzers --sanitizer coverage "$PROJECT_NAME"
python3 infra/helper.py coverage "$PROJECT_NAME" --corpus-dir "$CORPUS_NAME" --fuzz-target "$FUZZER_NAME"



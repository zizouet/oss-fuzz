#!/bin/bash
set -e  # Exit immediately if a command exits with a non-zero status

# Define cleanup function
cleanup() {
  echo "Cleaning up..."
  git reset --hard
}
# Register the cleanup function to run on EXIT signal
trap cleanup EXIT

# Create the target directory first
mkdir -p "projects/freetype2/"

# Copy the folder into directory, excluding .git directory
rsync -a --exclude='.git/' "SUBMISSION/part4/freetype2-testing" "projects/freetype2/"

# copy Dockerfile in directory 
cp "SUBMISSION/part4/Dockerfile" "projects/freetype2/"

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

python3 infra/helper.py reproduce freetype2 ftfuzzer SUBMISSION/part4/poc

#!/usr/bin/env bash

set -e

# Default custom branch
CUSTOM_BRANCH="add_pinio_input_for_blackbox_logging_2025.12.1"

# Parse arguments
while getopts "b:" opt; do
    case "$opt" in
        b)
            CUSTOM_BRANCH="$OPTARG"
        ;;
        *)
            echo "Usage: $0 [-b branch_name]"
            exit 1
        ;;
    esac
done

echo "Fetching latest tags from upstream..."
git fetch upstream --tags

echo
echo "Using branch: $CUSTOM_BRANCH"
echo

# Get latest Betaflight release
LATEST_TAG=$(git tag -l "2025*" --sort=-version:refname | head -1)

if [ -z "$LATEST_TAG" ]; then
    echo "No 2025.x release tags found."
    exit 1
fi

CURRENT_VERSION=$(git describe --tags --always --dirty)

echo "Current repository version: "
echo "  $CURRENT_VERSION"

echo
echo "Latest Betaflight release:"
echo "  $LATEST_TAG"

echo
echo "Available releases:"
git tag -l "2025*" --sort=-version:refname | head -20

echo
read -rp "Merge which tag? " TAG

# If empty, use latest
if [ -z "$TAG" ]; then
    TAG="$LATEST_TAG"
fi

echo
echo "Switching to branch: $CUSTOM_BRANCH"
git checkout "$CUSTOM_BRANCH"

echo
echo "Merging $TAG..."
git merge "$TAG"

echo
echo "=================================="
echo "Finished!"
echo "Branch : $CUSTOM_BRANCH"
echo "Merged : $TAG"
echo "=================================="
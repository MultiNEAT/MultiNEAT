#!/usr/bin/env bash

set -x
set -e

SOURCE_CODE_DIR=${SRC_DIR:-$(dirname $0)/..}

# NoveltySearch.py - requires pygame and runs forever
TESTS="TestTraits.py TestNEAT_xor.py TestHyperNEAT_xor.py"

echo $TESTS | xargs -t -n 1 -P 4 -I % bash -c "python -u ${SOURCE_CODE_DIR}/examples/% | sed 's/^/[%] /'"


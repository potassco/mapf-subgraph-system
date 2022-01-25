#!/usr/bin/env bash
# set -x

# Determine root dir (adopted from https://stackoverflow.com/questions/59895/get-the-source-directory-of-a-bash-script-from-within-the-script-itself)
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && cd .. && pwd)"

: ${CLINGO="clingo"}
: ${OPTS='--out-atomf=%s. --out-ifs=\n'}
: ${CONVOPTS="${OPTS} -V0"}
: ${CONVENC="$ROOT/encodings/convert.lp"}
: ${MAPFOPTS="$OPTS"}
: ${MAPFENC="$ROOT/encodings/singleshot/encoding.lp"}
: ${INSTANCE="$ROOT/resources/instances/ASP_example/instance.lp"}

convert () {
    $CLINGO $CONVOPTS "$CONVENC" "$INSTANCE" | \
        head -n -1 | \
        tee >(echo "#const horizon=$(grep horizon | grep -o "[0-9]\+").")
}

solve () {
    $CLINGO $MAPFOPTS $MAPFENC -
}

convert | solve

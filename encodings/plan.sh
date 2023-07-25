#!/usr/bin/env bash
# set -x

# Determine root dir (adopted from https://stackoverflow.com/questions/59895/get-the-source-directory-of-a-bash-script-from-within-the-script-itself)
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && cd .. && pwd)"

: ${CLINGO="clingo"}
: ${OBJECTIVE="mks"}
: ${OPTS='--out-atomf=%s.'}
: ${CONVOPTS="${OPTS} -V0"}
: ${CONVENC="$ROOT/encodings/translate.lp"}
: ${MAPFOPTS="-q --stat"}
: ${MAPFENC="$ROOT/encodings/$OBJECTIVE-encoding.lp"}
: ${INSTANCE="$ROOT/resources/instances/ASP_example/instance.lp"}

convert () {
    $CLINGO $CONVOPTS "$CONVENC" "$INSTANCE" | head -n -1 
}

solve () {
    $CLINGO $MAPFOPTS $MAPFENC -
}

convert | solve

# ASP-based Approach to Solve MAPF with Map-to-Subgraph Transformation

This is a ASP-based approach for solving MAPF problems that leverages map-to-subgraph transformation.

## Requirements

- A POSIX terminal with GNU [bash](https://www.gnu.org/software/bash/)
- ASP system [clingo](https://potassco.org/clingo/) in version 5.5.0 or higher

## Usage

For execution, you might either

- [call clingo directly](#direct-call) (only for single-shot solving!); or
- run the [shell script](#shell-script)


### <a id="direct-call"></a> Calling Clingo Directly via Command-Line

Manual call of clingo from the command line to solve `resources/instances/ASP_example/instance.lp` in a single-shot fashion using

- conversion encoding `encodings/convert.lp`
- asprilo encoding `encodings/{asprilo/m/encoding.lp`
- destination assignment constraint `encodings/singleshot/assign.lp`

``` bash
(
    clingo --out-atomf='%s.' --out-ifs='\n' -V0 \
           resources/instances/ASP_example/instance.lp encodings/convert.lp |
        head -n -1 |
        tee >(echo "#const horizon=$(grep horizon | grep -o "[0-9]\+").")
) | clingo --out-atomf='%s.' --out-ifs='\n' -V0 \
           encodings/singleshot/encoding.lp  -
```

### <a id="shell-script"></a> Via Bash Script

Use bash script `scripts/plan.sh` to solve `resources/instances/ASP_example/instance.lp` via

``` bash
INSTANCE="resources/instances/ASP_example/instance.lp" scripts/plan.sh
```

The instance and other options may be adjusted by setting the environment variables as specified in
the script, i.e., their names and default values are:

``` bash
: ${CLINGO="clingo"}
: ${OPTS='--out-atomf=%s. --out-ifs=\n'}
: ${CONVOPTS="${OPTS} -V0"}
: ${CONVENC="$ROOT/encodings/convert.lp"}
: ${MAPFOPTS="$OPTS"}
: ${MAPFENC="$ROOT/encodings/encodings/singleshot/encoding.lp"}
: ${INSTANCE="$ROOT/resources/instances/ASP_example/instance.lp"}
```

where `$ROOT` holds the project root

E.g. to get json output with detailed statistics, run


``` bash
INSTANCE="resources/instances/ASP_example/instance.lp" MAPFOPTS="--outf=2 --stat" scripts/plan.sh
```

## Directory Structure

- `./resources/instances/` contains example instances
- `./scripts/` contains command-line scripts


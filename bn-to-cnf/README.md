# bn-to-cnf
Encode Bayesian Networks into Conjunctive Normal Form and write it to output file in DIMACS format.

## Synopsis
The program takes a HUGIN encoded Bayesian Network as input and encodes it into Conjunctive Normal Form with various optimizations, e.g., prime implicates, context-specific independence, determinism, etc.

## Usage
  > ./bn-to-cnf -i \<HUGIN FILE\> [option] [...]

| Option | Optimization |
| --- |--- |
| -p| Partition cnf per CPT|
| -c| Constraints are suppressed|
| -e| Equal probabilities are encoded|
| -d| Determinism are encoded|
| -a| Apply boolean symplification|
| -b| Boolean variables are not mapped|
| -q| Quine-McCluskey (QM)|
| -l \<limit\>| Limit problem size for QM|

| Option | Other |
| --- | --- |
| -i \<filename\>| Input (HUGIN .net file)|
| -w| Write CNF in DIMACS format to file|
| -s| Show stats|
| -h| Help|

## Installation

To install to `<install_dir>`, type

    > mkdir build
    > cd build
    > cmake -DCMAKE_INSTALL_PREFIX:PATH=<install_dir> ..
    > make install

NOTE: This requires the [quine-mccluskey](https://github.com/gisodal/quine-mccluskey) repository! For a stand alone version use [bayes-to-cnf](https://github.com/gisodal/bayes-to-cnf).


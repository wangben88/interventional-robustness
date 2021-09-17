# bw-obdd-to-cnf

## Description
Code for converting a ordered decision diagram (.odd format) to CNF. This is only necessary if the classifier you wish
to verify is specified as an ODD (as is the case with the Bayesian network classifiers + BNC_SDD repo we use in
experiments in the paper); otherwise, you can simply specify the classifier in CNF directly.

Currently only binary predictions (i.e. OBDDs) are fully supported.

## Installation

Enter this subdirectory and run the following commands:

    > mkdir build
    > cd build
    > cmake ..
    > make

## File formats

Two file formats are used:

1. **ODD file** (.odd): See https://github.com/AndyShih12/BNC_SDD for details
2. **CNF file** (.cnf): Outputs a .cnf file representing the classifier/decision function, with some additional details
regarding the mapping from variable names to literals.
    
## Operation

Collect the .odd file you wish to convert to CNF, then run the following:

    > ./bw_obdd_to_cnf -i df.odd -o df.cnf

The input ODD df.odd will be converted into a CNF written to df.cnf.

## Downstream

The CNF file for the decision function df.cnf is used in conjunction with the CNF file for the Bayesian network
bn.cnf (along with any ordering constraints) in combine_cnf, which outputs the joint CNF for the entire system.
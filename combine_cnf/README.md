# combine_cnf

## Description
Combines the CNF representations of the Bayesian network (and, optionally, decision function) into a single CNF. Optimizes the combined CNF for compilation performance, subject to constraints. Outputs 
both the combined CNF and LMAP (literal map) files, ready to be compiled to an arithmetic circuit using C2D

## Installation

Enter this subdirectory and run the following commands:

    > mkdir build
    > cd build
    > cmake ..
    > make

## File formats

Two file formats are used:

1. **CNF file** (.cnf): We take as input CNFs for both the Bayesian network and Decision function. The decision function is optional, and can be omitted. Currently,
it is assumed that the Bayesian network CNF was generated using bn-to-cnf, and the Decision function CNF generated using bw-obdd-to-cnf. 
   The implementation takes advantage of the particular format of the CNF comments that are outputted by these programs;
   as such, it is not currently supported to input arbitrary CNFs, though this is a planned feature.
   
   
2. **Constraints file** (.txt): See constrained-ordering directory for details.

## Operation

Collect the Bayesian network bn.cnf, Decision function df.cnf, ordering constraints modconstraints.txt, and
run (possibly with decision function option omitted):

    > ./combine_cnf -c bn.cnf -d df.cnf -m modconstraints.txt -o combined

The combined CNF and LMAP will be written to combined.cnf and combined.lmap respectively.

## Downstream

The CNF and LMAP can then be compiled using C2D; make sure to use the -dt_method=3 option.

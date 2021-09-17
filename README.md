# IntRob: Provable Guarantees on the Robustness of Decision Rules to Causal Interventions

## Description
Code for the paper "Provable Guarantees on the Robustness of Decision Rules to Causal Interventions" @ IJCAI' 21, which 
provides guarantees on the robustness of classifier performance under distribution shifts.

This code can be used to quantitatively measure (verify) the robustness of classifiers/decision rules when faced
with distribution shifts, specified by causal interventions. This is quantified via the *worst-case* performance 
of the classifier on some metric/evidence **e**, e.g. false positive probability, *across a set of distributions*. In 
particular, given a causal Bayesian network **N**, an "intervention set"**I** specifiying all potential distribution 
shifts, and any classifier **F** whose behaviour is described logically using a CNF formula, it computes tight upper 
and lower bounds on the worst case probability **IntRob**(**I**, **N**, **F**, **e**).

Only discrete variables are supported; however continuous domains can be handled by discretizing variables into buckets.

*Implementation Note: Currently only Bayesian network classifiers compiled using the BNC_SDD repository are supported, 
as the code relies on this pipeline. However in the future the code will be updated to support any classifier (that 
can be converted to CNF, cf. Related Work section in paper).*

*Implementation Note: Currently only upper bounds available; lower bounds will be added very soon.*

## Structure
This repository consists of 5 main directories, which we describe briefly below:
1. **bn-to-cnf**: Converts Bayesian networks specified in HUGIN.net format to CNF. Implementation from
   https://github.com/gisodal/bn-to-cnf
   
2. **bw-obdd-to-cnf**: Converts ordered decision diagrams (.odd) to CNF. .odd files are obtained through the BNC_SDD 
compiler for Bayesian network classifiers @ https://github.com/AndyShih12/BNC_SDD
   
3. **constrained-ordering**: Utility for generating constraint files, which are used in the compilation process. 

4. **combine_cnf**: Combines the Bayesian network CNF with the Decision Function CNF, and optimizes for compilation
performance subject to required ordering constraints.
   
5. **bounding**: Given a compiled arithmetic circuit (AC), computes an upper bound on the interventional robustness.
Based upon the AC evaluator implemented in Ace: http://reasoning.cs.ucla.edu/ace/
   
There is also an **examples** directory containing example BNs and ODDs, as well as some example configuration files
(see below).

Roughly speaking, the pipeline is as follows (much of this is automated by a shell script):

> *.net* BN -> **1** -> *.cnf* BN 
> 
> *.odd* Classifier/Decision Function (DF) -> **2** -> *.cnf* DF
> 
> *.txt* Constraints -> **3** -> *.txt* Modified Constraints
> 
> *.cnf* BN, *.cnf* DF, *.txt* Modified Constraints -> **4** -> *.cnf*, *.lmap* (CNF representation of BN and DF)
>
> *.cnf* (CNF representation of BN and DF) -> **c2d** -> *.ac* (Compiled AC)
> 
> *.ac*, *.lmap* (Compiled AC) + *.txt* (Config file) -> **5** -> Upper Bound


## Installation

First, obtain the c2d compiler from http://reasoning.cs.ucla.edu/c2d/, which is required for this project. Note that 
they provide a 32-bit binary, so will 

Second, compile the code from all of the subdirectories. This can be done quickly using the shell script provided;
otherwise follow the instructions in the README in each subdirectory.

    > bash build.sh

## Operation

The following are required as input:
1. **Bayesian Network** (.net): Represents **N**
2. **Decision Function** (.odd): Represents **F**
3. **Initial Constraints File** (.txt): Represents **I**. This specifies ordering constraints (cf. Sec 4.2. of paper and the README in 
   the constrained-ordering subdirectory). For parametric intervention sets **I**, this can be left as an empty text
   file (as the code will automatically add topological constraints); for structural intervention sets, the appropriate 
   constraints will need to be added.
   
4. **Configuration File** (.txt): Represents **e**, **I**. See README in bounding subdirectory for more details.

Follow the pipeline above. The first 4 steps can be completed using the bash script:

    > bash obtain_joint_cnf.sh -n bn.net -d df.odd -m constraints.txt -o output -t

where *bn.net* is the Bayesian network file, *df.odd* the Decision function file, *constraints.txt* the initial
constraints file, and *output* the directory in which to output the *.cnf* and *.lmap* file (the output directory
must already exist). The output files will be named *combined.cnf* and *combined.lmap*.

For the 5th step, run the c2d compilation as follows (the -dt_method 3 option is crucial):
      
      > ./c2d_linux -in combined.cnf -dt_method 3

This will output a file *combined.cnf.nnf*; rename this to *combined.ac*.

For the 6th step, navigate to the *bounding/bin* directory and simply run:

      > java ace.UpperBound config.txt

where *config.txt* is the configuration file.
# constrained-ordering

## Description

Code for generating variable orderings, which are used in the process of compiling Bayesian networks +
decision functions to arithmetic circuits. Note that the use of these orderings is only necessary for
upper bounding interventional quantities; lower bounding and generic inference queries can be done 
without specific ordering constraints.

## Installation

Enter this subdirectory and run the following commands:

    > mkdir build
    > cd build
    > cmake ..
    > make

## File formats

Three main file formats are used:

1. **Network file** (.net): A Bayesian network, specified in HUGIN .net format
2. **Constraint file** (.txt): Specifies the required ordering constraints (cf. Sec 4.2). These files
consist of lines of the following form:
   > n p_1 ... p_k
   
Each entry represents a node in the network, and is specified using the node name, which must match
the .net file. This file specifies the constraints (p_1, n), ..., (p_k, n). As the naming suggests, 
in a typical use case p_1, ..., p_k will be the parents of n in the Bayesian network. 

3. **Ordering file** (.txt): The first line contains the number of variables. Each subsequent line
   contains a single variable name, with the file as a whole containing a complete ordering 
   of all variables/nodes in the Bayesian network .net file.

## Operation

First, collect the .net Bayesian network you wish to obtain an ordering for, and create a constraint
file. Note that the code automatically adds topological constraints; as a result, you only need to 
specify the following in the constraint file, depending on the type of interventions you wish to
verify:

* **Parametric**: For parametric interventions, you can leave the constraint file blank.
* **Structural**: For structural interventions, for each intervenable value W and its context 
  C(W) = {C_1, ... C_k}, add a line to the constraint file:
  > W C_1 ... C_k
  
Now run the following command:

    > ./constrained-ordering -i bn1.net -c constraints_bn1.txt -o ordering_bn1.txt -m modconstraints_bn1.txt

Here bn1.net is your input Bayesian network, constraints_bn1.txt the input constraint file, 
ordering_bn1.txt the file to write the outputted ordering to, and modconstraints_bn1.txt the file to
write the modified constraints (i.e. added topological constraints) to.

## Downstream

TBD
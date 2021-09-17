# bounding

## Description
Produces upper bounds on interventional probabilities IntRob(I, e), for evidence e (representing
e.g. a false positive classification) and parametric or structural intervention set I.

Requires the following to be specified as input:
1. An arithmetic circuit, which must have been compiled with the correct ordering for the desired intervention set I;
2. The evidence e, which represents the probability of interest;
3. The intervention set W, representing the variables which can be intervened on

This implementation is an adapted version of the AC evaluator included in Ace: http://reasoning.cs.ucla.edu/ace/

## Installation

Enter the *bin* subdirectory and run the following commands:

    > javac -d . ../src/ace/*.java

## File formats

The following file formats are used:

1. **Configuration file** (.txt): The program takes as input a configuration file, with the following format. The first line specifies the location of the .ac file. The second line specifies the location of the .lmap file. The third line gives the number of variables **M** for which evidence is specified. Each of the following **M** lines contains a variable instantiation, consisting of the variable name, followed by any number of variable values, all separated by spaces (e.g. MedCost 0 1 3 specifies MedCost = 0 OR 1 OR 3). Then, the following line gives the number of variables **N** in the intervention set W. Each of the following **N** lines contains a variable name, enumerating the intervention set. 

Note that the variable name "Sink" refers to the output of the classifier.
2. **AC file** (.ac) This is output by c2d, and must be specified in the configuration file.
3. **Literal Map file** (.lmap): This is output by c2d, and must be specified in the configuration file.

## Operation

Navigate to the *bin* subdirectory, and run the following:

    > java UpperBound.java config.txt

where "config.txt" is the configuration file.

## Downstream

The program will output the computed upper bound to the command line.
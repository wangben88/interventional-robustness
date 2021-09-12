# bw-obdd-to-cnf

## Description
This repo compiles a classifier (Bayesian network classifier) into CNF, combining it with the CNF representation of a Bayesian network.
The combined CNF can then be compiled (e.g. using c2d) into an AC.

This repo contains 2 other repos (from other public GitHub repos) as submodules:

* BNC_SDD: Compiles a BNC (as specified by a config file, and a .net Bayesian network file) to an OBDD/SDD. This has already been compiled and contains the executable.
* bn-to-cnf: Compiles a Bayesian network (.net) to CNF. Note that the resulting .cnf file has a very specific format in its comments, which contains important information.

My code (bw-obdd-to-cnf) takes as input the .bdd file from BNC_SDD, and .cnf from bn-to-cnf. It compiles the .bdd into CNF, and then combines it with the CNF 

## Installation

First off, you will need to compile the bn-to-cnf repo. Enter this subdirectory and run the following commands (for your choice of install_dir):

    > mkdir build
    > cd build
    > cmake -DCMAKE_INSTALL_PREFIX:PATH=<install_dir> ..
    > make install

Now you will need to compile the bw-obdd-to-cnf code. From the base directory, run:

    > cmake -Bbuild -H.
    > cmake --build build --target all
    
## Operation

Start with a Bayesian network file (.net): some of these are available in BNC_SDD/networks.

First, compile into an OBDD using the executable in BNC_SDD. Keep in mind that you need to change the config file appropriately to specify the classifier you are defining on the BN.
Note that you might get errors in the sdd compilation; this is fine, so long as the ODD (.odd) file is produced.

Second, compile the network to CNF using the built bn-to-cnf executable.

Finally, take the a.odd and b.cnf file and call:

    > ./bw-obdd-to-cnf -i a.odd -c b.cnf -o output.cnf
    
where the -o option determines where to output the file.
    
Optionally, you can also add the option:

    > -d c.txt
   
where c.txt is an file specifying the desired ordering of source variables - this affects how the CNF is later compiled to AC.

## Downstream

To compile to d-DNNF/AC, use the c2d tool. In particular, use option -dt_method 3, this will ensure that the AC splits on the source variables first and in the desired ordering.


## Other Notes

Note that it's not necessary that the BNC classifier uses the BN which models the distribution. For instance, if the variable we want to predict is not a root but rather a leaf of the Bayesian network, then it is not possible to directly use a BNC. However, if we use a different BN where it is the root, for the purposes of designing a BNC, we can then use the resulting compiled BNC normally, so long as the variables are named the same in both BNs.

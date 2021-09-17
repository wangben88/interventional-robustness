#!/bin/bash
add_top=false
while getopts "htn:d:m:o:" opt; do
  case ${opt} in
    h )
      echo "Usage: ./run_compilation.sh -n bn.net -d df.odd -m constraints.txt"
      echo "	-n <filename>: Input Bayesian network file (HUGIN .net format)"
      echo "	-d <filename>: Input Decision Function (.odd)"
      echo "	-m <filename>: Constraint file (.txt)"
      echo "    -t: Add all topological constraints"
      echo "	-o <dirname>: Output directory"
      exit 0
      ;;
    t )
      add_top=true;;
    n )
      bn_file=$OPTARG;;
    d )
      df_file=$OPTARG;;
    m )
      constraint_file=$OPTARG;;
    o )
      out_dir=$OPTARG;;
  esac
done

echo "Creating Bayesian Network CNF..."
bn-to-cnf/cmake-build/bn-to-cnf -i $bn_file -w "${out_dir}/bn.cnf"
echo "Done"
echo "Adjusting Constraints..."
if [ "$add_top" = true ]
then
  constrained-ordering/cmake-build/constrained_ordering -i "$bn_file" -c "$constraint_file" -o "${out_dir}/ordering.txt" -m "${out_dir}/modconstraints.txt"
else
  touch "${out_dir}/modconstraints.txt"
fi
echo "Done"
echo "Converting Decision Function to CNF..."
bw-obdd-to-cnf/cmake-build/bw_obdd_to_cnf -i "$df_file" -o "${out_dir}/df.cnf"
echo "Done"
echo "Combining BN and DF CNF..."
combine_cnf/cmake-build/combine_cnf -c "${out_dir}/bn.cnf" -d "${out_dir}/df.cnf" -m "${out_dir}/modconstraints.txt" -o "${out_dir}/combined"
echo "Done"

#!/bin/sh
echo Building bn-to-cnf...
echo =============================================================
mkdir bn-to-cnf/cmake-build
cd bn-to-cnf/cmake-build
cmake ..
make
cd ../..
echo

echo Building bw-obdd-to-cnf...
echo =============================================================
mkdir bw-obdd-to-cnf/cmake-build
cd bw-obdd-to-cnf/cmake-build
cmake ..
make
cd ../..
echo

echo Building combine_cnf...
echo =============================================================
mkdir combine_cnf/cmake-build
cd combine_cnf/cmake-build
cmake ..
make
cd ../..
echo

echo Building constrained-ordering...
echo =============================================================
mkdir constrained-ordering/cmake-build
cd constrained-ordering/cmake-build
cmake ..
make
cd ../..
echo

echo Building bounding...
echo =============================================================
cd bounding/bin
javac -d . ../src/ace/*.java
cd ../..
echo

echo Done

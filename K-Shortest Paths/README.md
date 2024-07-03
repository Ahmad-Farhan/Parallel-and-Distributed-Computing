# K Shortest Path Problem
This project focuses on addressing the Kth Shortest Path Problem using a customized version of Dijkstra Single Pair Shortest Path Algorithm. It includes both serial and parallel versions of the algorithm. Following are the instructions to compiling and executing the programs.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Compilation](#compilation)
- [Execution](#execution)
- [License](#license)

## Prerequisites

To compile and run this project, you'll need the following:
- Operating System (e.g., Linux)
- C++ Compiler (e.g., GCC)
- Libraries (e.g., STL C++)

Parallelization Libraries: <br>
• OpenMP (Version 5.2): For shared memory parallelization<br>
• MPI (Version 3.3.2): For distributed memory parallelization <br>

## Compilation

### File Preprocessing 
Compile the graph data file preprocessing  algorithm using: <br>
\`\`\`
g++ -o preprocess preprocess.cpp
\`\`\`

### Serial Version
Compile the serial version of the algorithm using: <br>
\`\`\`
g++ -o serial serial.cpp
\`\`\`

### Parallel Version
Compile the parallel version of the algorithm using: <br>
\`\`\`
mpic++ -o parallel parallel.cpp
\`\`\`

## Execution

### File Preprocessing
You may use a txt or csv file as input <br>
Execute the graph data file preprocessing  algorithm using: <br>
\`\`\`
./preprocess \<graph_filename\>
\`\`\`

For csv input files, 3 meta files will be generated:
1. \<original_filename>-grf.txt: Containing formatted graph edge list
2. \<original_filename>-enc.txt: Containing nodes encoding
2. \<original_filename>-sdn.txt: Containing 10 random pairs of source and destination nodes for testing

For txt input files, 2 meta files will be generated (i.e., assuming txt files will be in encoded format):
1. \<original_filename>-grf.txt: Containing formatted graph edge list
2. \<original_filename>-sdn.txt: Containing 10 random pairs of source and destination nodes for testing

### Serial Version
\`\`\`
./serial \<formatted_graphfile\>-grf.txt \<k-value\> \<enc\>
\`\`\`

1. \<formatted_graphfile>-grf.txt contains the graph edge list in correct format with required meta data
2. \<k-value> defines the number of shortest paths required
3. \<enc> For displaying path nodes with original node names. Specifically for csv files.

### Parallel Version
\`\`\`
mpiexec -np \<num_processes\> ./parallel \<formatted_graphfile\>-grf.txt \<k-value\> \<enc\>
\`\`\`

1. \<num_processes> defines number of mpi processes (generally, equal to number of nodes in cluster)
2. \<formatted_graphfile>-grf.txt contains the graph edge list in correct format with required meta data
3. \<k-value> defines the number of shortest paths required
4. \<enc> For displaying path nodes with original node names. Specifically for csv files.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
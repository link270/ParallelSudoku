#An application writted in C++ using MPI to solve a sudoku puzzle in parallel with any number of processors.

### Compile with MPI:

`mpic++ sudoku.cpp`

### Run:

`mpirun -np 4 ./a.out 3`

Where 4 is the number of processors your cpu has and 3 is the number of puzzles you want to generate and solve.


Written by Mitch Shelton and Ivon Saldivar.
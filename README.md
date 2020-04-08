# Parallel Sudoku Generator and Solver

**An application writted in C++ using MPI to solve a sudoku puzzle in parallel with any number of processors.**

This program can generate a sudoku puzzle and solve it in parallel. It can generate typical puzzle of a 9X9 grid, or something larger like 4X4 or 5X5, etc.

### Compile with MPI:

`mpic++ sudoku.cpp`

### Run:

`mpirun -n 4 ./a.out`

*Where 4 is the number of processors your cpu has.*
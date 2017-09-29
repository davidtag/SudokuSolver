# SudokuSolver
A C++ console application to solve and generate Sudoku puzzles.

# Instructions
Run the Solver.exe file with the puzzle.txt file in the same directory. Puzzles of different difficulties are also provided. Simply change the title of the desired text file to puzzle.txt to have it imported by the solver.

# About
In the context of Sudoku as a Constraint Satisfaction Problem (CSP), the algorithm uses a backtracking search with constraint propagation (with use of a Minimum Remaining Values (MRV) variable-ordering heuristic and random assignment for value-ordering) to satisfy the Sudoku constraints.

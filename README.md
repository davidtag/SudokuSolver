# SudokuSolver
An efficient C++ console application to solve and generate Sudoku puzzles.

## Instructions ##
Run the Solver.exe file with the puzzle.txt file in the same directory. Puzzles of different difficulties are also provided. Simply change the title of the desired text file to puzzle.txt to have it imported by the solver.

The sample files also demonstrate the input format expected by the solver. This format can be used to input any puzzle

## About ##
Formally, Sudoku can be represented as a Constraint Satisfaction Problem (CSP). A common method to solve CSPs is based on search. I built this solver while an undergrad studying Math/Stats, before formally studying search methods or CSPs (which I have since studied as a CS grad student). The solver dynamically interleaves logical deductions to determine cell values (in a manner akin to how a human solves Sudoku) with search. The search portion uses a minimum remaining values (MRV) variable-ordering heuristic with random value-ordering.

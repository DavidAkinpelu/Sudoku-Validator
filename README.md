# Multithreaded Sudoku Validator
This program reads a 9x9 sudoku puzzle from a file and evaluates the sudoku grids using 11 threads to determine whether the solutions are valid or invalid.The evaluation is done in the following manner:
```sh
    -> thread 1 evaluates the rows
    -> thread 2 evaluates the columns
    -> the remaining 9 threads evaluate the 9 subgrids of the sudoku puzzle
```

A binary semaphore is use for thread synchronzation while accessing the shared variable and also generating output on the console.

## To compile and Execute

```sh
$ g++ sudoku.cpp -o sudoku -lpthread
$ ./sudoku ./sudoku.txt
```

## Input
The input file contains the 9x9 sudoku puzzle to evaluate
```sh
6 2 4 5 3 9 1 8 7
5 1 9 7 2 8 6 3 4
8 3 7 6 1 4 2 9 5
1 4 3 8 6 5 7 2 9
9 5 8 2 4 7 3 6 1
7 6 2 3 9 1 4 5 8
3 7 1 9 5 6 8 4 2
4 9 6 1 8 2 5 7 3
2 8 5 4 7 3 9 1 6
```

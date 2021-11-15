/*
Description:

This programs takes a sudoku puzzle as input and check if
the puzzle is valid or not. It uses a total 11 threads.
thread 1 and 2 are responsibile for checking if the rows and columns 
are valid respectively. The reamining 9 threads are used for checking if each 
subgrids of 3x3 are vaild. The threads are synchronize using a binary semaphore
that ensures that we don't have race conditions while generating the 
output to the console and updating the shared array varibable. Each thread returns a 
integer value of 1 indicating that the corresponding part of the puzzle they are 
handling is valid and zero if invalid.
*/

#include <fstream>
#include <pthread.h>
#include <iostream>
#include <cstdlib>
#include <semaphore.h>

using namespace std;

// number of threads
#define num_threads 11

int result[27] = {0};

// binary semaphore for threads synchronization
sem_t mutex; 
/**
 * Structure that holds the parameters passed to a thread.
 */
typedef struct
{
    // The starting row.
    int row;
    // The starting column.
    int col;

    // The thread index
    int threadindex;

    // The pointer to the sudoku puzzle.
    int (* board)[9];

} parameters;

// Declaration for 3x3 square function.
void *check_grid(void *params);

// Declaration for the check_rows function.
void *walk_rows(void *params);

// Declaration for the check_cols function.
void *walk_cols(void *params);


int main(int argc, char** argv){
	if(argc < 2){
		cout<< "Usage: executable-name [input-file]" << endl;
		return 1;
	}
    int sudoku[9][9];
    ifstream filename(argv[1]);
    if(! filename){
        cout << "Error, file couldn't be opened" << endl;
        return 1;
    }
    for (int row=0; row<9; row++){
        for (int column=0; column<9; column++){
            filename >> sudoku[row][column];
            if (! filename){
                cout<< "Error while reading file at "<< row << ", " << column << endl;
           }
        }
    }

    // initilaize array of threads
    pthread_t threads[num_threads];

    // initilaize binary semaphores
    sem_init(&mutex, 0, 1);

    // initialize the thread index for the subgrids
    int threadIndex = 2;

    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            // ====== Declaration of the parameter for the row check threads =======
            if (j == 0)
            {
                parameters *rowData = (parameters *) malloc(sizeof(parameters));
                rowData->row = i;
                rowData->col = j;
                rowData->board = sudoku;
                rowData->threadindex = 0;
                pthread_create(&threads[0], NULL, walk_rows, rowData);
            }

            // ====== Declaration of the parameter for the column check threads =======
            if (i == 0)
            {
                parameters *columnData = (parameters *) malloc(sizeof(parameters));
                columnData->row = i;
                columnData->col = j;
                columnData->board = sudoku;
                columnData->threadindex= 1;
                pthread_create(&threads[1], NULL, walk_cols, columnData);

            }
            // ====== Declaration of the parameter for the 3X3 grid check threads =======
            if (i%3 == 0 && j%3 == 0)
            {
                parameters *gridData = (parameters *) malloc(sizeof(parameters));
                gridData->row = i;
                gridData->col = j;
                gridData->board = sudoku;
                gridData->threadindex = threadIndex;
                pthread_create(&threads[threadIndex++], NULL, check_grid, gridData);
            }
        }
    }

    // wait for all threads to finish their tasks
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    // destroy the binary semaphore
    sem_destroy(&mutex);

    for (int i = 0; i < num_threads; i++)
    {
        if (result[i] == 0){

            cout << "Sudoku solution is invalid" << endl;
        }
    }

    cout << "Sudoku solution is valid" << endl;
}

/*
check if a subgrid of size 3x3 contains all numbers from 1 to 9
An array called saved[10] is initilaized to 0.
once a number is found, the value in that index is changed to 1.
if value in the array is already 1, then it means the value is repeating therefore 
the solution is invalid,  hence valid. The first 9 indices of the result[] corresponds
to the 9 subgrids to be checked. If valid, change the value in the appropriate index to 1.
The binary semaphore mutex ensures the thread is the only thread in the critical section 
while generating output to the console and updating the global variable result[] in the 
parent thread

The thread id, associated row and column indices and valid or invalid whether the case are 
generated as output to the console.
*/
void * check_grid(void * params) {
    parameters * data = (parameters *) params;
    int startRow = data->row;
    int startCol = data->col;
    int threadid = data->threadindex;
    int saved[10] = {0};
    for (int i = startRow; i < startRow + 3; ++i) {
        for (int j = startCol; j < startCol + 3; ++j) {
            int val = data->board[i][j];
            if (saved[val] != 0) {
                sem_wait(&mutex);
                cout << "thread " << threadid+1<< ", subgrid R" 
                <<startRow << startRow+1<< startRow+2<< "-C"
                <<startCol <<startCol+1 <<startCol+2 << ", invalid" << endl;
                sem_post(&mutex);
                return (void *) 0;
            }
            else{
                saved[val] = 1;
            }
        }
    }
    sem_wait(&mutex);
    cout << "thread " << threadid+1<< ", subgrid R" 
    <<startRow << startRow+1<< startRow+2<< "-C"
    <<startCol <<startCol+1 <<startCol+2 << ", valid" << endl;
    result[startRow + startCol/3] = 1;
    sem_post(&mutex);
    return (void *) 1;
}

/*
Check if each row contain numbers from 1 to 9.
An array called saved[10] is initilaized to 0.
once a number is found, the value in that index is changed to 1.
if value in the array is already 1, then it means the value is repeating therefore 
the solution is invalid,  hence valid. The 9 to 17 indices of the result[] corresponds
to the 9 rows to be checked. If valid, change the value in the appropriate index to 1.
The binary semaphore mutex ensures the thread is the only thread in the critical section 
while generating output to the console and updating the global variable result[] in the 
parent thread.

The thread id, row number and valid or invalid whether the case are generated as output
to the console.
*/

void * walk_rows(void * params) {
    parameters * data = (parameters *) params;
    int row = data->row;
    int threadid = data->threadindex;
    int validarray[10] = {0};
    for (int i= 0; i < 9; i++) {
        int val = data->board[row][i];
        if (validarray[val] != 0) {
            sem_wait(&mutex);
            cout << "thread "<< threadid+1 << ", row" << row << ", invalid" <<endl;
            sem_post(&mutex);
            return (void *) 0;
        }
        else{
            validarray[val]= 1;
            }
    }
    sem_wait(&mutex);
    cout << "thread "<< threadid+1 << ", row" << row << ", valid" <<endl;
    result[9+row] = 1;
    sem_post(&mutex);
    return (void *) 1;
}

/*
Check if each column contain numbers from 1 to 9.
An array called saved[10] is initilaized to 0.
once a number is found, the value in that index is changed to 1.
if value in the array is already 1, then it means the value is repeating therefore 
the solution is invalid,  hence valid. The 18 to 26 indices of the result[] corresponds
to the 9 columns to be checked. If valid, change the value in the appropriate index to 1.
The binary semaphore mutex ensures the thread is the only thread in the critical section 
while generating output to the console and updating the global variable result[] in the 
parent thread.

The thread id, column number and valid or invalid whether the case are generated as output
to the console.
*/

 void * walk_cols(void * params) {
    parameters * data = (parameters *) params;
    int col = data->col;
    int threadid = data->threadindex;
    int validarray[10] = {0};
    for (int i =0; i < 9; i++) {
        int val = data->board[i][col];
        if (validarray[val] != 0) {
            sem_wait(&mutex);
            cout << "thread "<< threadid+1 << ", column" << col << ", invalid" <<endl;
            sem_post(&mutex);
            return (void *) 0;
            }
            else{
                validarray[val] = 1;
            }
    }
    sem_wait(&mutex);
    cout << "thread "<< threadid+1 << ", column" << col << ", valid" <<endl;
    result[19 + col] = 1;
    sem_post(&mutex);
    return (void *) 1;
}

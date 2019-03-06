/*	
 *	Colin Crumrine 
 *	Student ID: 570179634
 *	Date: 3/5/2019
 *	File Contents:
 *	This is the main file for Project 1. This file contains all functions, including main,
 *	used in the ShearSort algorithm solution.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <signal.h>
#include <pthread.h> 
#include <math.h>
#include <limits.h>

/*	
 *	Declaring:
 *	- global array arr
 *	- phase for counting the number of phases run
 *	- rowCount and colCount for tracking the number rows/cols thatve been sorted
 */
int** arr;
int phase = 1 , N, rowCount = 1, colCount = 1;
pthread_t* threads;
sem_t* sems;

// Function headers
void readFile();
void printArr();
void swap(int *xp, int *yp);
void rowBubbleSort(int r);
void colBubbleSort(int c); 
void* dispatch();

int main(int argc, char* argv[])
{
	// Read file and populate array arr and set N to the value of sqrt(array.size)
	readFile();
	printf("INTIIAL ARRAY VALUES:\n-------------------\n");
	printArr();
	printf("-------------------\n");
	int i;
	// After N is set, populate sems and threads with N sems\threads
	sems = malloc(N * sizeof(sem_t));
	threads = malloc (N * sizeof(pthread_t));
	// Initialize N semaphores and create N pthreads all calling dispatch()
	for(i = 0; i < N; i++) { sem_init(&sems[i], 0, 1); }
	for(i = 0; i < N; i++) { pthread_create(&threads[i], NULL, dispatch, NULL); }
	// Once the array has been sorted, join all threads to ensure 
	// all have finished and destroy semaphores
	for(i = 0; i < N; i++) { pthread_join(threads[i], NULL); }
	for(i = 0; i < N; i++) { sem_destroy(&sems[i]); }
	return 0;
}

/*
 *	Role: readFile() reads file input.txt and checks to see if the integers can be represented
 *		  in square format as specified. If it can, array arr gets populated with values from the
 *		  file If it can't, the function calls exit(0).
 *
 *	Parameters: NONE
 *	
 *	Return Values: NONE
 *
 */

void readFile()
{
	FILE *fp;
	fp = fopen("input.txt", "r");
	int i,j;
	int done = 0, flag, holder;
	double totalElts;
	for(i = 0; i < INT_MAX; i++)
	{
		for(j = 0; j < INT_MAX; j++)
		{
			 flag = fscanf(fp, "%d", &holder);
			 if(flag == EOF)
			 {
				done = 1;
				break;
			 }else{
				 totalElts++;
			 }
		}
		if(done == 1){break;}
	}
	fclose(fp);
	
	double tempN = sqrt(totalElts);
	
	if(floor(tempN) < tempN && tempN < ceil(tempN))
	{
		printf("Integers not in form of square. Please ensure the correct format is used.\n");
		exit(0);
	}else{
	N = tempN;
    arr = (int **)malloc(N * sizeof(int *)); 
    for (i=0; i<N; i++){arr[i] = (int *)malloc(N * sizeof(int));}
	fp = fopen("input.txt", "r");
	for(i = 0; i < N; i++)
	{
		for(j = 0; j < N; j++)
		{
			 flag = fscanf(fp, "%d", &arr[i][j]);
		}
	}
	fclose(fp);
		
	}
}

/*
 *	Role: printArr() iterates through array arr and prints out the values, from 0 to N as indexes
 *
 *	Parameters: NONE
 *
 *	Return Values: NONE
 *
 */

void printArr()
{
	int i,j;
	for(i = 0; i < N; i++)
	{
		for(j = 0; j < N; j++)
		{
			printf("|%d| ", arr[i][j]);
		}
		printf("\n");
	}
	
}

/*
 *	Role: swap(int *xp, int *yp) swaps the values of the 2 integer references it gets passed
 *
 *	Parameters: 2 references to 2 integers, xp and yp
 *
 *	Return Values: NONE
 *
 */

void swap(int *xp, int *yp) 
{ 
    int temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
}

/*
 *	Role: rowBubbleSort(int r) takes in a value, r, which is used to determine which row to sort
 *		  and in which orientation. If the row is an even number, then the row will get sorted in
 *		  ascending order, if not, it gets sorted in descending order.
 *
 *	Parameters: int r
 *
 *	Return Values: NONE
 *
 */

void rowBubbleSort(int r) 
{ 
   int i, j;
   if(r % 2 == 0)
   {
	    for (i = 0; i < N-1; i++)
		{
		   for (j = 0; j < N-i-1; j++)
		   {
				if (arr[r][j] > arr[r][j+1]){ swap(&arr[r][j], &arr[r][j+1]); }
		   }		   
		}
	  
   }else{
	   
		for (i = N-1; i > 0; i--)
		{
		   for (j = N-i; j > 0; j--)
		   {
				if (arr[r][j] > arr[r][j-1]){ swap(&arr[r][j], &arr[r][j-1]); }
		   }		   
		}		
   }
  	   
}

/*
 *	Role: colBubbleSort(int c) takes in a value, c, which is used to determine which column to sort.
 *
 *	Parameters: int c
 *
 *	Return Values: NONE
 *
 */

void colBubbleSort(int c)
{ 
   int i, j; 
   for (i = 0; i < N-1; i++)
   {
	   for (j = 0; j < N-i-1; j++)
	   {
			if (arr[j][c] > arr[j+1][c]){ swap(&arr[j][c], &arr[j+1][c]); }
	   }		   
   }	   
}

/*
 *	Role: dispatch() is the function that gets called by the threads generated in main.
 *		  This function either performs a column sort when the phase is even, or a row sort
 *		  when the phase is odd. The semaphore blocks finished process from moving onto the
 *		  next phase until after all processes have exited. This ensures that no process will
 *		  be modifying a section of the array arr until after the phase has completed.
 * 
 *		  Once a phase has completed, indicated by the global variables rowCount and colCount,
 *	      the final calling thread will print the newly sorted array, increment phase and set the
 *		  count of the appropriate counter variable to 1.
 *
 *	Parameters: NONE
 *
 *	Return Values: NONE
 *
 */

void* dispatch()
{
	while(phase <= (log2(N * N) + 1))
	{
	sem_wait(&sems[0]);
	
	if(phase % 2 == 0)
	{
		colBubbleSort(colCount - 1);
		if(colCount == N)
		{
			printf("PHASE: %d -----> Columns have been sorted\n-------------------\n", phase);
			printArr();
			printf("-------------------\n");
			phase++;
			colCount = 0;
		}
		colCount++;
	}else{
		rowBubbleSort(rowCount - 1);
		if(rowCount == N)
		{
			printf("PHASE: %d -----> Rows have been sorted\n-------------------\n", phase);
			printArr();
			printf("-------------------\n");
			phase++;
			rowCount = 0;
		}
		rowCount++;
	}
	
	sem_post(&sems[0]);
	}
	
	pthread_exit(NULL);
}
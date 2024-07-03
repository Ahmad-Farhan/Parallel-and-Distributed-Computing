// Name: Ahmad Farhan
// Roll no. 21I-1366
// Assignment 2
//==================================================
// Integer Array List Implementation
//--------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Structure for the list
struct Listint {
	int* arr;	// Elements Array
	int size;	// No of Elements
	int cap;	// Max Capacity
};
// Alias for struct Listint as list
typedef struct Listint list;

// Function to create new list of defined size
list new_list(const int size){
    list newList;
	newList.arr = (int*)malloc(sizeof(int)*size);
	newList.cap = size;
	newList.size = 0;
	return newList;
}

// Function to deallocate memory of list
void del_list(list* list){
	free(list->arr);
}

// Function to copy elements of one array to another
void copyi(int* arr1, int* arr2, int size) {
	for (int i = 0; i < size; i++)
		arr1[i] = arr2[i];
}

// Function to Reallocate a Larger Array
void relloci(list* array) {
	int new_cap = array->size * 2 + 1;
	int* new_arr = (int*)malloc(sizeof(int)*new_cap);
	copyi(new_arr, array->arr, array->size);

	free(array->arr);
	array->arr = new_arr;
	array->cap = new_cap;
}

// Function to Insert Element to End of List
void push(list* array, int val) {
	if (array->size + 1 > array->cap)
		relloci(array);
	array->arr[array->size] = val;
	array->size += 1;
}

// Function to Remove Element from End of List
void pop(list* array) {
	array->size -= 1;
}

// Function to display an integer array
void disp(int* arr, const int size){
	for(int i = 0; i < size; i++) 
		printf("%d ", arr[i]);
	printf("\n");
}

// Function to display list
void displayl(list array) {
	disp(array.arr, array.size);
}
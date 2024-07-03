// Name: Ahmad Farhan
// Roll no. 21I-1366
// Assignment 1
//==================================================

#define _GNU_SOURCE
#include "clist.h"
#include <pthread.h>

#define delimiter "%d\n"    // Element Delimiter in Data File
#define cores 8     // Number of CPU cores 


int serial(int* array, const int size);
int parallel(int* array, const int size);

int readElements(const char*, int**);
const char* inputFileName(const char* buffer);


// Pass Filename from terminal as ./exe filename.txt
// Or Provide Filename as Input
int main(int argc, char*argv[]) {
    int* array;
    const char *filename, buffer[100];
    if(argc > 1 ) filename = argv[1];           // Filename Passed from Terminal 
    else filename = inputFileName(buffer);      // Filename Taken as Input

    int size = readElements(filename, &array);   // Read elements from a file
    printf("Initializing List of Size: %d\n", size);

    serial(array, size);
    parallel(array, size);

    return -1;
}




//==============================================================================//
// Serial Versions Implementation                                               //
//------------------------------------------------------------------------------//

void readRollNumbers(FILE*, int*, int);
void addRollNumbersToList(struct Node**, int*, int);
struct Node* mergeSort(struct Node*);

// Main Function for Serial Execution
int serial(int* array, const int size) {
    struct timeval st, et;
    struct Node* list = NULL;
    
    gettimeofday(&st, NULL);                    //Start Time
    addRollNumbersToList(&list, array, size);   //Add elements to list
    list = mergeSort(list);                     //Perform Merge Sort
    gettimeofday(&et, NULL);                    //End Time
    
    printf("\nSerial Execution : \n");
    printf("---------------------------\n");
    printf("Total Time: %.3lf ms\n", calduration(et, st, false)); //Calculate Interval
    
    printf("Sorted List(First 50 Elements):\n");
    display(list);          //Display Sorted List

    return -1;   // Exit Program
}

//=================================================

// Function to read roll numbers from file
void readRollNumbers(FILE* inputFile, int* Numbers, int num) {
   fscanf(inputFile, delimiter, &Numbers[num]);
}

// Function to add roll numbers to the linked list
void addRollNumbersToList(struct Node** head, int* Numbers, int num) {
    for (int i = 0; i < num; i++)
        append(head, Numbers[i]);
}

// Serial Merge Sort on a Linked List
struct Node* mergeSort(struct Node* list) {
    if (!list || !list->next)
        return list;

    struct Node* left = list;
    struct Node* right = getmid(list);          // Split the List in Half
    left = (struct Node*) mergeSort(left);      // Sort Left Half 
    right = (struct Node*) mergeSort(right);    // Sort Right Half
    
    return merge(left, right);        // Merge two sorted lists
}





//==============================================================================//
// Parallel Versions Implementation                                             //
//------------------------------------------------------------------------------//

void* addRollNumbersToListParallel(void*);
void* mergeSortParallel(void*);
void setAffinity(pthread_t, int);

struct Node* mergeSortList(int*, const int);

// Main Function for Parallel Execution
int parallel(int* array, const int size) {
    struct timeval st, et;
    gettimeofday(&st, NULL);                         // Start Time
    struct Node* list = mergeSortList(array, size);  // Perform parallel merge sort on Lists
    gettimeofday(&et, NULL);                         // End Time

    printf("\nParallel Execution : \n");
    printf("---------------------------\n");
    printf("Total Time: %.3lf ms\n", calduration(et, st, false)); //Calculate Interval
    
    printf("Sorted List(First 50 Elements):\n");
    display(list);          //Display Sorted List

    return -1;   // Exit Program

}

//=================================================

// Function to set CPU affinity for a thread
void setAffinity(pthread_t thread, int coreid){
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(coreid, &cpuset);
    if(pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset) !=0)     
        perror("Error ");
}


// Function to display CPU affinity of a thread
void viewAffinity(pthread_t thread_id){
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    pthread_getaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset);

    for(int j = 0; j < cores; j++)
        if (CPU_ISSET(j, &cpuset)) 
            printf("\tCPU %d\n", j);
}

#include <unistd.h> 
// Function to set CPU Affinity of Process
void setSchedAffinity(int coreid){
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(coreid, &cpuset);
    if(sched_setaffinity(getpid(), sizeof(cpu_set_t), &cpuset) !=0)     
        perror("Error:");
}

// Structure to hold data for each thread
struct Thread {
    int* nums;
    int size;
};

// Function to initialize thread data
void initThread(struct Thread* thread, int* array, int divSize){
    thread->nums = array;
    thread->size = divSize;
}

// Thread function to add roll numbers to the list in parallel
void* addRollNumbersToListParallel(void* arg){
    struct Thread* args = (struct Thread*)arg;
    struct Node* list = NULL, *curr = NULL;
    int* nums = args->nums, size = args->size;
    //printf("|%d|", size);

    
    // Convert Array Data into Lists
    curr = append(&list, nums[0]);
    for (int i = 1; i < size; i++)
        curr = append(&curr, nums[i]);
    
    return list;                 // Perform Merge Sort        
}

// pthread_mutex_t mutex;
// Main Thread Function: converts arrays to lists, returns sorted lists 
void* sortAsLists(void* args) {
    // View CPU affinity of the current thread
    // pthread_mutex_lock(&mutex);
    // viewAffinity(pthread_self());
    // pthread_mutex_unlock(&mutex);

    struct Node* list = NULL;
    list = addRollNumbersToListParallel(args);
    return mergeSortParallel(list);
}

// Function to Create Threads, create Lists and return sorted List
struct Node* mergeSortList(int* array, const int size) {
    int divSize = size / cores;
    pthread_t thread_ids[cores];
    struct Thread threads[cores];

    setSchedAffinity(0);
    // Divide the array into segments and create threads
    for(int i = 0; i < cores; i++){
        initThread(&threads[i], array + i * divSize, divSize);
        if(i == cores - 1) threads[i].size += size - divSize*cores;

        int status = pthread_create(&thread_ids[i], NULL, 
                sortAsLists, (void*)&threads[i]);
        
        setAffinity(thread_ids[i], i); // Set CPU affinity for each thread
        if(status) perror("Error: ");
    }

    // Join threads and merge the sorted lists
    struct Node *res = NULL, *list;
    for(int i = 0; i < cores; i++){
        pthread_join(thread_ids[i], (void**)&list);
        res = merge(res, list);
    }
    return res; // Return the Sorted List
}

// Parallel Merge Sort on a Linked List
void* mergeSortParallel(void* args) {
    struct Node* list = (struct Node*)args;
    if (!list || !list->next)
        return list;

    struct Node* left = list;
    struct Node* right = getmid(list);                  // Split the List in Half
    left = (struct Node*) mergeSortParallel(left);      // Sort Left Half
    right = (struct Node*) mergeSortParallel(right);    // Sort Right Half
    
    return merge(left, right);          // Merge Halves as Sorted Lists
}




//==============================================================================//
// General IO Functions                                                         //
//------------------------------------------------------------------------------//

// Function to read elements from file and store them in an array
int readElements(const char* filename, int** array) {
    int size = countElements(filename);     // Count Number of elements in the file
    *array = generateBuffer(size);          // Allocate Memory for the data

    FILE* file = fopen(filename, "r");      // Open Data File
    if(file == NULL) {
        perror("Error:");       // Print error message if file opening fails
        exit(1);
    }
    for (int i = 0; i < size; i++)
       readRollNumbers(file, *array, i);    // Read elements from file
    fclose(file);               // Close the file
    
    return size;                // Return the size of the array
}

// Function to take filename input
const char* inputFileName(const char* buffer){
    printf("Enter Filename: ");
    scanf("%s", (char*)buffer);
    return buffer;
}
// Name: Ahmad Farhan
// Roll no. 21I-1366
// Assignment 1
//==================================================
// LinkedList Implementation
//--------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>

// Structure for Linked List Node 
struct Node {
    int val;
    struct Node* next;
};

// Function to create and initialize a new List Node
struct Node* node(const int val){
    struct Node* newNode= (struct Node*) malloc(sizeof(struct Node));
    if(newNode != NULL){
        newNode->next = NULL;
        newNode->val = val;
    }
    return newNode;
}

// Function to append a value as a Node to end of List
struct Node* append(struct Node** head, const int value) {
    if (*head == NULL){
        *head = node(value);
        return *head;
    } else {
        struct Node* curr = *head;
        while (curr->next != NULL) 
            curr = curr->next;
        curr->next = node(value);
        return curr->next;
    }
}

// Function to insert a new Node to end of List
struct Node* insert(struct Node** tail, struct Node* head) {
    if(*tail == NULL) *tail = head; 
    else (*tail)->next = head;
    return head;
}

// Function to Display the first 100 Elements of List
void display(struct Node* head) {
    int limit = 50;
    while (head != NULL) {
        printf("%d ", head->val);
        head = head->next;
        
        if (--limit == 0)
            { printf("...Output Truncated"); break;}
    }
    printf("\n");
}

// Function to Remove and Return the top Node of List
struct Node* poptop(struct Node** head) {
    struct Node* res = *head;
    *head = (*head)->next;
    res->next = NULL;
    return res;
}

// Function to check if List is Empty
bool empty(struct Node* curr) {
    return curr == NULL;
}

// Function to the Middle Node and break the List in Half
struct Node* getmid(struct Node* list) {
    struct Node* twice = list->next;
    while (twice && twice->next) {
        twice = twice->next->next;
        list = list->next;
    }
    struct Node* mid = list->next;
    list->next = NULL;
    return mid;
}

// Function to Merge two Sorted Lists
struct Node* merge(struct Node* list1, struct Node* list2)
{
    struct Node* res = NULL, *curr = NULL;
    while (!empty(list1) || !empty(list2)) {
        if (empty(list2) && !empty(list1))
            curr = insert(&curr, poptop(&list1));
        else if (empty(list1) && !empty(list2))
            curr = insert(&curr, poptop(&list2));
        else curr = insert(&curr, (list1->val > list2->val) ?
            poptop(&list1) : poptop(&list2));
        if(res == NULL) res = curr;
    }
    return res;
}

// Function to Generate a Linked List from an Array
struct Node* genlist(int arr[], int size) {
    struct Node* list = NULL;
    for (int i = 0; i < size; i++)
        append(&list, arr[i]);
    return list;
}

// Function to count the number of elements in a file
int countElements(const char* filename) {
    int count = 0, status = 1, input;
    FILE* file = fopen(filename, "r");
    if(file == NULL) {
        perror("Error");       // Print error message if file opening fails
        exit(1);
    }
    while (status == 1) {
       status = fscanf(file, "%d ", &input);
       count++;
    }
    return count - 1;
}

// Function to generate an int buffer for storing elements
int* generateBuffer(const int size) {
    return (int*) malloc(sizeof(int)*size);
}

// Function to calculate the duration between two timeval structs
double calduration(struct timeval et, struct timeval st, bool micro) {
    if (micro) return ((et.tv_sec - st.tv_sec) * 1000000u + (et.tv_usec - st.tv_usec)) / 1000.0;
    else return (et.tv_sec - st.tv_sec) * 1000u + (et.tv_usec - st.tv_usec) / 1000.0;
}
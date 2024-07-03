// Name: Ahmad Farhan
// Roll no. 21I-1366
// Assignment 2
// Question 2
//==================================================
// Collective Communication Test Code
//--------------------------------------------------
#include "mpicc.h"
#define elements 5      // No. of Elements

void test_allgather();      // Test Code for mpi_allgather
void test_allgatherv();     // Test Code for mpi_allgatherv
void test_alltoall();       // Test Code for mpi_alltoall
void test_alltoallv();      // Test Code for mpi_alltoallv

int main(){
    printf("\n");
    MPI_Init(NULL, NULL);
    // Function Being Tested
    test_allgatherv();
    MPI_Finalize();
}

// Initialize int buffers with test sample data
void init_buffers(int* array, const int rank, const int size){
    for(int i = 0; i < size; i++)
        array[i] = rank * size + i;
}
// Initialize char buffers with test sample data
void init_buffersc(char* array, const int rank, const int size) {
    for(int i = 0; i < size; i++)
        array[i] = rank * size + i + 'A';
}

// Function to Test mpi_allgather
void test_allgather(){
    int rank, com_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // Get Rank of Current Process
    MPI_Comm_size(MPI_COMM_WORLD, &com_size);   // Get No. of Processes

    char rcvbuf[elements * com_size], sndbuf[elements]; // Allocate send/ recv buffers
    init_buffersc(sndbuf, rank, elements);          // Initialize Buffers with send data

    // Display Data Being Sent By each Process
    printf("%d Sending: ", rank);
    displayc(sndbuf, elements);
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Execute MPI Collective Communication Function
    MPI_Allgather(sndbuf, elements, MPI_CHAR, rcvbuf, elements, MPI_CHAR, MPI_COMM_WORLD);
    if(rank == root) printf("MPI Function Results\n----------------------\n");
    MPI_Barrier(MPI_COMM_WORLD);
    // Display Data Recieved By each Process
    printf("%d Recieved: ", rank);
    displayc(rcvbuf, elements*com_size);

	for(int i=0;i<elements*com_size;rcvbuf[i++] = 0);
    MPI_Barrier(MPI_COMM_WORLD);

    // Execute pseudo MPI Collective Communication Function
    mpi_allgather(sndbuf, elements, MPI_CHAR, rcvbuf, elements, MPI_CHAR, MPI_COMM_WORLD);
    if(rank == root) printf("Pseudo MPI Results\n----------------------\n");
    MPI_Barrier(MPI_COMM_WORLD);
    // Display Data Recieved By each Process
    printf("%d Recieved: ", rank);
    displayc(rcvbuf, elements*com_size);
}

void test_allgatherv(){
    int rank, com_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // Get Rank of Current Process
    MPI_Comm_size(MPI_COMM_WORLD, &com_size);   // Get No. of Processes

    // Initialize Send and Recieve Data 
    int buffer[elements*com_size], sndbuf[elements];
    int counts[elements], disps[elements];

    init_buffers(sndbuf, rank, elements);
    for(int i = 0; i < elements*com_size; buffer[i++] = 0);
    for(int i = 0; i < elements; i++) counts[i] = i%3+2, disps[i] = i*elements;
   
    // Display Data Sent By each Process
    printf("%d Sending (%d, %d): ", rank, counts[rank], disps[rank]);
    display(sndbuf, counts[rank]);
    MPI_Barrier(MPI_COMM_WORLD);

    // Execute MPI Collective Communication Function
    MPI_Allgatherv(sndbuf, counts[rank], MPI_INT, buffer, counts, disps, MPI_INT, MPI_COMM_WORLD);
    if(rank == root) printf("MPI Function Results\n----------------------\n");
    MPI_Barrier(MPI_COMM_WORLD);
    // Display Data Recieved By each Process
    printf("%d Recieved: ", rank);
    display(buffer, elements*com_size);
    
	for(int i=0;i<elements*com_size;buffer[i++] = 0);
    MPI_Barrier(MPI_COMM_WORLD);

    // Execute pseudo MPI Collective Communication Function
    mpi_allgatherv(sndbuf, counts[rank], MPI_INT, buffer, counts, disps, MPI_INT, MPI_COMM_WORLD);
    if(rank == root) printf("Pseudo MPI Function Results\n----------------------\n");
    MPI_Barrier(MPI_COMM_WORLD);
    // Display Data Recieved By each Process
    printf("%d Recieved: ", rank);
    display(buffer, elements*com_size);
}

void test_alltoall(){
    int rank, com_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // Get Rank of Current Process
    MPI_Comm_size(MPI_COMM_WORLD, &com_size);   // Get No. of Processes

    // Initialize Send and Recieve Buffers
    int sndcount = elements/com_size;
    int sndbuf[elements], rcvbuf[elements];
    init_buffers(sndbuf, rank, elements);
	for(int i=0;i<elements;rcvbuf[i++] = 0);

    // Display Data Being Sent By each Process
    printf("Rank %d Sending %d: ", rank, sndcount);
    display(sndbuf, elements);
    MPI_Barrier(MPI_COMM_WORLD);


    // Execute Collective Communication Function
    MPI_Alltoall(&sndbuf, 1, MPI_INT, rcvbuf, 1, MPI_INT, MPI_COMM_WORLD);
    if(rank == root) printf("MPI Function Results\n----------------------\n");
    MPI_Barrier(MPI_COMM_WORLD);
    // Display Data Recievd By each Process
    printf("Rank %d recieved: ", rank);
    display(rcvbuf, elements);

	for(int i=0;i<elements;rcvbuf[i++] = 0);
    MPI_Barrier(MPI_COMM_WORLD);


    // Execute Pseudo MPI Collective Communication Function    
    mpi_alltoall(&sndbuf, 1, MPI_INT, rcvbuf, 1, MPI_INT, MPI_COMM_WORLD);
    if(rank == root) printf("Pseudo MPI Function Results\n----------------------\n");
    MPI_Barrier(MPI_COMM_WORLD);
    // Display Data Recievd By each Process
    printf("Rank %d recieved: ", rank);
    display(rcvbuf, elements);
}

void testcase_alltoallv(int rank, int sc[], int rc[], int sd[], int rd[]){
    switch(rank) {
        case 0:
            sc[0] = 1, sc[1] = 2, sc[2] = 0;
            rc[0] = 1, rc[1] = 0, rc[2] = 1;
            sd[0] = 0, sd[1] = 1, sd[2] = 0;
            rd[0] = 1, rd[1] = 0, rd[2] = 0;
            break;
        case 1:
            sc[0] = 0, sc[1] = 0, sc[2] = 3;
            rc[0] = 2, rc[1] = 0, rc[2] = 0;
            sd[0] = 0, sd[1] = 0, sd[2] = 0;
            rd[0] = 0, rd[1] = 0, rd[2] = 0;
            break;
        case 2:
            sc[0] = 1, sc[1] = 0, sc[2] = 0;
            rc[0] = 0, rc[1] = 3, rc[2] = 0;
            sd[0] = 0, sd[1] = 0, sd[2] = 0;
            rd[0] = 0, rd[1] = 0, rd[2] = 0;
            break;
    }
}

#define total 5
void test_alltoallv(){
    int rank, com_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // Get Rank of Current Process
    MPI_Comm_size(MPI_COMM_WORLD, &com_size);   // Get No. of Processes
	if(com_size != 3){if(rank == root)printf("Error: Test code for np 3\n");return;}

    // Initialize send and recv buffers
    char sndbuf[total], rcvbuf[total * com_size];
    for(int i=0; i<total*com_size; rcvbuf[i++]=0);
    init_buffersc(sndbuf, rank, total);


    // Initialize Send and Recv Displacements and Counts
    int scounts[total] = {1,1,1}, sdisps[total] = {0,2,5};
    int rcounts[total] = {1,1,1}, rdisps[total] = {0,3,5};
    testcase_alltoallv(rank, scounts, rcounts, sdisps, rdisps);

    // Display Data Being Sent By each Process
    printf("Rank %d Sending: ", rank);
    displayc(sndbuf, total);

    // Execute MPI Collective Communication Function
    MPI_Alltoallv(&sndbuf, scounts, sdisps, MPI_CHAR, rcvbuf, rcounts, rdisps, MPI_CHAR, MPI_COMM_WORLD);
    if(rank == root) printf("MPI Function Results\n----------------------\n");
    MPI_Barrier(MPI_COMM_WORLD);
    // Display Data Recievd By each Process
    printf("Rank %d Recieved: ", rank);
    displayc(rcvbuf, total*com_size);

	for(int i=0;i<total*com_size;rcvbuf[i++] = 0);
    MPI_Barrier(MPI_COMM_WORLD);


    // Execute pseudo MPI Collective Communication Function
    mpi_alltoallv(&sndbuf, scounts, sdisps, MPI_CHAR, rcvbuf, rcounts, rdisps, MPI_CHAR, MPI_COMM_WORLD);
    if(rank == root) printf("Pseudo MPI Function Results\n----------------------\n");
    MPI_Barrier(MPI_COMM_WORLD);
    // Display Data Recievd By each Process
    printf("Rank %d Recieved: ", rank);
    displayc(rcvbuf, total*com_size);
}

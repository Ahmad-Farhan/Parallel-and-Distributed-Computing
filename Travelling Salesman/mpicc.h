// Name: Ahmad Farhan
// Roll no. 21I-1366
// Assignment 2
// Question 2
//==================================================
// Collective Communication Implementations
//--------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <mpi/mpi.h>
#define root 0


// Collective Communication API
// ===============================================================


// Gathers fixed amounts of data from all processes
// and sends combined data to all processes.
void mpi_allgather( 
    void* sndbuf, int sndcount, MPI_Datatype sndtype,
    void* rcvbuf, int rcvcount, MPI_Datatype rcvtype,
    MPI_Comm comm
);

// Gathers variable amounts of data from all processes
// and sends combined data to all processes.
void mpi_allgatherv(
    void* sndbuf, int sndcount, MPI_Datatype sndtype,
    void* rcvbuf, const int* rcvcounts, const int *displs,
    MPI_Datatype rcvtype, MPI_Comm comm
);

// All processes scatter fixed amount of data to all other processes
void mpi_alltoall(
    void* sndbuf, int sndcount, MPI_Datatype sndtype, 
    void* rcvbuf, int rcvcount, MPI_Datatype rcvtype,
    MPI_Comm comm
);

// All processes scatter variable amounts of data to all other processes
void mpi_alltoallv(
    void* sndbuf, const int* sndcounts, const int* sdispls, MPI_Datatype sndtype, 
    void* rcvbuf, const int* rcvcounts, const int* rdispls, MPI_Datatype rcvtype, 
    MPI_Comm comm
);

// General Function Implementations
// ===============================================================

// Function to display an integer array
void display(int* array, const int size){
    for(int i = 0; i < size; i++)
        printf("%d ", array[i]);
    printf("\n");
}

// Function to display a character array
void displayc(char* array, const int size){
    for(int i = 0; i < size; i++)
        printf("%c ", array[i]);
    printf("\n");
}

// Function to concatenate two arrays 
void concat(void* array, void* other, const int start, const int count, const int typesize){
    char* arrayc = array + start * typesize, *otherc = other;
    for(int i = 0; i < count * typesize; i++) arrayc[i] = otherc[i];
}

// Function to calculate Max Buffer size using Counts and Displacements
int buffsize(const int* counts, const int* disps, const int size){
    int maxsize = 0;
    for(int i = 0; i < size; i++)
        if(counts[i] + disps[i] > maxsize)
            maxsize = counts[i] + disps[i];
    return maxsize; 
}

// MPI Collective Communication Function Implementations
// ===============================================================

void mpi_allgather( 
    void* sndbuf, int sndcount, MPI_Datatype sndtype,
    void* rcvbuf, int rcvcount, MPI_Datatype rcvtype,
    MPI_Comm comm) {

    int rank, com_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // Get Rank of Current Process
    MPI_Comm_size(MPI_COMM_WORLD, &com_size);   // Get no. of Processes
    
    // Send sndcount amount of Data from Each Process to Root
    MPI_Send(sndbuf, sndcount, sndtype, root, 0, MPI_COMM_WORLD);

    if(rank == root) {
        int typesize;
        MPI_Type_size(rcvtype, &typesize);              // Get no. of bytes of rcvtype
        void* tmpbuf = malloc(typesize * rcvcount);     // Allocate temp recieve buffer

        for(int i = 0; i < com_size; i++) {             // Root Recieves Data from each Process
            MPI_Recv(tmpbuf, rcvcount, rcvtype, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            concat(rcvbuf, tmpbuf, i * sndcount, sndcount, typesize);   // Root Combines Recieved Data
        }
        // Send Combined Data to all Other Processes
        for(int i = 1; i < com_size; i++)
            MPI_Send(rcvbuf, rcvcount * com_size, rcvtype, i, 1, MPI_COMM_WORLD);
        free(tmpbuf);       // Free up temporary recieving buffer
    } 
    // Each Process except Root Recieves the Combined Data from Root
    else MPI_Recv(rcvbuf, rcvcount * com_size, rcvtype, root, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void mpi_allgatherv(
    void* sndbuf, int sndcount, MPI_Datatype sndtype,
    void* rcvbuf, const int* rcvcounts, const int *displs,
    MPI_Datatype rcvtype, MPI_Comm comm) {

    int rank, com_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // Get Rank of Current Process
    MPI_Comm_size(MPI_COMM_WORLD, &com_size);   // Get no. of Processes
    
    // Send sndcount amount of Data from Each Process to Root
    MPI_Send(sndbuf, sndcount, sndtype, root, 0, MPI_COMM_WORLD);
    // Calculate max buffer size needed to recieve data
    int rcvsize = buffsize(rcvcounts, displs, com_size); 

    if(rank == root) {
        int typesize;
        MPI_Type_size(rcvtype, &typesize);             // Get no. of bytes of rcvtype
        void* tmpbuf = malloc(typesize * rcvsize);     // Allocate temp recieve buffer

        for(int i = 0; i < com_size; i++) {             // Root Recieves Data from each Process
            MPI_Recv(tmpbuf, rcvcounts[i], rcvtype, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Combine recieved data based on displacement and size
            concat(rcvbuf, tmpbuf, displs[i], rcvcounts[i], typesize);
        }
        // Send Combined Data to all Other Processes
        for(int i = 1; i < com_size; i++)
            MPI_Send(rcvbuf, rcvsize, rcvtype, i, 1, MPI_COMM_WORLD);
        free(tmpbuf);       // Free Recieving Buffer
    }   // Each Process Except Root Recieves Combined Data
    else MPI_Recv(rcvbuf, rcvsize, rcvtype, root, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void mpi_alltoall(
    void* sndbuf, int sndcount, MPI_Datatype sndtype, 
    void* rcvbuf, int rcvcount, MPI_Datatype rcvtype,
    MPI_Comm comm) {

    int rank, com_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // Get Rank of Current Process
    MPI_Comm_size(MPI_COMM_WORLD, &com_size);   // Get no. of Processes

    int stypesize, rtypesize;
    MPI_Type_size(sndtype, &stypesize);             // Get no. of bytes of sndtype
    MPI_Type_size(rcvtype, &rtypesize);             // Get no. of bytes of rcvtype

    void* tmpbuf = malloc(rtypesize * rcvcount);    // Allocate temp recieve buffer
    int diff = sndcount * stypesize;        // Differential for Start index of Recieved Data

    for(int i = 0; i < com_size; i++)       // Each Process Scatters Data to all Other Processes
        MPI_Send(sndbuf + i * diff, sndcount, sndtype, i, 0, MPI_COMM_WORLD);

    for(int i = 0 ; i < com_size; i++){     // Each Process Recieves from all other Processes
        MPI_Recv(tmpbuf, rcvcount, rcvtype, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        concat(rcvbuf, tmpbuf, i * rcvcount, rcvcount, rtypesize);  // Each Processes Combines Data
    }
    free(tmpbuf);       // Free Recieving Buffer
}

void mpi_alltoallv(
    void* sndbuf, const int* sndcounts, const int* sdispls, MPI_Datatype sndtype, 
    void* rcvbuf, const int* rcvcounts, const int* rdispls, MPI_Datatype rcvtype, 
    MPI_Comm comm) {

    int rank, com_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // Get Rank of Current Process
    MPI_Comm_size(MPI_COMM_WORLD, &com_size);   // Get no. of Processes
    
    int stypesize, rtypesize;
    MPI_Type_size(sndtype, &stypesize);             // Get no. of bytes of sndtype
    MPI_Type_size(rcvtype, &rtypesize);             // Get no. of bytes of rcvtype
    
    int rcvsize = buffsize(rcvcounts, rdispls, com_size);   //(rcvcounts, com_size);         // Calculate max buffer needed to recieve data
    void *tmpbuf = malloc(rtypesize * rcvsize);     // Allocate Recieving Buffer

    // Each Proccess Sends data to all other Processes based on sdispls and scounts 
    for(int i = 0; i < com_size; i++)
        MPI_Send(sndbuf + sdispls[i] * stypesize, sndcounts[i], sndtype, i, 0, MPI_COMM_WORLD);

    for(int i = 0; i < com_size; i++) {    // Each Proccess Recieves data from all other Processes 
        MPI_Recv(tmpbuf, rcvcounts[i], rcvtype, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // Combine recieved data based on rdispls and rcounts
        concat(rcvbuf, tmpbuf, rdispls[i], rcvcounts[i], rtypesize);
    }
    free(tmpbuf);       // Free Recieving Buffer
}

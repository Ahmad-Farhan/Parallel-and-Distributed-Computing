// Name: Ahmad Farhan
// Roll no. 21I-1366
// Assignment 1
// Question 1
//==================================================
// Parallel MPI Implementation
//--------------------------------------------------

#include "clist.h"
#include "graph.h"
#include <mpi/mpi.h>

#define root 0			// Rank of Root Process
#define inf 1e5			// Infinite Cost

// Function to Add Node to Graph
void addNode(graph* g){
	pushn(g);
}

// Function to get Node from Graph
node* getNode(graph* g, const int key){
	if(key < g->num_nodes)
		return &g->nodes[key];
	return NULL;
}

// Function to add Edge to Graph
void addEdge(graph* g, int src, int dst, int cost){
	if(src >= g->num_nodes || dst >= g->num_nodes)
	{printf("Error: Nodes not in graph\n");return;}

	node* srcn, *dstn;
	srcn = getNode(g, src);
	if(!g->directed) dstn = getNode(g, dst);

	pushe(srcn, new_edge(dst, cost));
	if(!g->directed) pushe(dstn, new_edge(src, cost));
}

// Function to get Cost of Edge
int edgeCost(graph* g, int src, int dst) {
	node* srcn = getNode(g, src);

	for (int i = 0; i < srcn->num_edges; i++){
		edge* edg = getEdge(srcn, i);
		if(edg->dst == dst) return edg->cost;
	}
	return inf;
}

// Function to Recursively explore all possible paths
void recurse(graph* g, int* min_path, bool* visit, list* curr_path, int curr,
			 int cost, int depth, int* min_cost) {

	if (!depth) {		// If all Nodes Explored
		cost += edgeCost(g, curr, curr_path->arr[0]);		//Get cost to start node
		if (cost < *min_cost) {						 		//Update min_cost and min_path
			copyi(min_path, curr_path->arr, curr_path->size + 1);
			*min_cost = cost;
		}
		return;
	}
	node* srcn = getNode(g, curr);
	for (int i = 0; i < srcn->num_edges; i++) {		// Check all Outgoing Edges
		edge edg = *getEdge(srcn, i);

		if (!visit[edg.dst]) {				// If Destination Node of Edge is Unvisited
			visit[edg.dst] = true;			// Mark Destination node as visited
			push(curr_path, edg.dst);		// Add Destination Node to Path

			// Recursively Explore all possible paths of unvisited nodes from current node
			recurse(g, min_path, visit, curr_path, edg.dst, cost + edg.cost, depth - 1, min_cost);
			visit[edg.dst] = false;			// Mark Destination Node as Unvisited
			pop(curr_path);					// Remove Destination Node from Path
		}
	}
}


// Function to Solve Travelling Salesman Problem
int tsp(graph* g, int* min_path, int rank, int com_size) {

	bool *visit = calloc(g->num_nodes, sizeof(bool));		// Boolean Array to Mark Nodes Visited
	list curr_path = new_list(g->num_nodes);			// list maintains track of pathe being explored
	int min_cost = inf;

	for(int n = rank; n < g->num_nodes; n += com_size){
		push(&curr_path, n);							// Add initial node to path
		visit[n] = true;								// Mark initial node as visited
		curr_path.arr[g->num_nodes] = n;						// Set target node as initial

		// Recursively Explore all possible paths given a starting node
		recurse(g, min_path, visit, &curr_path, n, 0, g->num_nodes-1, &min_cost);
		visit[n] = false;								// Mark initial as unvisited
		pop(&curr_path);								// Remove initial from explore path
	}

	del_list(&curr_path);			// Free Path Memory
	free(visit);					// Free Visited Tracking Array

	return min_cost;
}

// Function to get index of Minimum Element in Array
int minidx(int* arr, const int size){
	int min = inf, idx = -1;
	for(int i=0;i<size;i++)
		if(arr[i] < min){
			min = arr[i];
			idx = i;
		}
	return idx;
}

// Function to Manage Communications across Processes solving TSP
void tsp_control(graph* g){
	int rank, com_size;
	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);			// Get Rank of Current Process
	MPI_Comm_size(MPI_COMM_WORLD, &com_size);		// Get No. of Processes in MPI_Comm

	if(rank == root) {		// Root Process Displays Graph and Error Message If Any
		printf("Displaying Graph...\n");
		display(g);
		if(com_size > g->num_nodes) 
			printf("Error: Processes more than Nodes\n");
	}
	MPI_Barrier(MPI_COMM_WORLD);		// Syncronize for Error Checking
	if(com_size > g->num_nodes) return;		// Excessive no. of processes

	int* rcvbuf = NULL;
	int* min_path = malloc(g->num_nodes * sizeof(int) + 1);	// Allocate space for minimum path
	if(rank == 0) rcvbuf = malloc(sizeof(int) * com_size);	// Buffer for Recieving shortest path

	int min_cost = tsp(g, min_path, rank, com_size);		//Each Process Executes TSP

	// Root Process Gathers minimum cost found by each process
	MPI_Gather(&min_cost, 1, MPI_INT, rcvbuf, 1, MPI_INT, 0, MPI_COMM_WORLD);
	// Each Process sends mininum cost path found to Root
	MPI_Send(min_path, g->num_nodes+1, MPI_INT, 0, 0, MPI_COMM_WORLD);

	if(rank == root){	
		int idx = minidx(rcvbuf, com_size);		// Get index of minimum cost found
		min_cost = rcvbuf[idx];			// Recieve minimum cost path only from that process
		MPI_Recv(min_path, g->num_nodes+1, MPI_INT, idx, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		// Display Results
		printf("Overall Min cost: %d - ", min_cost);
		disp(min_path, g->num_nodes+1);
	}
	if(rank == 0) free(rcvbuf);		// Free Recieving Buffer Memory
	free(min_path);					// Free min_path Memory 

	MPI_Finalize();
}

// Function to add Edges to Graph
void init_graph(graph* g){
    char line[10];
	int src, dst, cost;
	FILE *file = fopen("12.txt", "r");
	if(file == NULL) {
		perror("File: ");
		exit(EXIT_FAILURE);
	}
    while (fgets(line, 10, file)) {
        sscanf(line, "%d,%d,%d", &src, &dst, &cost);
		addEdge(g, src, dst, cost);
    }
    fclose(file);
}


int main() {
	printf("\n");
	//Create Graph
	graph* g = new_graph(12, true);
	
	//Initialize Edges
	init_graph(g);
	//Solve TSP
	tsp_control(g);
	//Delete Graph
	del_graph(g);
	
	return -1;
}

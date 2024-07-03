// Name: Ahmad Farhan
// Roll no. 21I-1366
// Assignment 1
// Question 1
//==================================================
// Serial Implementation
//--------------------------------------------------

#include "clist.h"
#include "graph.h"

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
			copyi(min_path, curr_path->arr, curr_path->size+1);
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
int tsp(graph* g) {
	int* min_path = malloc(g->num_nodes * sizeof(int) + 1);	// Allocate space for minimum path
	bool *visit = calloc(g->num_nodes, sizeof(bool));		// Boolean Array to Mark Nodes Visited
	list curr_path = new_list(g->num_nodes);			// list maintains track of pathe being explored
	int min_cost = inf;

	for(int n = 0; n < g->num_nodes; n ++){
		push(&curr_path, n);							// Add initial node to path
		visit[n] = true;								// Mark initial node as visited
		curr_path.arr[g->num_nodes] = n;						// Set target node as initial

		// Recursively Explore all possible paths given a starting node
		recurse(g, min_path, visit, &curr_path, n, 0, g->num_nodes-1, &min_cost);
		visit[n] = false;								// Mark initial as unvisited
		pop(&curr_path);								// Remove initial from explore path
	}

	printf("Overall Min cost: %d - ", min_cost);
	disp(min_path, g->num_nodes+1);

	del_list(&curr_path);			// Free Path Memory
	free(visit);					// Free Visited Tracking Array

	return min_cost;
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
	display(g);
	//Solve TSP
	tsp(g);
	//Delete Graph
	del_graph(g);
	
	return -1;
}

// Name: Ahmad Farhan
// Roll no. 21I-1366
// Assignment 2
//==================================================
// Edge List Graph Implementation
//--------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


// Structure for Edge 
struct Edge {
	int dst;	// Destination Node
	int cost;	// Edge cost
};
// Alias for struct Edge as edge
typedef struct Edge edge;

// Function to create new Edge
edge new_edge(const int dst, const int cost){
	edge newEdge;
	newEdge.dst = dst;
	newEdge.cost = cost;
	return newEdge;
}

// Structure for Node
struct Node {
	edge* edges;	// Outgoing Edges
	int num_edges;	// No of Edges
	int cap;		// Max Capacity
};
// Alias of struct Node as node
typedef struct Node node;

// Function to create a new Node
node new_node(const int num_edges){
	node newNode;
	newNode.edges = (edge*)malloc(sizeof(edge)*num_edges);
	newNode.cap = num_edges;
	newNode.num_edges = 0;
	return newNode;
}

// Function to delete a node
void del_node(node* n){
	free(n->edges);
}

// Function to copy edges of one list into another
void copye(edge* arr1, edge* arr2, int num_edges) {
	for (int i = 0; i < num_edges; i++)
		arr1[i] = arr2[i];
}

// Function to Reallocate a large outgoing edge list
void relloce(node* array) {
	int new_cap = array->num_edges * 2;
	edge* new_arr = (edge*)malloc(sizeof(edge)*new_cap);
	copye(new_arr, array->edges, array->num_edges);

	free(array->edges);
	array->edges = new_arr;
	array->cap = new_cap;
}

// Function to append edge to node with zero cost
void pushei(node* array, int val) {
	if (array->num_edges + 1 > array->cap)
		relloce(array);
	array->edges[array->num_edges] = new_edge(val, 0);
	array->num_edges += 1;
}

// Function to Append Edge to node
void pushe(node* array, edge val) {
	if (array->num_edges + 1 > array->cap)
		relloce(array);
	array->edges[array->num_edges] = val;
	array->num_edges += 1;
}

// Function to Remove Edge from node
void pope(node array) {
	array.num_edges -= 1;
}

// Function to get Outgoing Edges of a node
edge* getEdge(node* n, int index){
	return &n->edges[index];
}


// Structure for Graph
struct Graph{
	node* nodes;	// Array of Nodes
	bool directed;	// Nature of Edges
	int num_nodes;	// No of Nodes
	int cap;		// Max Capacity of Nodes
};
// Alias of struct Graph as graph
typedef struct Graph graph;

// Function to create a new dynamic Graph
graph* new_graph(const int num_nodes, bool direct){
    graph* newGraph = (graph*)malloc(sizeof(graph));
	if (newGraph != NULL){
		newGraph->nodes = (node*)malloc(sizeof(node)*num_nodes);
		newGraph->num_nodes = num_nodes;
		newGraph->directed = direct;
		newGraph->cap = num_nodes;
	}
	return newGraph;
}

// Function to Deallocate Graph Memory
void del_graph(graph* g){
	for(int i=0;i<g->num_nodes;i++)
		del_node(&g->nodes[i]);
	free(g->nodes);
	free(g);
}

// Function to copy nodes from one array to another
void copyn(node* arr1, node* arr2, int num_nodes) {
	for (int i = 0; i < num_nodes; i++)
		arr1[i] = arr2[i];
}

// Function to Reallocate Larger memory of nodes
void rellocn(graph* array) {
	int new_cap = array->num_nodes * 2 + 1;
	node* new_arr = (node*)malloc(sizeof(node)*new_cap);
	copyn(new_arr, array->nodes, array->num_nodes);

	free(array->nodes);
	array->nodes = new_arr;
	array->cap = new_cap;
}

// Function to Append nodes to graph
void pushn(graph* array) {
	if (array->num_nodes + 1 > array->cap)
		rellocn(array);
	array->nodes[array->num_nodes] = new_node(5);
	array->num_nodes += 1;
}

// Function to Remove nodes from graph
void popn(graph array) {
	array.num_nodes -= 1;
}

// Function to display edges of a node
void displayn(node array) {
	for (int i = 0; i < array.num_edges; i++)
		printf("%d(%d) ", array.edges[i].dst, array.edges[i].cost);
	printf("\n");
}

// Functino to display Graph
void display(graph* g) {
	if(g == NULL){ printf("Error: Graph Empty");return;}

	for(int i=0;i < g->num_nodes;i++){
		printf("%d: ", i);
		displayn(g->nodes[i]);
	}
	printf("\n");
}

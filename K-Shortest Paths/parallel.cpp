#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <chrono>
#include <queue>

#include <mpi/mpi.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define defaultfile "new-who-grf.txt"
#define inf 1e6

using namespace std;
typedef chrono::_V2::system_clock::time_point time_val;
typedef vector<vector<pair<int, int>>> _graph;				// Graph Structure
typedef pair<int, vector<int>> pi;							// Node cost - path


//Find index of maximum value in distance vector
int argmax(vector<pi> dis) {
	int max_val = dis[0].first, idx = 0;
	for (int i = 0; i < dis.size(); i++)
		if (dis[i].first > max_val)
			max_val = dis[i].first, idx = i;
	return idx;
}
// Read file meta data to get number of nodes 
int readNumNodes(ifstream& file){
	string line;
	int num_nodes = -1;
	while (getline(file, line) && line[0] =='#') {
		if (line.find("Nodes:") != string::npos) {	// Line Containing Value
			stringstream ss(line);
			string token;
			while (ss >> token)
				if (token == "Nodes:") {			// Position of value
					ss >> num_nodes;
					break;
				}
		}
	}
	return num_nodes;
}
// Read graph data from -grf files 
_graph readGraph(string filename) {
	string line;
	int fromNode, toNode, weight;
	ifstream file(filename, ios::in);
	if(!file.is_open()) 
		throw runtime_error("File not found");

	_graph graph(readNumNodes(file));					// Initialize graph structure
	getline(file, line);
	while (file >> fromNode >> toNode) {
		if (file.peek() != '\n') file >> weight;
		else weight = 1;
		graph[fromNode].push_back({ toNode, weight });	// Append to node adjecency list  
	}
	file.close();
	return graph;
}
// Display graph as adjecency list
void display(_graph g) {
	for (int i = 0; i < g.size(); i++) {
		cout << i << ": ";
		for (int j = 0; j < g[i].size(); j++)
			cout << g[i][j].first << '(' << g[i][j].second << ") ";
		cout << "\n";
	}
}
// String encodings structure
vector<string> enc;
// Format node output as integer code or string name
string format(const int node){
	if (enc.size() > 0) return enc[node];
	return to_string(node);
}
// Displays resulting k-shortest paths as vectors
void display(vector<pi> results, const int k) {
	cout << endl;
	for (int i = 0; i < k; i++) {
		if (results[i].first == int(inf)) {
			printf("No further Paths Found\n");
			break;
		}
		cout << "Distance: " << results[i].first << ", Path: ";
		for (int j = 0; j < results[i].second.size(); j++){
			cout << format(results[i].second[j]);
			if (j!=results[i].second.size() - 1)
				cout << " - ";
		}
		cout << endl;
	}
}
bool validateFilename(string filename){
	size_t index = filename.find("-grf");
    if (index == string::npos) {
		printf("Error: Invalid File Type. Use -grf file");
		return false;
	}
	ifstream file(filename, ios::in);
	if(!file.is_open()) {
		printf("Error: File not found\n");
		return false;
	}
	file.close();
	return true;
}
// Generates names of meta data files(encoding or node pairs)
string metafile(string file, string meta) {
	if (meta == "enc")
		return file.substr(0, file.length() - 8) + "-enc.txt";
	else if (meta == "sdn")
		return file.substr(0, file.length() - 8) + "-sdn.txt";
	printf("Error: Invalid Meta Argument\n");
	return "";
}
// Loads source-destination pairs from file
vector<pair<int,int>> loadsdns(string originalfile){
	string filename = metafile(originalfile, "sdn");
	ifstream file(filename, ios::in);
	if(!file.is_open()) 
		throw runtime_error("SDN file not found");

	// Create a vector of node pairs
	vector<pair<int,int>> sdns;
	int first, second;
	while(file >> first >> second)
		sdns.push_back(pair<int,int>{first, second});
	file.close();
	return sdns;
}
// Loads node encodings from file
vector<string> loadencs(string original){
	string line, strkey;
	ifstream file(metafile(original, "enc"), ios::in);
	if(!file.is_open()) 
		throw runtime_error("ENC file not found");

	// Create vector indexable by int coded node
	vector<string> nodeCodes;
	while (getline(file, line)) {
		stringstream ss(line);
		getline(ss, strkey, ',');
		nodeCodes.push_back(strkey);
	}
	file.close();
	return nodeCodes;
}
// Thread safe display of k-shortest paths
void displayt(vector<pi> results, const int k, const int rank, int src, int dst, int d){
	string output = "\nThread: " + to_string(rank) + "\nK-Shortest Paths: " 
                   + to_string(src) + " -> " + to_string(dst);
    output += "\nParallel Execution Time : " + to_string(d) + " ms\n";
	for (int i = 0; i < k; i++) {
		if (results[i].first == int(inf)) {
			output += "No further Paths Found\n";
			break;
		}
		output += "Distance: " + to_string(results[i].first) + ", Path: ";
		for (int j = 0; j < results[i].second.size(); j++){
			output += format(results[i].second[j]);
			if (j!=results[i].second.size()-1)
				output += " - "; 
		}
		output += "\n";
	} // Create a string of output and print it with one call
	cout << output;
}
// Distributes graph data across MPI processes
void shareGraph(_graph& g, string filename){
    int rank, nodes;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// Broadcast number nodes to all processes
    if (!rank) nodes = g.size();
    MPI_Bcast(&nodes, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// Initialize graph structures in recieving processes
    if(rank) g = _graph(nodes);
	// Broadcast edges for each node
    for(int i = 0; i < nodes; i++){
        int size = g[i].size();
        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if(rank) g[i].resize(size);

		// Broadcast node and cost of each edge
        for(int j = 0; j < size; j++){
            MPI_Bcast(&g[i][j].first, 1, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(&g[i][j].second, 1, MPI_INT, 0, MPI_COMM_WORLD);
        }
    }
}
// Parallel version using Dijkstra'a Algorithm 
vector<pi> findKShortest(_graph g, int src, int dst, int k) {
	// Initialize distance - path vectors
	vector<vector<pi>> dis(g.size(), vector<pi>(k, { int(inf), {} }));	
	priority_queue<pi, vector<pi>, greater<pi>> pq;
	// Initialize source node data
	dis[src][0] = { 0, {src} };	
	pq.push({ 0, {src} });

	while (!pq.empty()) {
		// Extract min-cost node and distance
		vector<int> currpath = pq.top().second;
		int u = currpath.back();
		int d = pq.top().first;
		pq.pop();
		
		// Current path longer than kth shortest path
		if (dis[u][k - 1].first < d) continue;
		// Current node has no outgoing edges
        if(g[u].size() == 0) continue;

		// Process outgoing edges in parallel
		vector<pair<int, int>> v = g[u];
		// Create threads only when creation cost is worth it 
		#pragma omp parallel for num_threads(4) if v.size() > 3
			for (int i = 0; i < v.size(); i++) {
				int dest = v[i].first;
				int cost = v[i].second;
				int idx = argmax(dis[dest]);

				// Check against the worst path so far
				if (d + cost < dis[dest][idx].first) {
					vector<int> path = currpath;
					path.push_back(dest);
					dis[dest][idx] = { d + cost, path };
					#pragma omp critical 
						pq.push({ (d + cost), path });
				}
			}
		sort(dis[u].begin(), dis[u].end());
	}
	return dis[dst];
}
// Serial execution of n iterations of k-shortest path
int serialSolve(_graph g, string file, int iters, int k, bool encode){
    int src, dst, sum = 0;
	if(encode) enc = loadencs(file);
	vector<pair<int,int>> sdns = loadsdns(file);

	for(int i = 0; i < iters; i++){
		src = sdns[i].first, dst = sdns[i].second;
		cout << "\nK-Shortest Paths: " << src << " -> " << dst << endl;
		auto st = chrono::high_resolution_clock::now();					//Start time
		vector<pi> res = findKShortest(g, src, dst, k);
		auto et = chrono::high_resolution_clock::now();					//End time
		auto d = chrono::duration_cast<chrono::milliseconds>(et - st);
		printf("Parallel Execution Time : %ld ms", d.count());
		display(res, k);												// Display Results
		sum += d.count();			// Aggregate individual times for total execution time
	}
	return sum;
}
// Assigns work to MPI processes for k shortest path computation
int assignWork(_graph g, string file, int iters, int k, bool encode) {
	MPI_Request req[iters];						// For non-blocking send
	int src, dst, sum = 0, rank, com_size;
	if(encode) enc = loadencs(file);			// Load node encodings if required

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &com_size);
	MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);	// Broadcast k value
	if(!rank) {		// Root process distributes sdn data to workers
		vector<pair<int,int>> sdns = loadsdns(file);	// Read source-destination node pairs
		for(int i = 0; i < iters; i++){
			int data[2] = {sdns[i].first, sdns[i].second};
			MPI_Isend(data, 2, MPI_INT, i%com_size, 0, MPI_COMM_WORLD, &req[i]);
		}
	}
	int durations = 0; 	// For tracking total duration per process
	for(int i = rank; i < iters; i += com_size) {
		int data[2];				// Blocking recieve source-target node pair
		MPI_Recv(data, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		src = data[0]; dst = data[1];

		auto st = chrono::high_resolution_clock::now();			// Start timer
		vector<pi> res = findKShortest(g, src, dst, k);			// Parallel Algorithm
		auto et = chrono::high_resolution_clock::now();			// Stop timer
		auto d = chrono::duration_cast<chrono::milliseconds>(et - st);
		displayt(res, k, rank, src, dst, d.count());			// Display Results
		durations += d.count();
	}
	// Sum durations from each process to get total execution time
	MPI_Reduce(&durations, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	return sum;
}

int main(int argc, char* argv[]) {
    printf("\n");
	srand(time(NULL));
	MPI_Init(&argc, &argv);

	bool with_enc = false;			// Flag for original string output
    int com_size, sum_val, rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &com_size);

    _graph g;
	int iters = 10, k = 2;
	string filename = defaultfile;
	// Initialize execution parameters
	if(!rank && argc > 1) filename = argv[1];		// Set Graph file
	if(argc > 2) k = stoi(argv[2]);					// Set k-value
	if(argc > 3) with_enc = true;					// Set output type
	if(!validateFilename(filename)) return -1;

    if (rank == 0){		// Read graph file at root
		g = readGraph(filename);
        printf("K-Shortest Path Algorithm Parallel Version\n");
        printf("--------------------------------------------\n");
        printf("Graph Loaded from : %s\n", filename.c_str());
        printf("No. of Nodes: %ld\n", g.size());
        printf("Iterations : %d\nK-value: %d\n", iters, k);
    }
    auto st_time = chrono::high_resolution_clock::now();	// Start timer
	shareGraph(g, filename);		// Share graph data across MPI processes
	auto mt_time = chrono::high_resolution_clock::now();	// Stop timer

	// Perform work assignment based on number fo processes
	if(com_size > 1) sum_val = assignWork(g, filename, iters, k, with_enc);
	else sum_val = serialSolve(g, filename, iters, k, with_enc);

    if(rank == 0){	// Display overall metrics through root process
        auto et_time = chrono::high_resolution_clock::now();
        auto d = chrono::duration_cast<chrono::milliseconds>(et_time - st_time);
		auto t = chrono::duration_cast<chrono::milliseconds>(mt_time - st_time);
        printf("\n--------------------------------------------\n\n");
        printf("Total Parallel Execution Time : %ld ms\n", d.count());
        printf("Average Parallel Execution Time : %.2f ms\n", float(sum_val)/float(iters));
		printf("Total Graph Sharing Time : %ld ms\n", t.count());
    }

    MPI_Finalize();
    return -1;
}

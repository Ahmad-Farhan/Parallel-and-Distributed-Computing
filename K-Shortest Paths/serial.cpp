#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <chrono>
using namespace std;

#define testfile "new-who-grf.txt"
#define inf 1e6

typedef vector<vector<pair<int, int>>> _graph;			// Graph Structure
typedef pair<int, vector<int>> pi;						// Node cost - path

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
void validateFilename(string filename){
	size_t index = filename.find("-grf");
    if (index == string::npos)
		throw runtime_error("Invalid File Type. Use -grf file");
	ifstream file(filename, ios::in);
	if(!file.is_open()) 
	throw runtime_error("File not Found");
	file.close();
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
// Serial version using Dijkstra'a Algorithm 
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

		// Process outgoing edges in serial
		vector<pair<int, int>> v = g[u];
		for (int i = 0; i < v.size(); i++) {
			int dest = v[i].first;
			int cost = v[i].second;
			int idx = argmax(dis[dest]);

			// Check against the worst path so far
			if (d + cost < dis[dest][idx].first) {
				vector<int> path = currpath;
				path.push_back(dest);
				dis[dest][idx] = { d + cost, path };
					pq.push({ (d + cost), path });
			}
		}
		sort(dis[u].begin(), dis[u].end());
	}
	return dis[dst];
}
int main(int argc, char* argv[]) {
	srand(time(NULL));

	bool encoded = false;		// Flag for encoded output	
	float average = 0.0f;		// Average execution time
	const int iters = 10;
	int src, dst, k = 2;
	string filename = testfile;

	// Initialize execution parameters
	if(argc > 1) filename = argv[1];		// Set Graph file
	if(argc > 2) k = stoi(argv[2]);			// Set k-value
	if(argc > 3) enc = loadencs(filename);	// Set output type
	validateFilename(filename);

	// Initialize graph from file
	_graph g = readGraph(filename);
	printf("K-Shortest Path Algorithm Serial Version\n");
	printf("--------------------------------------------\n");
	printf("Graph Loaded from : %s\n", filename.c_str());
	printf("No. of Nodes: %ld\n", g.size());
	printf("Iterations : %d\nK-value: %d\n", iters, k);
	// Read source-destination node pairs
	vector<pair<int, int>> targetnodes = loadsdns(filename);
	auto st_time = chrono::high_resolution_clock::now();	//Start timer

	// Serially perform all 10 iterations
	for(int i=0;i<iters;i++){
		src = targetnodes[i].first, dst = targetnodes[i].second;
		cout << "\nK-Shortest Paths: " << src << " -> " << dst;
		auto st = chrono::high_resolution_clock::now();		//Start timer
		vector<pi> res = findKShortest(g, src, dst, k);		// Serial algorithm
		auto et = chrono::high_resolution_clock::now();		//Stop timer
		auto d = chrono::duration_cast<chrono::milliseconds>(et - st);	//Individual times
		printf("\nSerial Execution Time : %ld ms", d.count());
		average += d.count();								//Display results
		display(res, k);
	}
	// Display overall execution metrics
	auto et_time = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(et_time - st_time);
	printf("\n--------------------------------------------\n");
	printf("\nTotal Serial Execution Time : %ld ms", duration.count());
	printf("\nAverage Serial Execution Time : %.2f ms\n", average/iters);
	return 0;
}

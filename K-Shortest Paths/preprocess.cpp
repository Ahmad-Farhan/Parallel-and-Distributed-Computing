#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <algorithm>
#include <unordered_set>	
#include <unordered_map>
using namespace std;

// Appends suffix to a given file name
string editFilename(string initname, string suffix) {
	return initname.substr(0, initname.length() - 4) + suffix + ".txt";
}
// Generates names of meta data files(graph, encoding or node pairs)
string metafile(string filename, string meta) {
	if (meta == "enc")
		return editFilename(filename, "-enc");
	else if (meta == "sdn")
		return editFilename(filename, "-sdn");
	else if (meta == "grf" || meta == "csv" || meta == "txt")
		return editFilename(filename, "-grf");
	printf("Error: Invalid Meta Argument\n");
	return "";
}
// Copies comments within input data file to formatted file
string copyComments(ifstream& file, ofstream& outfile) {
	string line;
	while (getline(file, line) && line[0] == '#')
		outfile << line << "\n";
	return line.substr(0,line.length()-1) + "\t1\n";
}
// Read file meta data to get number of nodes 
int readNumNodes(string filename) {
	string line;
	int num_nodes = -1;
	ifstream file(filename, ios::in);
	while (getline(file, line) && line[0] == '#') {
		if (line.find("Nodes:") != string::npos) {
			stringstream ss(line);
			string token;
			while (ss >> token)
				if (token == "Nodes:") {
					ss >> num_nodes;
					break;
				}
		}
	} file.close();
	return num_nodes;
}
// Generates a single source-target node pair
void genRandomPair(int n, int& u, int& v) {
	if (n < 100000) do {		// For smaller graphs
		u = rand() % n;
		v = rand() % n;
	} while (u == v || abs(u - v) < (n / 4));
	else do {					// For larger graphs
		u = rand() % n;
		v = (rand() + n / 2) % n;
		int prob = rand() % 100;
		if (prob > 50) continue;
		int t = u; u = v, v = t;
	} while (u == v || abs(u - v) < (n / 2));
}
// Generates iter number of random node pairs in -sdn file
void randomNodePairs(string datafile, int n, int iters) {
	int u = 0, v = 0;
	string filename = metafile(datafile, "sdn");
	ofstream file(filename, ios::out);
	for (int i = 0; i < iters; i++) {
		genRandomPair(n, u, v);
		cout << u << " " << v << endl;
		file << u << "\t" << v << "\n";
	}
	file.close();
}
// Generates a string-int code mapping file
void saveToFile(string filename, unordered_set<string> nodes) {
	int index = 0;
	ofstream outfile(filename, ios::out);
	for (const auto& str : nodes)
		outfile << str << "," << index++ << "\n";
	outfile.close();
}
// Generates a list of unique string node names
unordered_set<string> encodeNodes(string filename) {
	vector<string> nodes;
	string line, fromNode, toNode;
	unordered_set<string> nodeStrings;
	ifstream infile(filename, ios::in);
	getline(infile, line);
	while (getline(infile, line)) {
		stringstream ss(line);
		getline(ss, fromNode, ',');
		getline(ss, toNode, ',');
		// If node names not already read
		if (nodeStrings.find(fromNode) == nodeStrings.end())
			nodeStrings.insert(fromNode);
		if (nodeStrings.find(toNode) == nodeStrings.end())
			nodeStrings.insert(toNode);
	}
	infile.close();
    return nodeStrings;
}
// Reads string to int mapping/encoding of nodes
unordered_map<string, int> readStrCode(string filename) {
	int code;
	string line, strkey;
	ifstream file(filename, ios::in);
	unordered_map<string, int> nodeCodes;
	while(getline(file, line)) {
		stringstream ss(line);
		getline(ss, strkey, ',');
		ss >> code;
		nodeCodes[strkey] = code;
	}
	file.close();
	return nodeCodes;
}
// Generates an encoded edge list of graph data
vector<array<int, 3>> encodeEdgeList(string filename, unordered_map<string, int> codes) {
	string line, data;
	vector<array<int, 3>> list;
	ifstream file(filename, ios::in);
	getline(file, line);
	int node1, node2, cost;
	while (getline(file, line)) {
		stringstream ss(line);
		// Encode source node
		getline(ss, data, ',');
		node1 = codes[data];
		// Encode destination node
		getline(ss, data, ',');
		node2 = codes[data];
		// Read edge cost
		ss >> cost;
		getline(ss, data);
		// Create encoded edge
		list.push_back({ node1, node2, cost });
	}
	file.close();
	return list;
}
// Generates a text file of encoded edge data
void makeGraphTxt(vector<array<int,3>> data, string original, int nodes) {
    string filename = metafile(original, "csv");
	ofstream file(filename, ios::out);
	file << "# Auto-generated file from " << original << '\n';
	file << "# Nodes: " << nodes << " Edges: " << data.size() << "\n";
	file << "# FromNodeId\tToNodeId\n";
	for (const auto& element : data)
		file << element[0] << "\t" << element[1] << "\t" << element[2] << "\n";
	file.close();
	cout << "Processed Graph File:\n" << filename << "\n";
}
// Controller for preprocessing csv files
void preprocesscsv(string filename) {
    cout << "Preprocessing csv : " << filename << endl;
    string codesfile = metafile(filename, "enc");

    printf("Encoding Node Names...\n");
    unordered_set<string> nodeStrings = encodeNodes(filename);
    
	printf("Generating Codes File %ld...\n", nodeStrings.size());
    saveToFile(codesfile, nodeStrings);
	unordered_map<string, int> codes = readStrCode(codesfile); 		// Reads Codes File
	vector<array<int,3>> rawData = encodeEdgeList(filename, codes); // Creates Encoded Edges
 	sort(rawData.begin(), rawData.end());			// Sort edges

	printf("Generating Graph File %ld...\n", rawData.size());
    makeGraphTxt(rawData, filename, codes.size());	// Generate formatted file
    printf("csv Processing Completed\n");
}
// Controller for preprocessing txt files
void preprocesstxt(string filename){
	cout << "Preprocessing txt: " << filename << endl;
    string nfilename = metafile(filename, "txt");

	// Assign weights to each edge
	printf("Formatting txt File...\n");
	int fromNode, toNode, weight = 1;
	ifstream infile(filename, ios::in);
	ofstream outfile(nfilename, ios::out);
	if(!infile.is_open()) 
	throw runtime_error("File not Found");

	string line;
    outfile << copyComments(infile, outfile);
	printf("Generating Graph File...\n");
	while (infile >> fromNode >> toNode)
		outfile << fromNode << "\t" << toNode << "\t" << weight << "\n";
	infile.close(); outfile.close();
	cout << "Processed Graph File:\n" << nfilename << "\n";
    printf("txt Processing Completed\n");
}
// Retrieve Input Extension(csv, txt)
string getExtension(string filename){
    size_t index = filename.find('.');
    if (index != string::npos){
        return filename.substr(index+1, filename.length()-index);
    }
    printf("Error: Incorrect File Format\n");
    return "";
}
// Checks if input file exists
void validateFilename(string filename){
	ifstream file(filename, ios::in);
	if(!file.is_open()) throw runtime_error("File not Found");
	else file.close();
}
int main(int argc, char* argv[]) {
	srand(time(NULL));
	string filename = "";
	if(argc > 1) filename = string(argv[1]);	// Filename as input

	validateFilename(filename);		// Validate if file exists
	string type = getExtension(filename);	// Get file extension
    if (type == "txt") preprocesstxt(filename);		// Preprocess txt file
    else if (type == "csv") preprocesscsv(filename);	// Preprocess csv file
	else return -1;		// Terminate if file type not supported

	// Check if sdn file is required
	if(argc < 3 || argv[2] != string("sdn")) return -1;
	printf("\nGenerating Source-Destination Pairs...\n");
    string datafile = metafile(filename, "grf");
	const int iters = 10, num_nodes = readNumNodes(datafile);
	randomNodePairs(filename, num_nodes, iters);

	return -1;
}

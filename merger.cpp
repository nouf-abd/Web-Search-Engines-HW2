#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>
#include <map>
#include <cctype>
#include <algorithm>
#include <optional>
using namespace std;

struct runRead
{
    string identTerm;
    vector<pair<int, int>> res;
    int runIndex;
};

struct compareRunRead
{   bool operator()(const runRead &a, const runRead &b)
    { // custom comparator for minHeap 
        return a.identTerm > b.identTerm;
    }
};

int getRunCount()
{ // need to pass runCount from indexer.cpp
    ifstream count_file("run_count.txt");
    int runCount;
    count_file >> runCount;
    count_file.close();
    return runCount;
}

pair<string, vector<pair<int, int>>> parseRunLine(const string &line)
{ // inverse of flushRun
    istringstream ss(line);
    string identTerm;
    ss >> identTerm; // what is the identifying term? ie. magic

    vector<pair<int, int>> res;
    int docId, freq;
    while (ss >> docId >> freq)
    {
        res.push_back({docId, freq}); // push pair into result vector
    }
    return {identTerm, res}; // res will be eg. {"magic", [(3,2)]}
}
vector<ifstream> opensRuns(int runCount)
{
    vector<ifstream> runFiles;
    for (int i = 0; i < runCount; i++)
    {                                                                  // open every run file in binary mode
        ifstream runFile("run_" + to_string(i) + ".bin", ios::binary); // opnes runfile with same logic from parser
        if (!runFile.is_open())
        {
            cerr << "Error opening file: run_" << to_string(i) << ".bin" << endl;
            exit(1);
        }
        runFiles.push_back(move(runFile)); // no heavy dupes
    }
    return runFiles;
}

optional<runRead> readNextLine(ifstream &runFile, int runIndex)
{ // reads unconsumed line from specific run file, parses via parseRunLine
    string line;
    if (getline(runFile, line))
    {
        auto [identTerm, res] = parseRunLine(line);
        return runRead{identTerm, res, runIndex};
    }
    return nullopt; // no more lines in run file to read
}

priority_queue<runRead, vector<runRead>, compareRunRead> initializeHeap(int runCount, vector<ifstream> &runFiles)
{ // calls readNextLine once per open run file to seed the heap with everyone's first front, takes place before main merge
    priority_queue<runRead, vector<runRead>, compareRunRead> minHeap;
    for (int i = 0; i < runCount; i++)
    {
        auto entry = readNextLine(runFiles[i], i);
        if (entry.has_value())
        {
            minHeap.push(entry.value());
        }
    }
    return minHeap;
}

vector<pair<int, int>> mergePostingsForTerm(vector<runRead> matched)
{ // merge every result in2 1 for specific term
    vector<pair<int, int>> merged;

    for (auto &entry : matched)
    {
        merged.insert(merged.end(), entry.res.begin(), entry.res.end());
    }
    sort(merged.begin(), merged.end());

    return merged; // sorted in docID order
}
vector<runRead> matchingTerms(priority_queue<runRead, vector<runRead>, compareRunRead> &minHeap)
{ // find all matching terms, return in single vector using our minHeap
    vector<runRead> matched;
    string matchTerm = minHeap.top().identTerm;

    while (!minHeap.empty() && minHeap.top().identTerm == matchTerm)
    {
        matched.push_back(minHeap.top());
        minHeap.pop();
    }

    return matched;
}
writeOutput()
{

} // writes the merged postings to the output file
int main()
{

    return 0;
}
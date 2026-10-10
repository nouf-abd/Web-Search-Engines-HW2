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
#include <queue>
#include <filesystem>
#include <cstring>
#include <cerrno>
using namespace std;

const int BLOCK_SIZE = 128;
struct runRead
{
    string identTerm;
    vector<pair<int, int>> res;
    int runIndex;
};

struct compareRunRead
{
    bool operator()(const runRead &a, const runRead &b)
    { // custom comparator for minHeap that makes it return the smallest identTerm at the top instead of the largest
        return a.identTerm > b.identTerm;
    }
};
using MinHeap = priority_queue<runRead, vector<runRead>, compareRunRead>; // minHeap for runRead, including the comparator so we can use priority queue as a minheap

int getRunCount()
{ // need to pass runCount from indexer.cpp for the for-loop later on
    ifstream count_file("runs/run_count.txt");
    int runCount;
    count_file >> runCount;
    count_file.close();
    return runCount;
}

pair<string, vector<pair<int, int>>> parseRunLine(const string &line) // parse line takes the identifying term and returns something like {"identTerm",[(docId,freq)]}
{                                                                     // inverse of flushRun in indexer.cpp
    istringstream ss(line);
    string identTerm;
    ss >> identTerm; // what is the identifying term? ie. magic - take the identifying term

    vector<pair<int, int>> res; // res holds docId and frequency pairings
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
        ifstream runFile("runs/run_" + to_string(i) + ".bin", ios::binary); // opnes runfile with same logic from parser
        if (!runFile.is_open())
        {
            cerr << "Error opening file: runs/run_" << i << ".bin" << endl;
            cerr << "Merger is running in: " << filesystem::current_path() << endl;
            cerr << "File exists from here? " << filesystem::exists("runs/run_" + to_string(i) + ".bin") << endl;
            cerr << "Reason: " << strerror(errno) << endl;
            exit(1);
        }
        runFiles.push_back(std::move(runFile)); // no heavy dupes with move
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

MinHeap initializeHeap(int runCount, vector<ifstream> &runFiles)
{ // calls readNextLine once per open run file to seed the heap with everyone's first front, takes place before main merge
    MinHeap minHeap;
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

vector<pair<int, int>> mergePostingsForTerm(const vector<runRead> &matched)
{ // merge every result in2 1 for specific term ie. "magic"
    vector<pair<int, int>> merged;
    for (auto &entry : matched)
    {
        merged.insert(merged.end(), entry.res.begin(), entry.res.end());
    }
    sort(merged.begin(), merged.end());

    return merged; // sorted in docID order
}
vector<runRead> matchingTerms(MinHeap &minHeap)
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

void varbyteEncode(uint32_t n, vector<uint8_t> &encoded)
{
    while (n >= 128)
    { // defined max varbyte size at 128
        encoded.push_back(static_cast<uint8_t>((n & 127) | 128));
        n >>= 7; // keeps one byte in for continuation flag, shift to the right for next 7
    }
    encoded.push_back(static_cast<uint8_t>(n));
}

uint32_t decodevar(const vector<uint8_t> &encoded, size_t &pos)
{
    uint32_t n = 0; // holding decoded val
    int shift = 0;
    while (true)
    {
        uint8_t curr = encoded[pos++];
        n |= static_cast<uint32_t>(curr & 127) << shift; //
        if (!(curr & 128))
        {
            break;
        } // zero flag
        shift += 7;
    }
    return n;
}
void writeu32(ofstream &encoded, uint32_t b)
{ // 32 bits = 4 bytes
    encoded.write(reinterpret_cast<const char *>(&b), sizeof(b));
}
void writeTermEntry(ofstream &indexFile, ofstream &lexFile, const string &term, const vector<pair<int, int>> &postings)
{
    uint64_t offset = indexFile.tellp();
    size_t df = postings.size();
    size_t numBlocks = (df + BLOCK_SIZE - 1) / BLOCK_SIZE;

    vector<uint32_t> lastDocIDs, docSizes, freqSizes;
    vector<uint8_t> payload;
    uint32_t prev = 0; // carries across blocks

    for (size_t b = 0; b < numBlocks; b++)
    {
        size_t start = b * BLOCK_SIZE;
        size_t end = min(start + BLOCK_SIZE, df);
        vector<uint8_t> docBytes, freqBytes;
        for (size_t i = start; i < end; i++)
        {
            uint32_t docID = static_cast<uint32_t>(postings[i].first);
            varbyteEncode(docID - prev, docBytes);
            varbyteEncode(static_cast<uint32_t>(postings[i].second), freqBytes);
            prev = docID;
        }
        lastDocIDs.push_back(prev);
        docSizes.push_back(docBytes.size());
        freqSizes.push_back(freqBytes.size());
        payload.insert(payload.end(), docBytes.begin(), docBytes.end());
        payload.insert(payload.end(), freqBytes.begin(), freqBytes.end());
    }

    writeu32(indexFile, numBlocks);
    for (size_t b = 0; b < numBlocks; b++)
    {
        writeu32(indexFile, lastDocIDs[b]);
        writeu32(indexFile, docSizes[b]);
        writeu32(indexFile, freqSizes[b]);
    }
    indexFile.write(reinterpret_cast<const char *>(payload.data()), payload.size());

    uint64_t length = static_cast<uint64_t>(indexFile.tellp()) - offset;
    lexFile << term << ' ' << offset << ' ' << length << ' ' << df << '\n';
}
void runMerge(MinHeap &minHeap, vector<ifstream> &runFiles, ofstream &indexFile, ofstream &lexFile)
{
    while (!minHeap.empty())
    {
        vector<runRead> matched = matchingTerms(minHeap);
        string term = matched[0].identTerm;
        vector<pair<int, int>> merged = mergePostingsForTerm(matched);
        writeTermEntry(indexFile, lexFile, term, merged);

        for (auto &entry : matched)
        {
            auto next = readNextLine(runFiles[entry.runIndex], entry.runIndex);
            if (next.has_value())
                minHeap.push(next.value());
        }
    }
}
int main()
{
    int runCount = getRunCount();
    vector<ifstream> runFiles = opensRuns(runCount);
    MinHeap minheap = initializeHeap(runCount, runFiles);

    ofstream indexFile("index.bin", ios::binary);
    ofstream lexFile("lexicon.txt");
    if (!lexFile || !indexFile)
    {
        cerr << "Failed to open output files." << endl;
        return 1;
    }

    runMerge(minheap, runFiles, indexFile, lexFile);
    cout << "Merge complete" << endl;

    return 0;
}
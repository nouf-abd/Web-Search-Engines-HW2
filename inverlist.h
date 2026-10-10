#ifndef INVERLIST_H
#define INVERLIST_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <algorithm>
using namespace std;

const int BLOCK_SIZE = 128; // must match merger.cpp
const uint32_t NOT_FOUND = 0xFFFFFFFF;

struct LexEntry
{
    uint64_t offset;
    uint64_t length;
    int df;
};

map<string, LexEntry> loadLexicon(const string &filename)
{
    map<string, LexEntry> lexicon;
    ifstream lexFile(filename);
    if (!lexFile)
    {
        cerr << "Could not open " << filename << endl;
        exit(1);
    }
    string term;
    LexEntry e;
    while (lexFile >> term >> e.offset >> e.length >> e.df)
        lexicon[term] = e;
    return lexicon;
}

uint32_t varbyteDecode(const vector<uint8_t> &data, size_t &pos)
{
    uint32_t n = 0;
    int shift = 0;
    while (true)
    {
        uint8_t b = data[pos++];
        n |= static_cast<uint32_t>(b & 127) << shift;
        if (!(b & 128))
            break;
        shift += 7;
    }
    return n;
}

uint32_t readu32(ifstream &f)
{
    uint32_t x = 0;
    f.read(reinterpret_cast<char *>(&x), sizeof(x));
    return x;
}

class InvertedList
{
public:
    // returns false if the term isn't in the lexicon
    bool open(const string &term, const map<string, LexEntry> &lexicon, const string &indexName = "index.bin")
    {
        auto it = lexicon.find(term);
        if (it == lexicon.end())
            return false;

        file.open(indexName, ios::binary);
        if (!file)
        {
            cerr << "Could not open " << indexName << endl;
            return false;
        }

        df = it->second.df;
        uint64_t offset = it->second.offset;

        // read the header only, postings stay compressed
        file.seekg(offset);
        numBlocks = readu32(file);
        uint64_t start = offset + 4 + 12 * (uint64_t)numBlocks;
        lastDocIDs.clear();
        blockStart.clear();
        for (int b = 0; b < numBlocks; b++)
        {
            lastDocIDs.push_back(readu32(file));
            uint32_t docSize = readu32(file);
            uint32_t freqSize = readu32(file);
            blockStart.push_back(start);
            start += docSize + freqSize;
        }
        blockStart.push_back(start); // end of the last block

        atEnd = false;
        loadBlock(0);
        return true;
    }

    void close()
    {
        file.close();
        atEnd = true;
    }

    // moves to the first docID >= target, never goes backwards
    uint32_t nextGEQ(uint32_t target)
    {
        if (atEnd)
            return NOT_FOUND;

        int b = loadedBlock;
        while (b < numBlocks && lastDocIDs[b] < target)
            b++;

        if (b >= numBlocks)
        {
            atEnd = true;
            return NOT_FOUND;
        }

        if (b != loadedBlock)
            loadBlock(b);

        while (docIDs[pos] < target)
            pos++;

        return docIDs[pos];
    }

    uint32_t getDocID() { return atEnd ? NOT_FOUND : docIDs[pos]; }
    uint32_t getFreq() { return atEnd ? 0 : freqs[pos]; }
    int getDF() { return df; }

private:
    ifstream file;
    int df = 0;
    int numBlocks = 0;
    vector<uint32_t> lastDocIDs;
    vector<uint64_t> blockStart;

    int loadedBlock = -1;
    int pos = 0;
    bool atEnd = true;
    vector<uint32_t> docIDs, freqs;

    void loadBlock(int b)
    {
        int count = min(BLOCK_SIZE, df - b * BLOCK_SIZE); // last block can be smaller
        vector<uint8_t> bytes(blockStart[b + 1] - blockStart[b]);
        file.clear();
        file.seekg(blockStart[b]);
        file.read(reinterpret_cast<char *>(bytes.data()), bytes.size());

        // gaps continue from the previous block's last docID
        uint32_t prev = (b == 0) ? 0 : lastDocIDs[b - 1];
        size_t p = 0;
        docIDs.clear();
        freqs.clear();
        for (int i = 0; i < count; i++)
        {
            prev += varbyteDecode(bytes, p);
            docIDs.push_back(prev);
        }
        // p now sits at the start of the freq bytes
        for (int i = 0; i < count; i++)
            freqs.push_back(varbyteDecode(bytes, p));

        loadedBlock = b;
        pos = 0;
    }
};

#endif
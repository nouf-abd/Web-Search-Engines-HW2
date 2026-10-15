#include "startup.h"

struct IndexData
{
    map<string, LexEntry> lexicon;
    double averageDocLength;
    vector<uint32_t> documentLengths;
    uint32_t numDocs = 0;
    vector<uint64_t> passageOffset; // snippet
    ifstream collect;               // snippet
};

void docTable(const string &filename, IndexData &data)
{

    uint32_t docId, length;
    ifstream f(filename); // read doctable
    if (!f)
    {
        cerr << "Could not open" << endl;
        exit(1);
    }

    uint64_t totalLength = 0;
    while (f >> docId >> length)
    {
        if (docId >= data.documentLengths.size())
        {
            data.documentLengths.resize(docId + 1); // are we still big enough
        }
        data.documentLengths[docId] = length; // how long is each doc
        totalLength += length;
        data.numDocs++;
    }

    data.averageDocLength = double(totalLength) / data.numDocs;
}

void passaget(const string &filename, IndexData data)
{
    uint64_t bytecounter = 0;
    ifstream f(filename, ::iosbinary) // don't change file endings in binary mode, don't wanna throw off byte count
        if (!f)
    {
        cerr << "Could not open " << filename << endl;
    }

    while (getline(f, line))
    {
        size_t tab = line.find("/t")
    }
}
#endif
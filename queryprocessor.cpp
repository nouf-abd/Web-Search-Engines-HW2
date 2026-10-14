#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include "startup.h"
#include <cmath> // for log
#include <memory>
#include <sstream>
#include <queue>
using namespace std;
using ranked = pair<double, uint32_t>; // score and docid - for minheap so it will

struct BestRes
{
    size_t limit;                                                     // what is the number of res u want to return
    priority_queue<ranked, vector<ranked>, greater<ranked>> minheap2; // smallest on top

    void filling(uint32_t docID, double rank)
    {
        if (minheap2.size() < limit) // how many did you want
        {
            minheap2.push({rank, docID});
        }
        else if (rank > minheap2.top().first) // only if it's better than the current worst
        {
            minheap2.pop();               // remove worst score
            minheap2.push({rank, docID}); // add new worst score - but still better than initial worst
        }
    }

    vector<ranked> bestofbest()
    {
        vector<ranked> result;
        while (!minheap2.empty())
        {
            result.push_back(minheap2.top()); // fill with minheap2 worst to best
            minheap2.pop();
        }
        reverse(result.begin(), result.end()); // now best to worst whee
        return result;
    }
};

vector<string> tokens(string text) // from indexer
{
    vector<string> token_list;
    text.erase(remove_if(text.begin(), text.end(), [](unsigned char c)
                         {
                             return ispunct(c) && c != '\''; // remove punctuation and spaces, but things like "isn't" remain "isn't"
                         }),
               text.end());
    for (auto &i : text)
    { // lowercase all characters
        i = tolower(static_cast<unsigned char>(i));
    }
    stringstream ss(text);
    string token;
    while (ss >> token)
    {
        // Remove apostrophes at bginning and end because we don't want 'hello'
        if (!token.empty() && token.front() == '\'')
        {
            token.erase(0, 1); // starting at position 0, remove 1 character
        }
        if (!token.empty() && token.back() == '\'')
        {
            token.pop_back();
        }
        if (token.empty())
            continue; // skips all empty tokens
        token_list.push_back(token);
    }
    return token_list;
}
vector<string> clean_query(const string &query)
{
    vector<string> terms;
    for (auto &t : tokens(query))
    { // runs tokenizer on query and cleans it
        if (find(terms.begin(), terms.end(), t) == terms.end())
        {                       // searches terms for curr t
            terms.push_back(t); // only pushes back if no duplicate terms
        }
    }
    return terms; // yay no dupes
}

double inversedocfreq(int docfreq, uint32_t numDocs)
{
    return log((numDocs - docfreq + 0.5) / (docfreq + 0.5) + 1.0); // idf is calculated via log(n/how many docs term appeared in), add one to make sure not neg
}

double ranking(uint32_t termfreq, uint32_t document_length, double avgdoclength, double idocfreq)
{
    const double k1 = 1.2, b = 0.75;
    double norm = k1 * (1.0 - b + b * document_length / avgdoclength); // okapi
    double bm25 = idocfreq * (termfreq * (k1 + 1.0)) / (termfreq + norm);
    return bm25;
}
// AND
vector<ranked> conjunctiveDAAT(double averageDocLength, const vector<uint32_t> &documentLengths, const vector<string> &terms, const map<string, LexEntry> &lexicon, size_t limit, uint32_t numDocs, const string &indexFile = "index.bin")
{
    // start daat process
    BestRes res{limit};
    vector<unique_ptr<InvertedList>> lists;
    for (const string &t : terms)
    {
        auto lp = make_unique<InvertedList>();
        if (!lp->open(t, lexicon, indexFile))
        {
            for (auto &l : lists)
            {
                l->close();
            }
            return {};
        }
        lists.push_back(move(lp));
    }
    if (lists.empty())
    {
        return {};
    }

    // begin w shortest list
    sort(lists.begin(), lists.end(), [](const unique_ptr<InvertedList> &a, const unique_ptr<InvertedList> &b)
         {
             return a->getDF() < b->getDF(); // custom comparator to give shortest list first
             // getdf refers to the number of documents that contain that specific term so return the list with the fewest documents first
         });

    vector<double> inverteddocfreqs;

    for (auto &x : lists)
    {
        inverteddocfreqs.push_back(inversedocfreq(x->getDF(), numDocs));
    }

    uint32_t currdocno = 0;
    while (true)
    {
        currdocno = lists[0]->nextGEQ(currdocno); // get next docid from shortest list

        if (currdocno == NOT_FOUND)
        {
            break;
        } // but check

        uint32_t d = currdocno;

        for (size_t i = 1; i < lists.size() && (d = lists[i]->nextGEQ(currdocno)) == currdocno; i++) // check if d==currdocno aka if this list contains this docid
        {
        }
        if (d == NOT_FOUND)
            break;
        if (d > currdocno)
        { // move to the next docid bc it's not in every list
            currdocno = d;
            continue;
        }

        // score
        double score = 0.0;
        for (size_t i = 0; i < lists.size(); i++)
        {
            score += ranking(lists[i]->getFreq(), documentLengths[currdocno], averageDocLength, inverteddocfreqs[i]);
        }
        res.filling(currdocno, score); // add to minheap
        currdocno++;                   // next docid
    }

    for (auto &x : lists)
    {
        x->close();
    }
    return res.bestofbest(); // return the best results
}

// OR
bool terminatedisjunctive(const vector<uint32_t> &documents)
{
    for (uint32_t x : documents)
    {
        if (x != NOT_FOUND)
        {
            return false;
        }
    }
    return true;
}
vector<ranked> disjunctiveDAAT(double averageDocLength, const vector<uint32_t> &documentLengths, const vector<string> &terms, const map<string, LexEntry> &lexicon, size_t limit, uint32_t numDocs, const string &indexFile = "index.bin")
{
    vector<unique_ptr<InvertedList>> lists;
    vector<double> idocfreqs;
    for (auto &t : terms)
    {
        auto list = make_unique<InvertedList>();
        if (!list->open(t, lexicon, indexFile))
            continue; // not in index, so skip ahead
        idocfreqs.push_back(inversedocfreq(list->getDF(), numDocs));
        lists.push_back(move(list));
    }

    BestRes res{limit};

    uint32_t currdocno = 0;
    while (true)
    {
        vector<uint32_t> documents;
        for (auto &l : lists)
        {
            documents.push_back(l->nextGEQ(currdocno));
        }
        if (terminatedisjunctive(documents))
        {
            break;
        }
        currdocno = *min_element(documents.begin(), documents.end()); // get the smallest docid

        double score = 0.0;
        for (size_t i = 0; i < lists.size(); i++)
        {
            if (documents[i] == currdocno)
            { // for this particular document
                score += ranking(lists[i]->getFreq(), documentLengths[currdocno], averageDocLength, idocfreqs[i]);
            }
        }
        res.filling(currdocno, score); // add to minheap
        currdocno++;                   // next docid
    }
    for (auto &l : lists)
    {
        l->close();
    }

    return res.bestofbest(); // return the best results
}

vector<ranked> search(const string &query, bool conjunctive, IndexData &data, size_t limit = 10, const string &indexFile = "index.bin")
{
    vector<string> terms = clean_query(query); // clean query and tokenize
    if (terms.empty())
        return {};
    if (conjunctive)
    {
        return conjunctiveDAAT(data.averageDocLength, data.documentLengths, terms, data.lexicon, limit, data.numDocs, indexFile);
    }
    else
    {
        return disjunctiveDAAT(data.averageDocLength, data.documentLengths, terms, data.lexicon, limit, data.numDocs, indexFile);
    }
}

int main()
{

    return 0;
}

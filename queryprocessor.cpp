#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include "inverlist.h"
#include <queue>
using namespace std;
using ranked = pair<double, uint32_t>; // score and docid - for minheap so it will

struct
{
    size_t limit;                                                     // what is the number of res u want to return
    priority_queue<ranked, vector<ranked>, greater<ranked>> minheap2; // smallest on top

    void filling(uint32_t docID, double rank)
    {
        if (minheap2.size() < limit)
        {
            minheap2.push({rank, docID});
        }
        else
        {
            minheap2.pop();               // remove worst score
            minheap2.push({rank, docID}); // add new worst score
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

string clean_query()
{
    string user_input = "";
    cout << ("What are you searching for?");
    getline(cin, user_input);
    user_input.erase(remove_if(user_input.begin(), user_input.end(), [](unsigned char c)
                               { return ispunct(c) && != '\''; }),
                     user_input.end()); // Remove punctuation except '

    for (char &c : user_input)
    {
        c = tolower(c); // To lowercase
    }
}

double inversedocfreq(int docfreq, uint32_t numDocs)
{
    return log((numDocs + 0.5) / (docfreq + 0.5) / +1.0); // idf is calculated via log(n/how many docs term appeared in), add one to make sure not neg
}

double ranking(uint32_t termfreq, uint32_t document_length, double avgdoclength, double idocfreq)
{
    const double k1 = 1.2, b = 0.75;
    double norm = k1 * (1.0 - b + b * document_length / avgdoclength); // okapi
    double bm25 = idocfreq * (termfreq * (k1 + 1.0)) / (termfreq + norm);
    return bm25;
}

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
                return {};
            }
            lists.push_back(move(lp));
        }
    }
    if (lists.empty())
    {
        return {};
    }

    // begin w shortest list
    sort(lists.begin(), lists.end()), [](const unique_ptr<InvertedList> &a, const unique_ptr<InvertedList> &b)
    {
        return a->getDF() < b->getDF(); // custom comparator to give shortest list first
    };

    vector<double> inverteddocfreqs;

    for (auto &x : lists)
    {
        inverteddocfreqs.push_back(inversedocfreq(x->getdocfreq(), numDocs));
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

        for (size_t i = 1; i < lists.size() && (d = lists[i]->nextGEQ(currdocno) == currdocno); i++)
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
            res.push(currdocno, score); // add to minheap

            currdocno++; // next docid
        }
    }

    for (auto &x : lists)
    {
        x->close();
    }
    return res.bestofbest(); // return the best results
}

int main()
{

    return 0;
}

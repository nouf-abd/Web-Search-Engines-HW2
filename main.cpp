#include <iostream>
#include "queryprocessor.hpp"
using namespace std;

int main()
{
    string query;
    IndexData data;
    loadIndexData(data, "lexicon.txt", "doctable.txt", "collection.tsv");

    bool conjunctive = false;

    while (cout << "What are you searching for?" && getline(cin, query))
    {
        if (query == "/q")
            break;
        if (query == "/and")
        {
            conjunctive = true;
        }
        if (query == "/or")
        {
            conjunctive = false;
        }
    }

    printRes()

        return 0;
}
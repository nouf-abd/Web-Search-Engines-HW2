#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>
#include <map>
#include <cctype>
#include <algorithm>
using namespace std;

const size_t memory_limit = 5000000; // limit for main memory

// tokenizer (from the parser)
vector<string> tokens(string text)
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

void flushRun(map<string, vector<pair<int, int>>> &postings, int &runCount)
{
   ofstream runFile("run_" + to_string(runCount) + ".bin", ios::binary); // opens file
   if (!runFile)
   {
      cerr << "Could not open run_" << runCount << ".bin for writing" << endl;
      exit(1);
   }
   for (const auto &[term, docList] : postings)
   {
      runFile << term;
      for (const auto &[docID, freq] : docList)
         runFile << ' ' << docID << ' ' << freq; // loops through the docs and writes docid, freq
      runFile << '\n';
   }
   postings.clear();
   runCount++;
}

int main(int argc, char **argv)
{
   ifstream file("collection.tsv"); // Opens the collection.tsv file for reading
   if (!file)
   {
      cerr << "Could not open collection.tsv" << endl;
      return 1;
   }
   ofstream mappingFile("docid_map.bin", ios::binary); // Writes to a binary file about the docIDs
   if (!mappingFile)
   {
      cerr << "Could not open docid_map.bin for writing" << endl;
      return 1;
   }
   map<string, vector<pair<int, int>>> postings; // map of terms to a vector of pairs (docID, frequency)
   size_t postingCount = 0;
   int runCount = 0;
   int nextDocID = 0;
   string line;
   while (getline(file, line))
   {
      vector<string> parts;
      string field;
      istringstream ss(line);
      while (getline(ss, field, '\t'))
      { // splits the line into parts based on tabs because its a tsv file
         parts.push_back(field);
      }

      if (parts.empty())
         continue;
      int docID = nextDocID++; // counter to assign a unique docIDs
      string text = (parts.size() > 1) ? parts[1] : "";

      uint64_t originID = stoull(parts[0]);                                           // should we add try catch to this ?
      mappingFile.write(reinterpret_cast<const char *>(&originID), sizeof(originID)); // mapping msMARCOID to docID

      map<string, int> termFreq;
      for (const auto &token : tokens(text))
      {
         termFreq[token]++;
      }
      for (const auto &[term, freq] : termFreq)
      {
         postings[term].push_back({docID, freq});
         postingCount++;
      }
      if (postingCount >= memory_limit)
      {
         flushRun(postings, runCount);
         postingCount = 0;
      }
   }
   if (!postings.empty())
   {
      flushRun(postings, runCount);
   }
   mappingFile.close();
   cout << "Parsed " << nextDocID << " documents" << endl;
   ofstream count_file("run_count.txt");
   count_file << runCount;
   count_file.close();
   return 0;
}

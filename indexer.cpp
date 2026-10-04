#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>
using namespace std;

int main(int argc, char** argv) {
   ifstream file("collection.tsv");
   if (!file) {
      cerr << "Could not open collection.tsv" << endl;
      return 1;
   }
   ofstream mappingFile("docid_map.bin", ios::binary);
   if (!mappingFile) {
      cerr << "Could not open docid_map.bin for writing" << endl;
      return 1;
   }
   int nextDocID = 0;
   string line;
   while (getline(file, line)) {
      vector<string> parts;
      string field;
      istringstream ss(line);
      while (getline(ss, field, '\t')) {
            parts.push_back(field);
      }

      if (parts.empty()) continue; 
      int docID = nextDocID++;
      string text = (parts.size() > 1) ? parts[1] : "";

      uint64_t originID = stoull(parts[0]);
      mappingFile.write(reinterpret_cast<const char*>(&originID), sizeof(originID));
      }
   mappingFile.close();
   cout << "Parsed " << nextDocID << " documents" << endl;
   return 0;
   }
   



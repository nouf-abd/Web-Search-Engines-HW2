#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

int main(int argc, char** argv) {
   ifstream file("collection.tsv");
   if (!file) {
      cerr << "Could not open collection.tsv" << endl;
      return 1;
   }

   string line;
   while (getline(file, line)) {
      vector<string> parts;
      string field;
      istringstream ss(line);
      while (getline(ss, field, '\t')) {
            parts.push_back(field);
      }

      if (parts.empty()) continue;  
      cout << "First " << parts.size() << " elements: " << parts[0] << endl;
   }
   return 0;
}

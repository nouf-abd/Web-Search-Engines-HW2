# Web-Search-Engines-HW2

Summary: Disk-based inverted index search engine written in C++ using BM25 ranking over MS MARCO passages.

Raemae Kok & Nouf Aljomiha

Files: 

1. indexer.cpp 
2. inverlist.hpp 
3. main.cpp 
4. merger.cpp 
5. queryprocessor.hpp 
6. startup.h 


Requirements:
1. C++17 compiler
2. collection.tsv (Ms Marco)
3. Disk space needed: approx. 12 GB

How to build and run: 

*Note that MAX_DOCS in indexer.cpp controls how many passages are indexed. Right now set at 10000000.*

Compile:
1. g++ -std=c++17 -O2 indexer.cpp -o indexer
2. g++ -std=c++17 -O2 merger.cpp -o merger
3. g++ -std=c++17 -O2 main.cpp -o query

Build index: 
caffeinate -i ./indexer
(If on Mac, ulimit -n 1024)
caffeinate -i ./merger

Use query processor: 

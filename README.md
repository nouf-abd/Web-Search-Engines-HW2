# Web-Search-Engines-HW2

#parser.py 
#def tokeniser(text):
-Parses the text and cleans it of stopwords using stopword library on github and removes punctuation

Download collection.tar.gz from the MS MARCO passage ranking page and extract collection.tsv into this folder.

To Run
g++ -std=c++17 indexer.cpp -o indexer
g++ -std=c++17 merger.cpp -o merger
g++ -std=c++17 test_inverlist.cpp -o test_inverlist

.\indexer.exe
.\merger.exe
.\test_inverlist.exe
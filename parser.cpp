#include <iostream> 
#include <string>
#include <algorithm>
#include <cctype>
#include <vector> 
#include <sstream>
using namespace std;

// Define data - download tsv and open the file

vector<string> tokens(string text){
    vector<string> token_list;
    text.erase(remove_if(text.begin(),text.end(),[](unsigned char c){
        return ispunct(c) && c != '\''; // remove punctuation and spaces, but things like "isn't" remain "isn't"
    }),text.end()); 
    for(auto& i:text){ // lowercase all characters
        i=tolower(static_cast<unsigned_char>(i));
    }
    stringstream ss(text);
    string token;
    while(ss >> token){
        // Remove apostrophes at bginning and end because we don't want 'hello' 
        if(!token.empty() && token.front()=='\''){
            token.erase(0,1) // starting at position 0, remove 1 character
        }
        if(!token.empty() && token.back()=='\''){
            token.pop_back();
        }
        token_list.push_back(token);
    }
    return token_list;
}



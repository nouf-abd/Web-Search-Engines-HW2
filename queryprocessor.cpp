#include <iostream> 
#include <string>
#include <algorithm>
#include <cctype>
using namespace std;

// function to clean query - keep stopwords or no 
// remember to clean the query the same way you did it in parser
// 

string clean_query() {
    string user_input="";
    cout<<("What are you searching for?");
    cin>>user_input;
    user_input.erase(remove_if(user_input.begin(),user_input.end(),[](unsigned char c){
        return ispunct(c);
    }),user_input.end()); // Remove punctuation

    for(char &c:user_input){
        c=tolower(c); // To lowercase
    }
}

#pragma once // don't want to include in query.cpp
#include <string> 
#include <unordered_map>
using namespace std; 

struct lexicalfairy{
    uint64_t bposition; // byte position
    uint64_t length; 
    uint32_t document_frequency; // how many documents contain this term
}
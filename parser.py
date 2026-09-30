import re

#Define data - download tsv and open the file
Open .tsv 
#Function for tokenization
def tokenizer(text): 
    text=text.lower()
    rawtoken=re.findall(r"[a-z0-9']+", text) #skips punctuation and spaces except for ' ie. isn't stays isn't
    tokens=[t.strip("'") for t in rawtoken]
    return tokens #suggested 
#Handle special characters + clean text - decision to be made whether to keep filler words etc.
def cleaner: 
#Map terms to documents 
def dora: 
#Store the index
def storeindex: 
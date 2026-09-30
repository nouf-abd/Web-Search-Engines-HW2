import re
import requests 
#Define data - download tsv and open the file
Open .tsv 
#Function for tokenization
def tokenizer(text): 
    stopwords_list = requests.get("https://gist.githubusercontent.com/rg089/35e00abf8941d72d419224cfd5b5925d/raw/12d899b70156fd0041fa9778d657330b024b959c/stopwords.txt").content
    stopwords = set(stopwords_list.decode().splitlines()) 
    text=text.lower()
    rawtoken=re.findall(r"[a-z0-9']+", text) #skips punctuation and spaces except for ' ie. isn't stays isn't
    tokens=[t.strip("'") for t in rawtoken] #but 'Hello' would become hello
    tokens=[t for t in tokens if not t in stopwords] #Remove stopwords 
    #How to remove stop words 
    return tokens #suggested 
#Map terms to documents 
def dora: 
#Store the index
def storeindex: 
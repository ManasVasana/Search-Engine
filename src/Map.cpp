#include "Map.hpp"
using namespace std;
// Constructor
Mymap::Mymap(int size, int buffersize) : size(size), buffersize(buffersize)
{
    // Allocate arrays
    documents = new char *[size];
    doc_lengths = new int[size];

    // Initialize to prevent undefined behavior
    for (int i = 0; i < size; i++)
    {
        documents[i] = nullptr;
        doc_lengths[i] = 0;
    }
}
// Destructor
Mymap::~Mymap()
{
    for (int i = 0; i < size; i++)
    {
        delete[] documents[i];
    }
    delete[] documents;
    delete[] doc_lengths;
}
int Mymap::insert(char* line, int i){
    if(line == nullptr || i < 0 || i >= size){
        return -1;
    }
    
    // Remove newline if present
    int len = strlen(line);
    if(len > 0 && line[len-1] == '\n'){
        line[len-1] = '\0';
        len--;
    }
    
    // Trim leading spaces
    char* start = line;
    while(*start == ' ' || *start == '\t'){
        start++;
        len--;
    }
    
    // Trim trailing spaces
    char* end = start + len - 1;
    while(end > start && (*end == ' ' || *end == '\t')){
        *end = '\0';
        end--;
        len--;
    }
    
    // Allocate memory for this document
    documents[i] = new char[len + 1];
    strcpy(documents[i], start);
    // doc_lengths will be set by split() with word count for BM25
    
    return 1;
} 
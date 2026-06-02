#include "Document_store.hpp"
#include <fstream>
#include <string>
using namespace std;
int read_sizes(int *linecounter,int *maxlength, char *file_name){
    ifstream file(file_name);
    if(!file.is_open()){
        cout<<"Cannot open file: "<<file_name<<endl;
        return -1;
    }
    
    // Check if file is empty
    if(file.peek() == EOF){
        cout<<"File is empty: "<<file_name<<endl;
        file.close();
        return -1;
    }
    
    string line;
    int current_length;
    while(getline(file, line)){
        current_length = line.length();
        if(current_length>*maxlength)
            *maxlength=current_length;
        (*linecounter)++;
    }
    file.close();
    return 1;
}
void split(char* temp,int id,TrieNode* trie,Mymap* mymap){
    char* token;
    token = strtok(temp, " \t");
    int i=0;
    while(token != NULL){
        i++;
        trie->insert(token, id);
        token = strtok(NULL, " \t");
    }
    mymap->setlength(id,i);

}
int read_input(Mymap* mymap,TrieNode *trie, char* file_name){
    ifstream file(file_name);
    if(!file.is_open()){
        cout << "Error opening file: " << file_name << endl;
        return -1;
    }
    string line;
    char *temp = (char*)malloc(mymap->get_buffersize()*sizeof(char));
    for(int i=0;i<mymap->get_size();i++){
        if(!getline(file, line)){
            cout << "Error reading line " << i << endl;
            free(temp);
            file.close();
            return -1;
        }
        char *line_cstr = (char*)malloc((line.length() + 1) * sizeof(char));
        strcpy(line_cstr, line.c_str());
        if (mymap->insert(line_cstr, i) == -1) {
            cout << "Error inserting line " << endl;
            free(line_cstr);
            free(temp);
            file.close();
            return -1;
        }
        free(line_cstr);
        strcpy(temp,mymap->getDocument(i));
        split(temp,i,trie,mymap);
    }
    free(temp);
    file.close();
    return 1;
}
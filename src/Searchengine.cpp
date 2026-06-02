#include "Searchengine.hpp"

using namespace std;

int inputmanager(char* input, TrieNode* trie, Mymap* mymap, int k){
    char* token=strtok(input, " \t\n");
    
    if(token == NULL){
        return 0;  // Empty input, continue
    }
    
    if(!strcmp(token,"/search")){
        search(token,trie,mymap,k);
        return 1;
    }
    else if(!strcmp(token,"/df")){
        df(trie);
        return 1;
    }
    else if(!strcmp(token,"/tf")){
        tf(token,trie);
        return 1;
    }
    else if(!strcmp(token,"/exit")||!strcmp(token,"/quit")){
        return 2;  // Signal to exit
    }
    else{
        cout<<"Unknown command: "<<token<<endl;
        cout<<"Available commands: /search, /df, /tf, /exit, /quit"<<endl;
        return 0;  // Continue, not exit
    }
}
// read document/books/searchengine.md for more information
int main(int argc, char** argv) {
    if (argc != 5 ||
        strcmp(argv[1], "-d") != 0 ||
        strcmp(argv[3], "-k") != 0) {
        cout << "Wrong arguments. Usage: -d <file> -k <number>" << endl;
        return -1;
    }

    cout << "Please wait..." << endl;
    int linecounter = 0;
    int maxlength = -1;
    int k;
    try {
        k = stoi(argv[4]); 
    } catch (...) {
        cout << "Invalid value for -k (must be an integer)" << endl;
        return -1;
    }
    
    if(read_sizes(&linecounter, &maxlength,argv[2]) == -1){
        return -1;
    }

    Mymap *mymap=new Mymap(linecounter, maxlength);
    TrieNode *trie=new TrieNode();

    if(read_input(mymap,trie, argv[2]) == -1){
        delete (mymap);
        delete (trie);
        return -1;
    }
    cout<<"File read successfully. Lines: " << linecounter << ", Max Length: " << maxlength << endl;
    std::string input;
    while(1){
        cout << "Enter query (or type '/exit' to /quit): ";
        if(!getline(cin, input)){
            // EOF or error
            break;
        }
        
        char* cinput = new char[input.size() + 1];
        input.copy(cinput, input.size());
        cinput[input.size()] = '\0';
        int ret = inputmanager(cinput, trie, mymap, k);
        delete [] cinput;

        if(ret==2){
            cout<<"Exiting program..."<<endl;
            break;
        }
        // ret == 0 or 1: continue
    }
    
    delete (mymap);
    delete (trie);
    return 0;
}
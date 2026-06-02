#include <iostream>
#include <cstdlib>
#ifndef SCORE_HPP
#define SCORE_HPP
using namespace std;
class Scorelist
{
    int id;
    Scorelist *next;
    public:
        Scorelist(int docId=-1): id(docId){next=NULL;};
        ~Scorelist();
        void insert(int docId);
        Scorelist* get_next(){return next;};
        int get_id(){return id;};
};
#endif
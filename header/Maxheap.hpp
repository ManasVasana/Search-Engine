#include <iostream>
#include <cstdlib>
#include <cstring>
#ifndef MAXHEAP_HPP
#define MAXHEAP_HPP
using namespace std;
class Maxheap
{
    double* heap;
    int *ids;
    int curnumofscores;
    int maxnumofscores;
    void swapscore(int index1,int index2);
    int minindex(int low,int high);
    public:
    Maxheap(int k);
    ~Maxheap(){
        free(heap);
        free(ids);
    }
    void insert(double score, int id);
    int MaxChild(int number1,int number2);
    double remove();
    int get_id(){
        return ids[0];
    }
    int get_count(){
        return curnumofscores;
    }
    double get_score(){
        return heap[0];
    }

};
#endif
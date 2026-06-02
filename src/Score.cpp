#include "Score.hpp"
using namespace std;
Scorelist::~Scorelist()
{
    if(next!=NULL)
        delete(next);
}
void Scorelist::insert(int docId)
{
    if(id==docId){
        return;
    }
    else
    if(id==-1){
        id=docId;
    }
    else{
        if(next==NULL){
            next=new Scorelist(docId);
        }
        else{
            next->insert(docId);
        }
    }
}
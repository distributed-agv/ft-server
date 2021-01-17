#ifndef ASTAR_H
#define ASTAR_H
#include <iostream>
#include <queue>
#include <vector>
#include <stack>
#include<algorithm>
using namespace std;


typedef struct Node
{
    int x,y;
    int g; //起始点到当前点实际代价
    int h;//当前节点到目标节点最佳路径的估计代价
    int f;//估计值
    Node* father;
    Node(int x,int y)
    {
        this->x = x;
        this->y = y ;
        this->g = 0;
        this->h = 0;
        this->f = 0;
        this->father = NULL;
    }
    Node(int x,int y,Node* father)
    {
        this->x = x;
        this->y = y ;
        this->g = 0;
        this->h = 0;
        this->f = 0;
        this->father = father;
    }
}Node;
class Astar{
    public:
        Astar(int ,int,int);
        ~Astar();
        bool search(Node* startPos,Node* endPos,int**,int*,int*);
        bool randomStep(int x1,int y1,int *x,int *y);
        void checkPoit(int x,int y,Node* father,int g);
        void NextStep(Node* currentPoint);
        int isContains(vector<Node*>* Nodelist ,int x,int y);
        void countGHF(Node* sNode,Node* eNode,int g);
        static bool compare(Node* n1,Node* n2);
        bool unWalk(int x,int y);
        void printPath(Node* current,int*,int*);
        void printMap();
        vector<Node*> openList;
        vector<Node*> closeList;
        Node *startPos;
        Node *endPos;
        int WeightW = 1;// 正方向消耗
        int car=0;
        int row = 6;
        int col = 8;
};
#endif
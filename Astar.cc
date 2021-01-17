#include "Astar.h"
int** map;

Astar::Astar(int col,int row,int car)
{
    this->col=col;
    this->row=row;
    this->car=car;
}
Astar::~Astar()
{

}
bool Astar::randomStep(int x1,int y1,int *x,int *y){
   if(x1-1>=0 && map[x1-1][y1]==car){
       *x=x1-1;
       *y=y1;
       return true;
   }
   if(y1-1>=0 && map[x1][y1-1]==car){
       *x=x1;
       *y=y1-1;
       return true;
   }
   if(x1+1<=row && map[x1+1][y1]==car){
       *x=x1+1;
       *y=y1;
       return true;
   }
   if(y1+1<=col&&map[x1][y1+1]==car){
       *x=x1;
       *y=y1+1;
       return true;
   }
   return false;
}

bool Astar::search( Node* startPos,Node* endPos,int** mapTmp,int *x,int*y)
{
    map=mapTmp;
    if (startPos->x < 0 || startPos->x > row || startPos->y < 0 || startPos->y >col ||
        endPos->x < 0 || endPos->x > row || endPos->y < 0 || endPos->y > col)
        return true;
    Node* current;
    this->startPos = startPos;
    this->endPos = endPos;
    openList.push_back(startPos);
    //主要是这块，把开始的节点放入openlist后开始查找旁边的4个节点，如果坐标超长范围或在closelist就return 如果已经存在openlist就对比当前节点到遍历到的那个节点的G值和当前节点到原来父节点的G值 如果原来的G值比较大 不用管 否则重新赋值G值 父节点 和f 如果是新节点 加入到openlist 直到opellist为空或找到终点
    while(openList.size() > 0)
    {
        current = openList[0];
        if (current->x == endPos->x && current->y == endPos->y)
        { 
            printPath(current,x,y);
            openList.clear();
            closeList.clear();
            if(*x==endPos->x && *y==endPos->y&& map[*x][*y]!=car){
                return randomStep(startPos->x,startPos->y,x,y);
            }
            return true;
        }
        NextStep(current);
        closeList.push_back(current);
        openList.erase(openList.begin());
        sort(openList.begin(),openList.end(),compare);
    }
    return false;
}

void Astar::checkPoit( int x,int y,Node* father,int g)
{
    if (x < 0 || x > row || y < 0 || y > col)
        return;
    if (this->unWalk(x,y))
        return;
    if (isContains(&closeList,x,y) != -1)
        return;
    int index;
    if ((index = isContains(&openList,x,y)) != -1)
    {
        Node *point = openList[index];
        if (point->g > father->g + g)
        {
            point->father = father;
            point->g = father->g + g;
            point->f = point->g + point->h;
        }
    }
    else
    {
        Node * point = new Node(x,y,father);
        countGHF(point,endPos,g);
        openList.push_back(point);
    }
}

void Astar::NextStep( Node* current )
{
    checkPoit(current->x - 1,current->y,current,WeightW);//左
    checkPoit(current->x + 1,current->y,current,WeightW);//右
    checkPoit(current->x,current->y + 1,current,WeightW);//上
    checkPoit(current->x,current->y - 1,current,WeightW);//下
    
}

int Astar::isContains(vector<Node*>* Nodelist, int x,int y )
{
    for (int i = 0;i < Nodelist->size();i++)
    {
        if (Nodelist->at(i)->x  == x && Nodelist->at(i)->y == y)
        {
            return i;
        }
    }
    return -1;
}

void Astar::countGHF( Node* sNode,Node* eNode,int g)
{
    int h = abs(sNode->x - eNode->x) + abs(sNode->y - eNode->y) * WeightW;
    int currentg = sNode->father->g + g;
    int f = currentg + h;
    sNode->f = f;
    sNode->h = h;
    sNode->g = currentg;
}

bool Astar::compare( Node* n1,Node* n2 )
{
    return n1->f < n2->f;
}

bool Astar::unWalk( int x,int y)
{
    if(x==endPos->x && y==endPos->y)
        return false;
    if (map[x][y] != -1&&map[x][y]!=car)
        return true;
    return false;
}

void Astar::printPath( Node* current ,int *x,int *y)
{
     while (current->father!=NULL) {
        if(current->father->x==this->startPos->x&&current->father->y==this->startPos->y)
        {
            
            *x=current->x;
            *y=current->y;
            
            return;
             
        }
        current=current->father;
    }
}
void Astar::printMap()
{
    for(int i=0;i<=row;i++){
        for(int j=0;j<=col;j++){
            printf("%d ",map[i][j]);
        }
        printf("\n");
    }
}
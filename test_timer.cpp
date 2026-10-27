#include <graphics.h>
#include <windows.h>
#include <stdio.h>

#define WINVER 0x0500
#define ROW_COUNTS 8
#define COLUMN_COUNTS 8
#define CELL_WIDTH (int)(960/COLUMN_COUNTS)
#define MAX_TREES 100
#define PLAYER_RECT_WIDTH (int)(CELL_WIDTH*0.625)  // 玩家矩形的宽度
#include <conio.h>
#include <time.h>
#include <stdlib.h>
struct Pos{
    int pos_id;
    int x;
    int y;
    bool up;
    bool down;
    bool left;
    bool right;
    bool isWall;// true表示墙，false表示通路
};
struct Pos_Stack{
    int pos_id;
    
    struct Pos_Stack *stackTop;
    int stackSize;
};
void initPosStack(struct Pos_Stack *stack)
{
    stack->pos_id = -1;
    // stack->isWall = true;
    stack->stackTop = NULL;
    stack->stackSize = 0;
}
void pushPosStack(struct Pos_Stack *stack, int pos_id, bool isWall)
{
    struct Pos_Stack *newNode = (struct Pos_Stack *)malloc(sizeof(struct Pos_Stack));
    newNode->pos_id = pos_id;
    // newNode->isWall = isWall;
    newNode->stackTop = stack->stackTop;
    stack->stackTop = newNode;
    stack->stackSize++;
}
int popPosStack(struct Pos_Stack *stack)
{
    if (stack->stackSize == 0)
    {
        return -1; // 栈为空，返回-1表示无法弹出
    }
    struct Pos_Stack *topNode = stack->stackTop;
    int pos_id = topNode->pos_id;
    stack->stackTop = topNode->stackTop;
    free(topNode);
    stack->stackSize--;
    return pos_id;
} 
void preorderPosStack(struct Pos_Stack *stack)
{
    struct Pos_Stack *current = stack->stackTop;
    while (current != NULL)
    {
        printf("pos_id: %d--> ", current->pos_id);
        current = current->stackTop;
    }
}
/* 树/森林的结点：孩子兄弟表示法 */
typedef struct CSNode {
    struct Pos pos;                  /* 数据域 */
    struct CSNode *firstchild;  /* 指向第一个孩子 */
    struct CSNode *nextsibling; /* 指向下一个兄弟 */
} CSNode, *CSTree;

/* 森林：多棵树的根结点组成一个线性表（这里用指针数组表示） */
#define MAX_TREES 100

typedef struct Forest {
    CSTree trees[MAX_TREES]; /* 每棵树的根结点 */
    int treeCount=0;           /* 树的棵数 */
} Forest;
// #include <windows.h>
// #include <graphics.h>

CSNode *createNode(int data)
{
    CSNode *node = (CSNode *)malloc(sizeof(CSNode));
    if (node == NULL) {
        printf("内存分配失败\n");
        exit(1);
    }
    node->pos.pos_id = data;
    node->firstchild = NULL;
    node->nextsibling = NULL;
    return node;
}
/* 初始化空森林 */
void initForest(Forest *f)
{
    f->treeCount = 0;
    for (int i = 0; i < MAX_TREES; i++) {
        f->trees[i] = NULL;
    }
}

/* 添加一棵树（传入该树的根结点） */
int addTree(Forest *f, CSTree root)
{
    if (f->treeCount >= MAX_TREES) {
        printf("森林已满\n");
        return 0;
    }
    f->trees[f->treeCount++] = root;
    return 1;
}
struct Pos g_posList[ROW_COUNTS*COLUMN_COUNTS]={0};
int g_isAdjoinList[ROW_COUNTS*COLUMN_COUNTS][ROW_COUNTS*COLUMN_COUNTS] = {0};
// int g_isAdjoinTable[ROW_COUNTS*COLUMN_COUNTS][ROW_COUNTS*COLUMN_COUNTS] = {0};
void draw_up(int x,int y){
    line(x,y,x+CELL_WIDTH,y);
}
void draw_down(int x,int y){
    line(x,y,x+CELL_WIDTH,y);
}
void draw_left(int x,int y){
    line(x,y,x,y+CELL_WIDTH);
}
void draw_right(int x,int y){
    line(x,y,x,y+CELL_WIDTH);
}

int random_one_or_minus_one()
{
    double p = (double)rand() / RAND_MAX;  /* 0.0 ~ 1.0 */
    int r = (p < 0.5) ? -1 : 1;
    return r;
}

void preOrderTree(CSTree root)
{
    if (root == NULL) return;
    printf("%d -> ", root->pos.pos_id);
    preOrderTree(root->firstchild);
    preOrderTree(root->nextsibling);
}

void preOrderForest(Forest *f)
{
    for (int i = 0; i < f->treeCount; i++)
        preOrderTree(f->trees[i]);
    printf("\n");
}
int search_tree(CSTree root0, CSTree root, int target_pos_id)
{
    if (root == NULL||root0 == NULL) return -1;
    int current = root0->pos.pos_id;
    if (root->pos.pos_id == target_pos_id) {
        int result = current;
        return result;
    }
    int result = search_tree(root0, root->firstchild, target_pos_id);
    if (result != -1) return result;
    return search_tree(root0, root->nextsibling, target_pos_id);
}
enum WALL_TYPE {
    WALL_UP = 1,
    WALL_DOWN = 2,
    WALL_LEFT = 3,
    WALL_RIGHT = 4,
    WALL_DEFAULT = 0
};
WALL_TYPE judge_wall_type(int pos_id1,int pos_id2){
    int y1 = (int)(pos_id1/COLUMN_COUNTS);
    int x1 = pos_id1%COLUMN_COUNTS;
    int y2 = (int)(pos_id2/COLUMN_COUNTS);
    int x2 = pos_id2%COLUMN_COUNTS;
    if(y1==y2){
        if(x1==x2+1){
            return WALL_LEFT;
        }else if(x1==x2-1){
            return WALL_RIGHT;
        }
    }else if(x1==x2)
    {
        if(y1==y2+1){
            return WALL_UP;
        }else if(y1==y2-1){
            return WALL_DOWN;
        }
    }
    return WALL_DEFAULT;
}
bool is_id_in_tree(CSTree root, int target_pos_id)
{
    if (root == NULL) return false;
    if (root->pos.pos_id == target_pos_id) return true;
    if (is_id_in_tree(root->firstchild, target_pos_id)) return true;
    // return is_id_in_tree(root->nextsibling, target_pos_id);
    return false;
}
int get_next_pos_id(int current_pos_id)
{
    int r = rand() % 4;
    int current_pos_x = current_pos_id % COLUMN_COUNTS;
    int current_pos_y = (int)(current_pos_id / COLUMN_COUNTS);
    if(r==0){
        return ((current_pos_y-1)*COLUMN_COUNTS+current_pos_x)>=0&&(current_pos_y-1)*COLUMN_COUNTS+current_pos_x<ROW_COUNTS*COLUMN_COUNTS?((current_pos_y-1)*COLUMN_COUNTS+current_pos_x):get_next_pos_id(current_pos_id);
        //上
    }else if(r==1){
        return (current_pos_y+1)*COLUMN_COUNTS+current_pos_x>=0&&(current_pos_y+1)*COLUMN_COUNTS+current_pos_x<ROW_COUNTS*COLUMN_COUNTS?(current_pos_y+1)*COLUMN_COUNTS+current_pos_x:get_next_pos_id(current_pos_id);
        //下    
    }else if(r==2){
        return current_pos_id-1>=0&&current_pos_x!=0&&current_pos_id-1<ROW_COUNTS*COLUMN_COUNTS?current_pos_id-1:get_next_pos_id(current_pos_id);
        //左
    }else{
        return current_pos_id+1>=0&&current_pos_x!=(COLUMN_COUNTS-1)&&current_pos_id+1<ROW_COUNTS*COLUMN_COUNTS?current_pos_id+1:get_next_pos_id(current_pos_id);
        //右
    }
}

struct Pos g_hasVisitedPosList[ROW_COUNTS*COLUMN_COUNTS*100]={0};
void init_globalHasVisitedPosList(){
    for(int i=0;i<ROW_COUNTS*COLUMN_COUNTS*100;i++){
        g_hasVisitedPosList[i].pos_id=-1;
        g_hasVisitedPosList[i].isWall=false;
    }
}
bool is_pos_visited(int pos_id){
    for(int i=0;i<ROW_COUNTS*COLUMN_COUNTS*100;i++){
        if(g_hasVisitedPosList[i].pos_id==pos_id){
            return true;
        }
    }
    return false;
}
bool is_pos_has_neighbor_not_in_visited(int pos_id){
    int pos_x = pos_id%COLUMN_COUNTS;
    int pos_y = (int)(pos_id/COLUMN_COUNTS);
    int neighbor_pos_id[4] = {-1,-1,-1,-1};
    neighbor_pos_id[0] = (pos_y-1)*COLUMN_COUNTS+pos_x>=0&&(pos_y-1)*COLUMN_COUNTS+pos_x<ROW_COUNTS*COLUMN_COUNTS?(pos_y-1)*COLUMN_COUNTS+pos_x:-1;//up
    neighbor_pos_id[1] = (pos_y+1)*COLUMN_COUNTS+pos_x>=0&&(pos_y+1)*COLUMN_COUNTS+pos_x<ROW_COUNTS*COLUMN_COUNTS?(pos_y+1)*COLUMN_COUNTS+pos_x:-1;//down
    neighbor_pos_id[2] = pos_y*COLUMN_COUNTS+pos_x-1>=0&&pos_y*COLUMN_COUNTS+pos_x-1<ROW_COUNTS*COLUMN_COUNTS?pos_y*COLUMN_COUNTS+pos_x-1:-1;//left
    neighbor_pos_id[3] = pos_y*COLUMN_COUNTS+pos_x+1>=0&&pos_y*COLUMN_COUNTS+pos_x+1<ROW_COUNTS*COLUMN_COUNTS?pos_y*COLUMN_COUNTS+pos_x+1:-1;//right
    for(int i=0;i<4;i++){
        if(neighbor_pos_id[i]!=-1&&!is_pos_visited(neighbor_pos_id[i])){
            return true;
        }
    }
    return false;
}
void build_forest(){
    //第一步把四周围墙存到墙表中
    for (int i=0;i<ROW_COUNTS*COLUMN_COUNTS;i++)
    {
        g_posList[i].pos_id = i;
        g_posList[i].up = true;
        g_posList[i].down = true;
        g_posList[i].left = true;
        g_posList[i].right = true;
        g_posList[i].x=i%COLUMN_COUNTS;
        g_posList[i].y=(int)(i/COLUMN_COUNTS);
        if(i!=0) g_posList[i].isWall = true;
        else g_posList[i].isWall = false;
        if(g_posList[i].x==0){
            g_posList[i].left = true;
            
        }
        if(g_posList[i].y==0){
            g_posList[i].up=true;
        }
        if(g_posList[i].x==COLUMN_COUNTS-1){
            g_posList[i].right=true;
        }
        if(g_posList[i].y==ROW_COUNTS-1){
            g_posList[i].down=true;
        }
    }


    // Forest f;
    // initForest(&f);

    Pos_Stack pos_stack;
    initPosStack(&pos_stack);
    pushPosStack(&pos_stack, 0, false); // 将起点入栈，标记为通路
    int count = 0;
    int next_pos_id_list[4] = {-1,-1,-1,-1};
    int next_pos_id = -1;
    int start_pos_id = 0;
    int end_pos_id = ROW_COUNTS*COLUMN_COUNTS-1;
    // CSNode *root = createNode(start_pos_id);
    int current_pos_id = start_pos_id;
    
        
    // for(int i=0;i<ROW_COUNTS*COLUMN_COUNTS;i++)
    // {
        
    //     int pos_1_x = i%COLUMN_COUNTS;
    //     int pos_1_y = (int)(i/COLUMN_COUNTS);
    //     int next_pos_id = -1;
    //     for(int j=i+1;j<ROW_COUNTS*COLUMN_COUNTS;j++)
    //     {
    //         int pos_2_x = j%COLUMN_COUNTS;
    //         int pos_2_y = (int)(j/COLUMN_COUNTS);
    //         // for(int k=0;k<4;k++)
    //         // {
    //         //     int count = 0;
    //         //     next_pos_id = next_pos_id_list[k];
    //         //     if(next_pos_id != -1)
    //         //     {
    //         //         current_pos_id = next_pos_id;
    //         //     }
    //         if(g_isAdjoinList[i][j]==1&&(i==current_pos_id))
    //         {   
    //             next_pos_id = j;
    //             CSNode *rootNode = createNode(start_pos_id);
    //             if(is_id_in_tree(rootNode, j))
    //             {
    //                 continue;
    //             }
                    
    //             addTree(&f, rootNode);
    //             current_pos_id = start_pos_id;
    //             CSNode *nextNode = rootNode;
    //             //改算法 改成DFS-回溯法


                
    //         }
    //         // }
            
    //     }
        
        
    //     // (nextNode++)->firstchild = nextNode;
    // }

    //以下为生成迷宫的DFS-回溯算法
    int count1 = 1;
    init_globalHasVisitedPosList();
    g_hasVisitedPosList[0].pos_id = 0;
    while(pos_stack.stackTop!=NULL)
    {
       current_pos_id = pos_stack.stackTop->pos_id;
       if(current_pos_id==end_pos_id)   
       {
            break;
       }else{
            
            if(is_pos_has_neighbor_not_in_visited(current_pos_id)){
                next_pos_id = get_next_pos_id(current_pos_id);
                if(!is_pos_visited(next_pos_id)){
                    pushPosStack(&pos_stack, next_pos_id, false);
                    g_hasVisitedPosList[count1].pos_id = next_pos_id;
                    count1++;
                    current_pos_id = next_pos_id;
                }else{
                    next_pos_id = get_next_pos_id(current_pos_id); 
                }
            }else{
                int huisu_posid = popPosStack(&pos_stack);
                current_pos_id = pos_stack.stackTop->pos_id;
                g_hasVisitedPosList[count1].pos_id = current_pos_id;
                count1++;
                
            }
       } 

    }

    preorderPosStack(&pos_stack);

    printf("\n");

    for(int i=0;i<count1;i++){
        printf("%d -- ",g_hasVisitedPosList[i].pos_id);
    }
    // }

    // preOrderForest(&f);
    // CSNode *roott = f.trees[0];
    // int i = roott->pos.pos_id;
    // int j = roott->firstchild->pos.pos_id;
    // while(roott->firstchild!=NULL)
    // {
    //     int i = roott->pos.pos_id;
    //     int j = roott->firstchild->pos.pos_id;
    //     printf("i=%d,j=%d\n",i,j);
    //     WALL_TYPE wall_type = judge_wall_type(i,j);
    //     if(wall_type==WALL_UP){
    //         g_posList[i].up=false;
    //         g_posList[j].down=false;
    //     }else if(wall_type==WALL_DOWN){
    //         g_posList[i].down=false;
    //         g_posList[j].up=false;
    //     }else if(wall_type==WALL_LEFT){
    //         g_posList[i].left=false;
    //         g_posList[j].right=false;
    //     }else if(wall_type==WALL_RIGHT){
    //         g_posList[i].right=false;
    //         g_posList[j].left=false;
    //         printf("hello\n");
    //     }
    //     roott = roott->firstchild;
    // }

    for(int i=0;i<count1-1;i++){
        int i_pos_id = g_hasVisitedPosList[i].pos_id;
        int j_pos_id = g_hasVisitedPosList[i+1].pos_id;
        WALL_TYPE wall_type = judge_wall_type(i_pos_id,j_pos_id);
        if(wall_type==WALL_UP){
            g_posList[i_pos_id].up=false;
            g_posList[j_pos_id].down=false;
        }else if(wall_type==WALL_DOWN){
            g_posList[i_pos_id].down=false;
            g_posList[j_pos_id].up=false;
        }else if(wall_type==WALL_LEFT){
            g_posList[i_pos_id].left=false;
            g_posList[j_pos_id].right=false;
        }else if(wall_type==WALL_RIGHT){
            g_posList[i_pos_id].right=false;
            g_posList[j_pos_id].left=false;
        }
    }

}


// 定时：间隔ms毫秒触发一次，返回true代表时间到
bool timer(DWORD interval)
{
    static DWORD last = 0;
    DWORD now = GetTickCount();
    if (now - last >= interval)
    {
        last = now;
        return true;
    }
    return false;
}
struct Player_Pos{
    int x;
    int y;
};
int main()
{
    srand((unsigned)time(NULL));   /* 用当前时间初始化随机种子 */

    for (int k=0;k<ROW_COUNTS*COLUMN_COUNTS;k++)
    {
        for (int t=0; t<ROW_COUNTS*COLUMN_COUNTS;t++)
        {
            int i = (int)(t/COLUMN_COUNTS);
            int j = t%COLUMN_COUNTS;
            if(k==((i-1)*COLUMN_COUNTS+j)||(k==((i+1)*COLUMN_COUNTS+j))||((k==((i)*COLUMN_COUNTS+j-1))&&j!=0)||(k==((i)*COLUMN_COUNTS+j+1)&&j!=(COLUMN_COUNTS-1)))
            {
                g_isAdjoinList[t][k]=1;
                g_isAdjoinList[k][t]=1;
            }else{
                g_isAdjoinList[t][k]=0;
            }
                
            
            
        }
        
    }

    // printf("%d ", g_isAdjoinList[2][3]);
    // printf("%d ", g_isAdjoinList[3][4]);
    // printf("%d ", g_isAdjoinList[2][4]);
    build_forest();
    

    initgraph(1000,1000);
    
    setbkcolor(RGB(228, 228, 236));
    cleardevice();
    setlinecolor(BLACK);
    //画背景围墙，可以只绘一次
    for(int i=0;i<ROW_COUNTS*COLUMN_COUNTS;i++){
        int x = 20 + g_posList[i].x*CELL_WIDTH;
        int y = 20 + g_posList[i].y*CELL_WIDTH;
        if(g_posList[i].up){
            draw_up(x,y);
        }
        if(g_posList[i].down){
            int x = 20 + g_posList[i].x*CELL_WIDTH;
            int y = 20 + (g_posList[i].y+1)*CELL_WIDTH;
            draw_down(x,y);
        }
        if(g_posList[i].left&&i!=0){
            draw_left(x,y);
        }
        if(g_posList[i].right&&i!=ROW_COUNTS*COLUMN_COUNTS-1){
            int x = 20 + (g_posList[i].x+1)*CELL_WIDTH;
            int y = 20 + g_posList[i].y*CELL_WIDTH;
            draw_right(x,y);
        }
    }

    // 在屏幕中央输出字符串
	RECT r1 = {-10, 30, 20, 60};
    settextcolor(BLUE);
	drawtext(LPCTSTR("start"), &r1, DT_RIGHT | DT_SINGLELINE);

    RECT r2 = {CELL_WIDTH*COLUMN_COUNTS, CELL_WIDTH*ROW_COUNTS-100, CELL_WIDTH*COLUMN_COUNTS+30, CELL_WIDTH*ROW_COUNTS-100+30};
    settextcolor(RED);
	drawtext(LPCTSTR("end"), &r2, DT_LEFT | DT_SINGLELINE);

    // setlinecolor(BLACK);
    // for(int i=0;i<=ROW_COUNTS;i++){
    //     line(10,10+i*CELL_WIDTH,10+ROW_COUNTS*CELL_WIDTH,10+i*CELL_WIDTH);
        
    // }
    // for(int j=0;j<=COLUMN_COUNTS;j++){
    //     line(10+j*CELL_WIDTH,10,10+j*CELL_WIDTH,10+COLUMN_COUNTS*CELL_WIDTH);
    // }
    struct Player_Pos player_pos={.x=0,.y=0};
    BeginBatchDraw();
    int centre_x = 20 + CELL_WIDTH/2;  // 中心点X坐标
    int centre_y = 20 + CELL_WIDTH/2;  // 中心点Y坐标
    int x = centre_x - PLAYER_RECT_WIDTH/2;  // 初始矩形左上角X坐标
    int y = centre_y - PLAYER_RECT_WIDTH/2;  // 初始矩形左上角

    // 保存上一次按键状态
    bool up_last = false, down_last = false, left_last = false, right_last = false;
    while(1)
    {
        // 检测有没有按键
        // if(timer(34)){
        //     if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        //         break;

        //     if (GetAsyncKeyState(VK_UP) & 0x8000)    {clearrectangle(x,y,x+100,y+100);y -= 160;}
        //     if (GetAsyncKeyState(VK_DOWN) & 0x8000)  {clearrectangle(x,y,x+100,y+100);y += 160;}
        //     if (GetAsyncKeyState(VK_LEFT) & 0x8000)  {clearrectangle(x,y,x+100,y+100);x -= 160;}
        //     if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {clearrectangle(x,y,x+100,y+100);x += 160;}
        // }

        // 当前按键状态
        bool up_now = GetAsyncKeyState(VK_UP) & 0x8000;
        bool down_now = GetAsyncKeyState(VK_DOWN) & 0x8000;
        bool left_now = GetAsyncKeyState(VK_LEFT) & 0x8000;
        bool right_now = GetAsyncKeyState(VK_RIGHT) & 0x8000;

        // 只有timer到点，并且是【刚按下】才移动一次
        // if (timer(40))
        // {
            if (up_now && !up_last)
            {
                if(player_pos.y>0&&!g_posList[player_pos.y*COLUMN_COUNTS+player_pos.x].up){
                    clearrectangle(x, y, x + PLAYER_RECT_WIDTH, y + PLAYER_RECT_WIDTH);
                    y -= CELL_WIDTH;
                    player_pos.y -= 1;
                }
                // clearrectangle(x, y, x + PLAYER_RECT_WIDTH, y + PLAYER_RECT_WIDTH);
                // y -= CELL_WIDTH;
                // player_pos.y -= 1;
            }
            if (down_now && !down_last)
            {
                if(player_pos.y<ROW_COUNTS-1&&!g_posList[player_pos.y*COLUMN_COUNTS+player_pos.x].down){
                    clearrectangle(x, y, x + PLAYER_RECT_WIDTH, y + PLAYER_RECT_WIDTH);
                    y += CELL_WIDTH;
                    player_pos.y += 1;
                }
                // clearrectangle(x, y, x + PLAYER_RECT_WIDTH, y + PLAYER_RECT_WIDTH);
                // y += CELL_WIDTH;
                // player_pos.y += 1;
            }
            if (left_now && !left_last)
            {
                if(player_pos.x>0&&!g_posList[player_pos.y*COLUMN_COUNTS+player_pos.x].left){
                    clearrectangle(x, y, x + PLAYER_RECT_WIDTH, y + PLAYER_RECT_WIDTH);
                    x -= CELL_WIDTH;
                    player_pos.x -= 1;
                }
                // clearrectangle(x, y, x + PLAYER_RECT_WIDTH, y + PLAYER_RECT_WIDTH);
                // x -= CELL_WIDTH;
                // player_pos.x -= 1;
            }
            if (right_now && !right_last)
            {
                if(player_pos.x<COLUMN_COUNTS-1&&!g_posList[player_pos.y*COLUMN_COUNTS+player_pos.x].right){
                    clearrectangle(x, y, x + PLAYER_RECT_WIDTH, y + PLAYER_RECT_WIDTH);
                    x += CELL_WIDTH;
                    player_pos.x += 1;
                }
                // clearrectangle(x, y, x + PLAYER_RECT_WIDTH, y + PLAYER_RECT_WIDTH);
                // x += CELL_WIDTH;
                // player_pos.x += 1;
            }
        // }

        // 更新上一帧状态（每一循环都更新！非常关键，不能放进if(timer)里）
        up_last = up_now;
        down_last = down_now;
        left_last = left_now;
        right_last = right_now;
        if(timer(40)){//定义FPS为25帧每秒，40毫秒刷新一次
            // cleardevice();
            // printf("x=%d,y=%d\n",x,y);
            setfillcolor(BLUE);
            solidrectangle(x,y,x+PLAYER_RECT_WIDTH,y+PLAYER_RECT_WIDTH);
            
            // circle(x,240,30);
            // 每50ms执行一次移动（定时）
            // if (timer(50))
            // {
            //     if(timer(25))
            //     {

            //     }
            //     x += 2;
            //     if(x>640) x=0;
            // }
            
            
            FlushBatchDraw();
        }
        
    }
    EndBatchDraw();
    _getch();
    closegraph();
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>   //需要使用time()函数
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <deque>
#include <vector>
#include <set>
#include <random>
#include <ctime>
#define WIDTH 40    //游戏界面宽度
#define HEIGHT 20   //游戏界面高度

//定义符号常量，表示各种不同的状态
#define BLANK '_'   //空白
#define WALL '*'    //墙壁
#define SNAKEHEAD '@'  //蛇头
#define SNAKEBODY 'o'  //蛇身
#define FOOD '$'    //食物
using namespace std;
//定义坐标结构体
struct Position
{
    int x;    //横坐标
    int y;    //纵坐标
    // 默认构造函数，使用初始化列表来初始化成员变量
    Position() : x(0), y(0) {}
    Position(int x_val, int y_val) : x(x_val), y(y_val) {}
};
struct Env
{
    int board[WIDTH][HEIGHT];
    Position food;
    // 默认构造函数，使用初始化列表来初始化成员变量
    Env(){init();}
//    Position(int x_val, int y_val) : x(x_val), y(y_val) {}
    void init(){
        for (int i = 0; i < WIDTH; i++)
        {
            for (int j = 0; j < HEIGHT; j++)
            {
                if (i == 0 || j == 0 || i == WIDTH - 1 || j == HEIGHT - 1) //设置墙壁
                    board[i][j] = WALL;
                else
                    board[i][j] = BLANK;    //其他为空白
            }
        }
    }
    void Draw(){
        system("clear");   //清屏，避免前一帧的内容残留
        for (int i = 0; i < HEIGHT; i++)
        {
            for (int j = 0; j < WIDTH; j++)
            {
                printf("%c", board[j][i]);  //输出一个字符
            }
            printf("\n");    //每行输出完后，换行
        }
        printf("\033[33m注意,请使用小写wasd输入\033[0m\n");   //输出得分
    }
};
//定义枚举类型，表示游戏状态
enum GameState
{
    Over = -1,   //游戏结束
    Running = 0, //游戏进行中
    Win = 1      //游戏胜利
};
class Snake{
    private:
        char ch;
        Env& env;
    public:
    int user_id = 1;
    int score = 0;   //得分
    struct Position head;   //蛇头位置
    struct Position body[WIDTH * HEIGHT];  //蛇身位置
//    struct Position food;   //食物位置
    struct Position oldtail;
//    char board[WIDTH][HEIGHT];   //游戏界面
    int length = 3;     //蛇身长度，初始为3
    int direction = 0;  //蛇的行进方向，0表示向右，1表示向下，2表示向左，3表示向上
    deque<Position> snakebody;      //蛇身位置2
//    Snake(){
//
//    }
    Snake(Env& e) : env(e) {  // 使用初始化列表
        // 这里不需要再赋值，因为初始化列表已经做了
    }
    Snake(Env& e,int uid) : env(e),user_id(uid) {  // 使用初始化列表
        // 这里不需要再赋值，因为初始化列表已经做了
    }
    //函数声明
    void setch(char c){
        switch (ch)   //根据输入方向屏蔽反方向
        {
            case 'd':
                ch = (c == 'a')?ch:c;
                break;
            case 's':  //向下
                ch = (c == 'w')?ch:c;
                break;
            case 'a':  //向左
                ch = (c == 'd')?ch:c;
                break;
            case 'w':  //向上
                ch = (c == 's')?ch:c;
                break;
            default:
                ch = c;
        }

    }
    void UpdateGame(){
        Move();
        // amend map
        if(oldtail.x!=0)
        env.board[oldtail.x][oldtail.y] = BLANK;
        // check
        enum GameState state = CheckGameOver();  //检查游戏是否结束
        if (state != Running)  //如果游戏结束，则进行相应操作
        {
            GameOver();
        }
        // update snake
        env.board[head.x][head.y] = SNAKEHEAD;
        for(auto a:snakebody){
            env.board[a.x][a.y] = SNAKEBODY;
        }
    }

    //游戏暂停
    static void Pause()
    {
        usleep(200000); // 暂停 500 毫秒
    }
    //初始化游戏界面和蛇的初始位置
    void InitGame()
    {
        // implemented in constructor
//        for (int i = 0; i < WIDTH; i++)
//        {
//            for (int j = 0; j < HEIGHT; j++)
//            {
//                if (i == 0 || j == 0 || i == WIDTH - 1 || j == HEIGHT - 1) //设置墙壁
//                    board[i][j] = WALL;
//                else
//                    board[i][j] = BLANK;    //其他为空白
//            }
//        }
        //初始化蛇头
//        head.x = WIDTH / 2;
        head.x = (user_id)==1?(WIDTH / 4):(WIDTH / 4 * 3);
//        head.y = HEIGHT / 2;
        head.y = (user_id)==1?(HEIGHT / 4):(HEIGHT / 4 * 3);
        // add two body block
        snakebody.push_back(Position(head.x - 1,head.y));
        snakebody.push_back(Position(head.x - 2,head.y));
        //add food
//        GenerateFood();
    }
    //绘制游戏画面
    void DrawGame()
    {
        system("clear");   //清屏，避免前一帧的内容残留
        for (int i = 0; i < HEIGHT; i++)
        {
            for (int j = 0; j < WIDTH; j++)
            {
                printf("%c", env.board[j][i]);  //输出一个字符
            }
            printf("\n");    //每行输出完后，换行
        }
        //printf("input: %c\n", ch);
        printf("\033[32m注意,请使用小写wasd输入\033[0m\n");   //输出得分
        //printf("Score: %d\n", score);   //输出得分
    }
    void Move(){
        //更新蛇头的位置
        Position oldPos(head.x,head.y);
        switch (ch)   //根据输入方向进行移动
        {
            case 'd':  //向右
                head.x++;
            break;
            case 's':  //向下
                head.y++;
            break;
            case 'a':  //向左
                head.x--;
            break;
            case 'w':  //向上
                head.y--;
            break;
        }
        if(ch=='w'||ch=='a'||ch=='s'||ch=='d'){
            snakebody.push_front(oldPos);
            oldtail = snakebody.back();
            snakebody.pop_back();
        }
        //吃到食物后，更新分数和蛇的长度，并生成新的食物
        if (head.x == env.food.x && head.y == env.food.y)
        {
            score += 10;
            length++;
            GenerateFood();
            snakebody.push_back(oldtail);
        }
    }
    //检查游戏是否结束
    enum GameState CheckGameOver()
    {
        //蛇头碰到墙壁，游戏结束
        if (env.board[head.x][head.y] == WALL)
            return Over;

        //蛇头碰到蛇身，游戏结束
        for(auto i : snakebody){
          if (head.x == i.x && head.y == i.y)
                return Over;
        }

        //游戏胜利
        if (length == (WIDTH - 2) * (HEIGHT - 2) / 3)
            return Win;

        return Running;   //游戏继续进行
    }
    //游戏结束
    void GameOver()
    {
        printf("Game over!\n");
        printf("Your score: %d\n", score);
        exit(0);  //直接退出程序
    }
    //生成随机食物
    void GenerateFood()
    {
        int x, y;
        do
        {
            x = rand() % (WIDTH - 2) + 1;    //随机x坐标，排除在边框上的墙壁位置
            y = rand() % (HEIGHT - 2) + 1;   //随机y坐标，排除在边框上的墙壁位置
        } while (env.board[x][y] != BLANK);   //如果随机到的位置不为空白，则重新随机
        env.food.x = x;
        env.food.y = y;
        env.board[x][y] = FOOD;  //在随机位置生成食物
    }

};
//char board[WIDTH][HEIGHT];   //游戏界面
//thread
pthread_t input_thread;
char ch_temp;
void StartNewThreadForInputDetection();
void getch();
void* ContinuousGetch(void* v);

int Runtest()
{
    Env env;
    Snake player1 = Snake(env);
    srand(time(NULL));   //用当前时间作为随机数种子，使每次运行的随机食物位置不同
    player1.InitGame();   //初始化游戏
    env.Draw();   //绘制游戏画面
    StartNewThreadForInputDetection();
    while (1)
    {
        player1.setch(ch_temp);
        player1.UpdateGame();  //更新游戏
//        player1.DrawGame();
        env.Draw();   //绘制游戏画面
        player1.Pause();       //游戏暂停
    }
    return 0;
}

void getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    if(buf=='w'||buf=='a'||buf=='s'||buf=='d')
        ch_temp = buf;
    return;
}
void StartNewThreadForInputDetection(){
    pthread_create(&input_thread,NULL,ContinuousGetch,NULL);
}
void* ContinuousGetch(void* v){
    while(true){
        getch();
    }
}
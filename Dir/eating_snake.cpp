#include "snake_class.h"
#include <cstring>
#include <netinet/in.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

//定义符号常量，表示各种不同的状态
#define BUFFER_SIZE 64
using namespace std;
// net para declaration
pollfd fds[2];
char read_buf[BUFFER_SIZE];
char* send_buf = new char[BUFFER_SIZE];
int pipefd[2];
int ret;
int sockfd;
int isplayer1;
int myturn;
int user_counter = 0;
int user_id = 1;
int seed = 7355608;
//function declaration
void ConnectToServer();
void sendch(char c);
char GetSuffix(string prefix);
//main
int main(){
    ConnectToServer();
    Env env;
    int val2;
    val2 = (user_id==1)?2:1;
    Snake player1 = Snake(env,user_id);
    Snake player2 = Snake(env,val2);
    srand(time(NULL));   //用当前时间作为随机数种子，使每次运行的随机食物位置不同
    srand(seed);
    player1.InitGame();   //初始化游戏
    player2.InitGame();   //初始化游戏
    player1.GenerateFood();
    env.Draw();   //绘制游戏画面
    StartNewThreadForInputDetection();
    while (1)
    {
        player1.setch(ch_temp);
        //send message and wait for opponent
        sendch(ch_temp);
        memset( read_buf, '\0', BUFFER_SIZE );
        ret = recv( fds[1].fd, read_buf, BUFFER_SIZE-1, 0 );
        assert( ret != -1 );
        player2.setch(GetSuffix("input:"));
        //update game
        player1.UpdateGame();  //更新游戏
        player2.UpdateGame();  //更新游戏
//      player1.DrawGame();
        env.Draw();   //绘制游戏画面
        Snake::Pause();       //游戏暂停
    }
    return 0;
}
void sendch(char c){
    memset( send_buf, '\0', BUFFER_SIZE );
    sprintf(send_buf,"input:%c",c);
    ret = send( sockfd, send_buf, strlen( send_buf ), 0 );
    assert(ret!=-1);
}
void ConnectToServer(){
    // net connection
    const char* ip = "127.0.0.1";
    int port = 8888;

    struct sockaddr_in server_address;
    bzero( &server_address, sizeof( server_address ) );
    server_address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &server_address.sin_addr );
    server_address.sin_port = htons( port );

    sockfd = socket( PF_INET, SOCK_STREAM, 0 );
    assert( sockfd >= 0 );
    if ( connect( sockfd, ( struct sockaddr* )&server_address, sizeof( server_address ) ) < 0 )
    {
        printf( "connection failed\n" );
        close( sockfd );
        exit(0);
    }
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN | POLLRDHUP;
    fds[1].revents = 0;
    ret = pipe( pipefd );
    assert( ret != -1 );

    // confrim self as nth player
    cout << "waiting for server confrimation..........." << endl;
    memset( read_buf, '\0', BUFFER_SIZE );
    ret = recv( fds[1].fd, read_buf, BUFFER_SIZE-1, 0 );
    assert( ret != -1 );
    printf( "you are %s player\n", read_buf );
    user_id = (strcmp(read_buf,"first")==0)?1:2;
    sleep(2);
    system("clear");
    cout << "Welcome to Snake." << endl;
    cout << "Waiting for opponent." << endl;
    while(1){
        memset( send_buf, '\0', BUFFER_SIZE );
        sprintf(send_buf,"%s","GETUSERNUM\n");
        ret = send( sockfd, send_buf, strlen( send_buf ), 0 );
        assert(ret!=-1);
        memset( read_buf, '\0', BUFFER_SIZE );
        ret = recv( fds[1].fd, read_buf, BUFFER_SIZE-1, 0 );
        assert( ret != -1 );

        user_counter = 0;
        // 使用 sscanf 从字符串中提取整数
        if (sscanf(read_buf, "user:%d", &user_counter) == 1) {
            std::cout << "The user value is: " << user_counter << std::endl;
            if(user_counter==2){
                std::cout<<"Opponent ready"<<std::endl;
                sleep(1);
                break;
            }
        } else {
            std::cout << "Failed to parse user value." << std::endl;
        }
        //printf( "current play num: %s \n", read_buf );
        sleep(3);
//        break;
    }
}
char GetSuffix(string prefix){
    char c_tmp;
    string s_prefix = prefix+"%c";
    //使用 sscanf 从字符串中提取整数
    if (sscanf(read_buf, s_prefix.c_str(), &c_tmp) == 1) {
        std::cout << "The opponent input is: " << c_tmp << std::endl;
    } else {
        std::cout << "Failed to parse user value." << std::endl;
    }
    return c_tmp;
}
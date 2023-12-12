//main.cpp파일
#include "GameServer.h"

int main() {

    GameServer gameServer(5000);
    gameServer.start();

    system("pause");
    // 서버 소멸
    gameServer.~GameServer();

    return 0;

}
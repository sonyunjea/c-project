#include "GameServer.h"

int main() {

    GameServer gameServer(5000);
    gameServer.start();

    system("pause");
    // ���� �Ҹ�
    gameServer.~GameServer();

    return 0;

}
#include "GameServer.h"

int main() {

    GameServer gameServer(5000);
    gameServer.start();

    system("pause");
    // ¼­¹ö ¼Ò¸ê
    gameServer.~GameServer();

    return 0;

}
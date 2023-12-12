//main.cppÆÄÀÏ

#include "GameClient.h"

int main() {

    GameClient gameClient("127.0.0.1", 5000);
    gameClient.start();
    system("pause");
    gameClient.~GameClient();

    return 0;
}
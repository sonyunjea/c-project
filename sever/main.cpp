#include "GameServer.h" // GameServer Ŭ������ ��� ������ �����մϴ�. �� Ŭ������ ���� ������ ����� �����մϴ�.

int main() { // ���� �Լ� - ���α׷��� �������Դϴ�.

    GameServer gameServer(5000); // ��Ʈ ��ȣ 5000�� ����Ͽ� GameServer ��ü�� �����մϴ�.
    gameServer.start(); // gameServer ��ü�� start �޼ҵ带 ȣ���Ͽ� ������ �����մϴ�.

    // ���� �Ҹ�
    gameServer.~GameServer(); // gameServer ��ü�� �Ҹ��ڸ� ��������� ȣ���մϴ�. 

    return 0; 

}

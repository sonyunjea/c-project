#include "GameServer.h" // GameServer 클래스의 헤더 파일을 포함합니다. 이 클래스는 게임 서버의 기능을 정의합니다.

int main() { // 메인 함수 - 프로그램의 시작점입니다.

    GameServer gameServer(5000); // 포트 번호 5000을 사용하여 GameServer 객체를 생성합니다.
    gameServer.start(); // gameServer 객체의 start 메소드를 호출하여 서버를 시작합니다.

    // 서버 소멸
    gameServer.~GameServer(); // gameServer 객체의 소멸자를 명시적으로 호출합니다. 

    return 0; 

}

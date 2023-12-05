#pragma once
#include <iostream>
#include <vector>
#include <Windows.h>
#include "GamePlayer.h"

void clearCursor() {
    CONSOLE_CURSOR_INFO cursorInfo = { 0, };
    cursorInfo.dwSize = 1; //커서 굵기 (1 ~ 100)
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

typedef enum {
    EMPTY,
    WALL,

    OCCUPATING_P1,
    OCCUPATING_P2,

    PLACE_P1,
    PLACE_P2,

    ENTER_P1,
    ENTER_P2,

    EXIT_P1,
    EXIT_P2
}MAPSTATE;

typedef enum {
    AREACAPTURE,
    READY,
    GUNFIGHT
}MODE;

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

class GameInfo {
private:
    static const int ROWS = 12;
    static const int COLS = 12;
    std::vector<std::vector<int>> map;

    // [플레이어번호][구역순서]
    std::vector<std::vector<std::vector<std::vector<int>>>> occupatedArea;

    // 입구 및 출구 [플레이어번호][구역순서]
    std::vector<std::vector<std::vector<int>>> enters;
    std::vector<std::vector<std::vector<int>>> exits;

    std::vector<GamePlayer> players;

    // 게임 모드
    int mode;

    // 프레임 수
    int fps;

public:

    // 게임 초기화
    GameInfo() {

        srand(time(NULL));

        map = std::vector<std::vector<int>>(ROWS, std::vector<int>(COLS, 0));

        // 맵 초기화
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                if (i == 0 || i == ROWS - 1 || j == 0 || j == COLS - 1) {
                    map[i][j] = MAPSTATE::WALL;
                }
            }
        }

        // 플레이어 좌표 초기화
        for (int i = 0; i < 2; i++) {

            int newX = rand() % 10 + 1;
            int newY = rand() % 10 + 1;

            bool flag = false;
            for (int j = 0; j < i; j++) {
                if (newX == players[j].getX() || newY == players[j].getY()) {
                    flag = true;
                    break;
                }
            }
            if (flag) {
                i--;
                continue;
            }

            occupatedArea.push_back({});
            players.push_back(GamePlayer(newX, newY));
            enters.push_back({});
            exits.push_back({});
        }

        // 모드 설정
        mode = MODE::AREACAPTURE;

        // 프레임 설정
        fps = 60;

    }
    ~GameInfo() {

        for (auto& p : players) p.~GamePlayer();

    }

    // 현재 맵 정보를 출력
    void printGame() const {

        clearCursor();
        gotoxy(0, 0);

        int i = 0, j = 0;
        for (const auto& row : map) {
            j = 0;
            for (int cell : row) {
                char ch = ' ';
                switch (cell) {
                case MAPSTATE::WALL: ch = '*'; break;

                case MAPSTATE::PLACE_P1: ch = 'a'; break;
                case MAPSTATE::PLACE_P2: ch = 'b'; break;

                default: break;
                }

                // 입구
                for (auto& enter : enters) {
                    for (auto& e : enter) {
                        if (i == e[0] && j == e[1]) {
                            ch = 'i';
                        }
                    }
                }

                // 출구
                for (auto& exit : exits) {
                    for (auto& e : exit) {
                        if (i == e[0] && j == e[1]) {
                            ch = 'o';
                        }
                    }
                }

                // 플레이어
                int index = 0;
                for (GamePlayer player : players) {
                    if (i == player.getY() && j == player.getX()) {
                        ch = 'A' + index;
                        break;
                    }
                    index++;
                }

                std::cout << ch;
                j++;
            }
            i++;
            std::cout << '\n';
        }
    }

    // 플레이어 이동
    void playerMove(int index, int d) {

        players[index].movePlayer(map, d, (index == 0) ? MAPSTATE::PLACE_P1 : MAPSTATE::PLACE_P2);

    }

    // 이동 외의 키 액션
    void playerAction(int index, int key) {

        // a키 : 총격전으로 준비
        if (key == 'a' || key == 'A') {

            if (players[index].getState() == STATE::NORMAL && isAbleGunFightMode()) {
                players[index].setState(STATE::GUN);
            }

        }

        // x키 : 점프
        if (key == 'x' || key == 'X') {

            int x = players[index].getX();
            int y = players[index].getY();

            int i = 0;

            // 구역이 하나면 안함
            if (exits[index].size() <= 1) {
                return;
            }

            for (auto& exit : exits[index]) {
                // 출구 위치에서 점프 키 누르면 다음 입구로 건너 뜀
                if (exit[0] == y && exit[1] == x) {
                    gotoxy(30, 0);
                    std::cout << "짬푸";
                    players[index].setPos(enters[index][(i + 1) % enters.size()][1], enters[index][(i + 1) % enters.size()][0]);
                    break;
                }
                i++;
            }

        }

        // s키 : 입출구 재설정
        if (key == 's' || key == 'S') {

            if (mode != MODE::AREACAPTURE) { return; }

            // 입출구 비워주기
            enters[index].clear();
            exits[index].clear();

            for (auto& op : occupatedArea[index]) {

                // 점령지역 내에서 입구 및 출구 생성
                // 길이 1일때 입구 출구 동일
                if (op.size() == 1) {
                    enters[index].push_back(op[0]);
                    exits[index].push_back(op[0]);
                }
                // 길이 1 초과일때 입구 출구 랜덤 생성
                else {

                    srand(time(NULL));

                    int enterIndex = 0;
                    int exitIndex = 0;

                    while (enterIndex == exitIndex) {
                        enterIndex = rand() % op.size();
                        exitIndex = rand() % op.size();
                    }

                    enters[index].push_back(op[enterIndex]);
                    exits[index].push_back(op[exitIndex]);

                }
            }

        }

        // 스페이스 바
        if (key == 32) {

            // 일반 상태
            if (players[index].getState() == STATE::NORMAL && mode != MODE::READY) {
                // 영역 지정
                players[index].setState(STATE::OCCUPATE);
            }

            // 점령 상태
            else if (players[index].getState() == STATE::OCCUPATE && mode != MODE::READY) {

                // 점령 (점령이 가능한지 확인 후)
                if (players[index].isAbleOccupation(index, map, (index == 0) ? MAPSTATE::PLACE_P2 : MAPSTATE::PLACE_P1)) {
                    occupatedArea[index].push_back(players[index].getOccuPos());

                    std::vector<std::vector<int>> op = players[index].getOccuPos();

                    // 점령지역 내에서 입구 및 출구 생성
                    // 길이 1일때 입구 출구 동일
                    if (op.size() == 1) {
                        enters[index].push_back(op[0]);
                        exits[index].push_back(op[0]);
                    }
                    // 길이 1 초과일때 입구 출구 랜덤 생성
                    else {

                        srand(time(NULL));

                        int enterIndex = 0;
                        int exitIndex = 0;

                        while (enterIndex == exitIndex) {
                            enterIndex = rand() % op.size();
                            exitIndex = rand() % op.size();
                        }

                        enters[index].push_back(op[enterIndex]);
                        exits[index].push_back(op[exitIndex]);

                    }

                    // 맵에 정보 생성
                    for (auto& x : op) {
                        map[x[0]][x[1]] = (index == 0) ? MAPSTATE::PLACE_P1 : MAPSTATE::PLACE_P2;
                    }

                }
                players[index].clearOccuPos();
                players[index].setState(STATE::NORMAL);
            }

            // 총격전 상태 
            else if (players[index].getState() == STATE::GUN && mode == MODE::GUNFIGHT) {
                // 총을 쏜다

            }

        }

    }

    // 총격전 전환 조건
    bool isAbleGunFightMode() {

        int count = 0;

        for (int i = 1; i < ROWS - 1; i++) {
            for (int j = 1; j < COLS - 1; j++) {
                switch (map[i][j]) {
                case MAPSTATE::PLACE_P1:
                case MAPSTATE::PLACE_P2:
                case MAPSTATE::ENTER_P1:
                case MAPSTATE::ENTER_P2:
                case MAPSTATE::EXIT_P1:
                case MAPSTATE::EXIT_P2: count++; break;
                }
            }
        }

        return count >= 50;

    }

    // 총격전 전환
    void checkGunFightMode() {

        int count = 0;
        for (auto& player : players) {
            if (player.getState() == STATE::GUN) count++;
        }
        if (count > 0) {
            mode = MODE::READY;

            for (auto& player : players) {
                player.clearOccuPos();

                if (player.getState() != STATE::GUN) player.setState(STATE::NORMAL);
            }
        }
        if (count == 2) {
            mode = MODE::GUNFIGHT;
        }

    }

    int getFps() { return fps; }
    int getMode() { return mode; }
    std::vector<std::vector<int>> getMap() { return map; }
    std::vector<GamePlayer> getPlayers() { return players; }

};
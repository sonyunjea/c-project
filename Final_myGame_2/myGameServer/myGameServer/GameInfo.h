//GameInfo.h����
#pragma once
#include <iostream>
#include <vector>
#include <Windows.h>
#include "GamePlayer.h"

void clearCursor() {
    CONSOLE_CURSOR_INFO cursorInfo = { 0, };
    cursorInfo.dwSize = 1; //Ŀ�� ���� (1 ~ 100)
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
    GUNFIGHT,
    END
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

    // [�÷��̾��ȣ][��������]
    std::vector<std::vector<std::vector<std::vector<int>>>> occupatedArea;
   
    // �Ա� �� �ⱸ [�÷��̾��ȣ][��������]
    std::vector<std::vector<std::vector<int>>> enters;
    std::vector<std::vector<std::vector<int>>> exits;

    std::vector<GamePlayer> players;

    // ���� ���
    int mode;

    // ������ ��
    int fps;

public:

    // ���� �ʱ�ȭ
    GameInfo() {
        
        srand(time(NULL));
        
        map = std::vector<std::vector<int>>(ROWS, std::vector<int>(COLS, 0));
        
        // �� �ʱ�ȭ
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                if (i == 0 || i == ROWS - 1 || j == 0 || j == COLS - 1) {
                    map[i][j] = MAPSTATE::WALL;
                }
            }
        }

        // �÷��̾� ��ǥ �ʱ�ȭ
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

        // ��� ����
        mode = MODE::AREACAPTURE;
        
        // ������ ����
        fps = 60;

    }
    ~GameInfo() {

        for (auto& p : players) p.~GamePlayer();
        
    }

    // ���� �� ������ ���
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

                // �Ա�
                for (auto& enter : enters) {
                    for (auto& e : enter) {
                        if (i == e[0] && j == e[1]) {
                            ch = 'i';
                        }
                    }
                }

                // �ⱸ
                for (auto& exit : exits) {
                    for (auto& e : exit) {
                        if (i == e[0] && j == e[1]) {
                            ch = 'o';
                        }
                    }
                }

                // �÷��̾�
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

    // �÷��̾� �̵�
    void playerMove(int index, int d) {

        players[index].movePlayer(map, d, (index == 0) ? MAPSTATE::PLACE_P1 : MAPSTATE::PLACE_P2);

    }

    // �̵� ���� Ű �׼�
    void playerAction(int index, int key) {

        // aŰ : �Ѱ������� �غ�
        if (key == 'a' || key == 'A') {
                
            if (players[index].getState() == STATE::NORMAL && isAbleGunFightMode()) {
                players[index].setState(STATE::GUN);
            }

        }

        // xŰ : ����
        if (key == 'x' || key == 'X') {

            int x = players[index].getX();
            int y = players[index].getY();

            int i = 0;
            
            // ������ �ϳ��� ����
            if (exits[index].size() <= 1) {
                return;
            }

            for (auto& exit : exits[index]) {
                // �ⱸ ��ġ���� ���� Ű ������ ���� �Ա��� �ǳ� ��
                if (exit[0] == y && exit[1] == x) {
                    players[index].setPos(enters[index][(i + 1) % enters[index].size()][1], enters[index][(i + 1) % enters[index].size()][0]);
                    break;
                }
                i++;
            }

        }

        // sŰ : ���ⱸ �缳��
        if (key == 's' || key == 'S') {

            if (mode != MODE::AREACAPTURE) { return; }

            // ���ⱸ ����ֱ�
            enters[index].clear();
            exits[index].clear();

            for (auto& op : occupatedArea[index]) {

                // �������� ������ �Ա� �� �ⱸ ����
                // ���� 1�϶� �Ա� �ⱸ ����
                if (op.size() == 1) {
                    enters[index].push_back(op[0]);
                    exits[index].push_back(op[0]);
                }
                // ���� 1 �ʰ��϶� �Ա� �ⱸ ���� ����
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

        // �����̽� ��
        if (key == 32) {

                // �Ϲ� ����
                if (players[index].getState() == STATE::NORMAL && mode != MODE::READY) {
                    // ���� ����
                    players[index].setState(STATE::OCCUPATE);
                }
                
                // ���� ����
                else if (players[index].getState() == STATE::OCCUPATE && mode != MODE::READY) {

                    // ���� (������ �������� Ȯ�� ��)
                    if (players[index].isAbleOccupation(index, map, (index == 0)?MAPSTATE::PLACE_P2:MAPSTATE::PLACE_P1)) {
                        occupatedArea[index].push_back(players[index].getOccuPos());

                        std::vector<std::vector<int>> op = players[index].getOccuPos();

                        // �������� ������ �Ա� �� �ⱸ ����
                        // ���� 1�϶� �Ա� �ⱸ ����
                        if (op.size() == 1) {
                            enters[index].push_back(op[0]);
                            exits[index].push_back(op[0]);
                        }
                        // ���� 1 �ʰ��϶� �Ա� �ⱸ ���� ����
                        else {

                            srand(time(NULL));

                            int enterIndex = 0;
                            int exitIndex = 0;

                            while (op[enterIndex][0] == op[exitIndex][0] && op[enterIndex][1] == op[exitIndex][1]) {
                                enterIndex = rand() % op.size();
                                exitIndex = rand() % op.size();
                            }

                            enters[index].push_back(op[enterIndex]);
                            exits[index].push_back(op[exitIndex]);

                        }

                        // ���� ä��� ���� ���� �迭
                        // [y��ǥ][x������, ����]
                        int lenInfo[12][2];

                        for (int i = 0; i < 12; i++) {
                            lenInfo[i][0] = 12;
                            lenInfo[i][1] = 0;
                        }

                        // �ʿ� ���� ����
                        for (auto& x : op) {
                            lenInfo[x[0]][0] = min(lenInfo[x[0]][0], x[1]);
                            lenInfo[x[0]][1] = max(lenInfo[x[0]][1], x[1]);

                            //map[x[0]][x[1]] = (index == 0) ? MAPSTATE::PLACE_P1 : MAPSTATE::PLACE_P2;
                        }

                        for (int i = 0; i < 12; i++) {
                            for (int j = lenInfo[i][0]; j <= lenInfo[i][1]; j++) {
                                map[i][j] = (index == 0) ? MAPSTATE::PLACE_P1 : MAPSTATE::PLACE_P2;
                            }
                        }

                    }
                    players[index].clearOccuPos();
                    players[index].setState(STATE::NORMAL);
                }

                // �Ѱ��� ���� 
                else if (players[index].getState() == STATE::GUN && mode == MODE::GUNFIGHT) {

                    srand(time(NULL));

                    // ���� ���

                    // UP RIGHT DOWN LEFT
                    int d = players[index].getDir();

                    int dpos[4][2] = {
                        {0, -1},
                        {1, 0},
                        {0, 1},
                        {-1, 0}
                    };

                    int x = players[index].getX();
                    int y = players[index].getY();
                   
                    while (0 < x && x < ROWS && 0 < y && y < COLS) {
                        int dx = x + dpos[d][0];
                        int dy = y + dpos[d][1];

                        if (players[(index + 1) % 2].getX() == dx && players[(index + 1) % 2].getY() == dy) {
                            if (rand() % 2) {
                                std::cout << "�¾Ҵ�!";
                                players[(index + 1) % 2].setState(3);
                                mode = MODE::END;
                            }
                            break;
                        }

                        x = dx;
                        y = dy;
                           
                    }
                    
                }

            }
         
    }

    // �Ѱ��� ��ȯ ����
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

    // �Ѱ��� ��ȯ
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
    std::vector<std::vector<std::vector<int>>> getEnters() { return enters; }
    std::vector<std::vector<std::vector<int>>> getExits() { return exits; }

};
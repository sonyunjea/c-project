#pragma once
#include <iostream>
#include <time.h>
#include "GameInfo.h"

typedef enum {
    NORMAL,
    OCCUPATE,
    GUN,
    DEAD
}STATE;

typedef enum {
    UP,
    RIGHT,
    DOWN,
    LEFT
}DIRECTION;


class GamePlayer {
private:
    int dir;    // �÷��̾��� ���� �ٶ󺸰� �ִ� ����
    int x;  // �÷��̾��� x ��ǥ
    int y;  // �÷��̾��� y ��ǥ

    int state; // �÷��̾��� �� ����
    /*
    
    0 : �Ϲ� �̵�
    1 : �����Ա� �� �̵�
    2 : �Ѱ��� ��� (�غ�)

    */

    std::vector<std::vector<int>> occupatingPos;      // ������ �õ��ϰ� �ִ� ���� ��ġ��

public:
    // ������: GamePlayer ��ü�� �ʱ�ȭ�մϴ�.
    GamePlayer(int startX, int startY) : x(startX), y(startY), state(STATE::NORMAL), dir(DIRECTION::UP) {}
    ~GamePlayer() {}

    // �÷��̾� �̵�
    void movePlayer(std::vector<std::vector<int>> map, int d, int area) {

        int newX = x;
        int newY = y;
        int newDir = dir;

        switch (d) {
        case 72: 
            newY = max(1, y - 1);
            newDir = DIRECTION::UP;
            break;
        case 77: 
            newX = min(map[0].size() - 2, x + 1);
            newDir = DIRECTION::RIGHT;
            break;
        case 80: 
            newY = min(map.size() - 2, y + 1);
            newDir = DIRECTION::DOWN;
            break;
        case 75: 
            newX = max(1, x - 1);
            newDir = DIRECTION::LEFT;
            break;
        }

        // ���� �̵�
        if (state == STATE::OCCUPATE) {
            // ������ ���� ����
            occupatingPos.push_back({ newY, newX });

            x = newX;
            y = newY;
            dir = newDir;
        }

        // �Ϲ� �̵�
        else {
            // �Ϲ� �̵��� ���� �ڱ� ������ ���� �̵�
            if (map[newY][newX] == area) {
                x = newX;
                y = newY;
                dir = newDir;
            }
        }

    }

    // ���� �õ�
    bool isAbleOccupation(int playerIndex, std::vector<std::vector<int>> map, int other) {

        for (auto& myP : occupatingPos) {

            int place = other;

            if (map[myP[0]][myP[1]] == 4 || map[myP[0]][myP[1]] == 5) return false;

        }

        // ������ ������ ���� ������ false
        if (occupatingPos[0][1] != x || occupatingPos[0][0] != y) return false;

        return true;

    }

    void clearOccuPos() {
        occupatingPos.clear();
    }

    // ���� ��ȯ
    void setState(int s) {
        state = s;

        if (state == STATE::OCCUPATE) {
            occupatingPos.push_back({ y, x });
        }
    }

    // ��ġ ��ȯ
    void setPos(int newX, int newY) {
        x = newX;
        y = newY;
    }

    int getX() { return x; }
    int getY() { return y; }
    int getDir() { return dir; }
    int getState() { return state; }
  
    std::vector<std::vector<int>> getOccuPos() { return occupatingPos; }

};
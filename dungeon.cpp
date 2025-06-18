#include "dungeon.h"
#include <random>
#include <algorithm>
#include <climits>
#include <QDebug>

Dungeon::Dungeon(int rows, int cols)
    : rows(rows), cols(cols), playerPos(0, 0), currentHealth(100),
    initialHealth(100), gameState(GameState::PLAYING) {
    map.resize(rows, std::vector<int>(cols, 0));
    dp.resize(rows, std::vector<int>(cols, 0));
}

void Dungeon::setSize(int rows, int cols) {
    this->rows = rows;
    this->cols = cols;
    map.resize(rows, std::vector<int>(cols, 0));
    dp.resize(rows, std::vector<int>(cols, 0));
}

void Dungeon::generateMap() {
    std::random_device rd;
    std::mt19937 gen(rd());

    bool hasSolution = false;
    int attempts = 0;
    const int maxAttempts = 100;

    while (!hasSolution && attempts < maxAttempts) {
        // 根据地图大小调整随机数范围，增加游戏难度
        int mapSize = rows * cols;
        int minVal, maxVal;

        if (mapSize <= 25) {  // 5x5及以下
            minVal = -30;
            maxVal = 15;
        } else if (mapSize <= 100) {  // 10x10及以下
            minVal = -16;
            maxVal = 8;
        } else {  // 更大地图
            minVal = -10;
            maxVal = 5;
        }

        std::uniform_int_distribution<> dis(minVal, maxVal);

        // 生成随机地图
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                map[i][j] = dis(gen);
            }
        }

        // 检查是否有解（静默检查）
        initializeDp();
        solveDp();

        // 检查最小健康值是否在合理范围内
        int minHealth = dp[0][0];
        int reasonableMax = mapSize * 2;  // 根据地图大小设置合理上限

        if (minHealth > 0 && minHealth <= reasonableMax) {
            hasSolution = true;
        }

        attempts++;
    }

    // 如果多次尝试仍无合理解，生成一个简单的可解地图
    if (!hasSolution) {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                map[i][j] = (i + j) % 3 - 1;  // 生成-1,0,1的简单模式
            }
        }
    }
}

void Dungeon::initializeDp() {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            dp[i][j] = INT_MAX;
        }
    }
}

void Dungeon::solveDp() {
    // 初始化最后一个位置
    dp[rows-1][cols-1] = std::max(1, 1 - map[rows-1][cols-1]);

    // 填充最后一行
    for (int j = cols - 2; j >= 0; --j) {
        dp[rows-1][j] = std::max(1, dp[rows-1][j+1] - map[rows-1][j]);
    }

    // 填充最后一列
    for (int i = rows - 2; i >= 0; --i) {
        dp[i][cols-1] = std::max(1, dp[i+1][cols-1] - map[i][cols-1]);
    }

    // 填充剩余位置
    for (int i = rows - 2; i >= 0; --i) {
        for (int j = cols - 2; j >= 0; --j) {
            int minHealth = std::min(dp[i+1][j], dp[i][j+1]);
            dp[i][j] = std::max(1, minHealth - map[i][j]);
        }
    }
}

int Dungeon::calculateMinHealth() {
    initializeDp();
    solveDp();
    return dp[0][0];
}

std::vector<QPoint> Dungeon::getOptimalPath() {
    std::vector<QPoint> path;
    int i = 0, j = 0;

    while (i < rows && j < cols) {
        path.push_back(QPoint(j, i));

        if (i == rows - 1) {
            // 只能向右
            j++;
        } else if (j == cols - 1) {
            // 只能向下
            i++;
        } else {
            // 选择dp值更小的方向
            if (dp[i+1][j] <= dp[i][j+1]) {
                i++;
            } else {
                j++;
            }
        }
    }

    return path;
}

// 手动模式相关方法
void Dungeon::resetGame(int initialHealth) {
    this->initialHealth = initialHealth;
    this->currentHealth = initialHealth;
    this->playerPos = QPoint(0, 0);
    this->gameState = GameState::PLAYING;
    this->playerPath.clear();
    this->playerPath.push_back(QPoint(0, 0));

    // 应用起始位置的效果
    currentHealth += map[0][0];
    updateGameState();
}

bool Dungeon::canMove(int dx, int dy) const {
    int newX = playerPos.x() + dx;
    int newY = playerPos.y() + dy;

    // 检查边界
    if (newX < 0 || newX >= cols || newY < 0 || newY >= rows) {
        return false;
    }

    // 检查游戏状态
    if (gameState != GameState::PLAYING) {
        return false;
    }

    return true;
}

bool Dungeon::movePlayer(int dx, int dy) {
    if (!canMove(dx, dy)) {
        return false;
    }

    // 移动玩家
    playerPos.setX(playerPos.x() + dx);
    playerPos.setY(playerPos.y() + dy);

    // 记录路径
    playerPath.push_back(playerPos);

    // 应用房间效果
    currentHealth += map[playerPos.y()][playerPos.x()];

    // 更新游戏状态
    updateGameState();

    return true;
}

void Dungeon::updateGameState() {
    if (currentHealth <= 0) {
        gameState = GameState::LOST;
    } else if (playerPos.x() == cols - 1 && playerPos.y() == rows - 1) {
        gameState = GameState::WON;
    } else {
        gameState = GameState::PLAYING;
    }
}

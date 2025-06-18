#include "dungeon.h"
#include <random>
#include <algorithm>
#include <climits>
#include <QDebug>

Dungeon::Dungeon(int rows, int cols)
    : rows(0), cols(0), playerPos(0, 0), currentHealth(100),
    initialHealth(100), gameState(GameState::PLAYING) {
    try {
        setSize(rows, cols);
    } catch (const std::exception& e) {
        qDebug() << "Dungeon constructor failed:" << e.what();
        // 设置为默认的安全尺寸
        this->rows = 5;
        this->cols = 5;
        map.resize(5, std::vector<int>(5, 0));
        dp.resize(5, std::vector<int>(5, 0));
    }
}

void Dungeon::validateMapSize(int rows, int cols) const {
    if (rows <= 0 || cols <= 0) {
        throw DungeonException("地图尺寸必须大于0");
    }

    if (rows > 100 || cols > 100) {
        throw DungeonException("地图尺寸过大，最大支持100×100");
    }

    // 检查内存使用量（粗略估计）
    size_t memoryNeeded = static_cast<size_t>(rows) * cols * sizeof(int) * 2; // map + dp
    if (memoryNeeded > 500 * 1024 * 1024) { // 500MB限制
        throw DungeonException("地图过大，超出内存限制");
    }
}

void Dungeon::setSize(int rows, int cols) {
    try {
        validateMapSize(rows, cols);

        this->rows = rows;
        this->cols = cols;

        // 清理现有数据
        map.clear();
        dp.clear();

        // 重新分配内存
        map.resize(rows, std::vector<int>(cols, 0));
        dp.resize(rows, std::vector<int>(cols, 0));

        qDebug() << "Map size set to:" << rows << "x" << cols;

    } catch (const std::bad_alloc& e) {
        throw DungeonException("内存分配失败，请尝试更小的地图尺寸");
    } catch (const DungeonException& e) {
        throw; // 重新抛出自定义异常
    } catch (const std::exception& e) {
        throw DungeonException(std::string("设置地图尺寸时发生未知错误: ") + e.what());
    }
}

void Dungeon::validateMapData() const {
    if (map.empty() || dp.empty()) {
        throw DungeonException("地图数据未初始化");
    }

    if (static_cast<int>(map.size()) != rows || static_cast<int>(dp.size()) != rows) {
        throw DungeonException("地图行数不匹配");
    }

    for (int i = 0; i < rows; ++i) {
        if (static_cast<int>(map[i].size()) != cols || static_cast<int>(dp[i].size()) != cols) {
            throw DungeonException("地图列数不匹配");
        }
    }
}

void Dungeon::generateMap() {
    try {
        validateMapData();

        std::random_device rd;
        std::mt19937 gen(rd());

        bool hasSolution = false;
        int attempts = 0;
        int maxAttempts = (rows * cols > 1000) ? 10 : 100; // 大地图减少尝试次数

        qDebug() << "Generating map with" << maxAttempts << "max attempts";

        while (!hasSolution && attempts < maxAttempts) {
            try {
                // 根据地图大小调整随机数范围
                int mapSize = rows * cols;
                int minVal, maxVal;

                if (mapSize <= 25) {  // 5x5及以下
                    minVal = -30;
                    maxVal = 15;
                } else if (mapSize <= 100) {  // 10x10及以下
                    minVal = -16;
                    maxVal = 8;
                } else if (mapSize <= 1000) {  // 大地图使用更保守的范围
                    minVal = -8;
                    maxVal = 4;
                } else {  // 超大地图
                    minVal = -4;
                    maxVal = 2;
                }

                std::uniform_int_distribution<> dis(minVal, maxVal);

                // 生成随机地图
                for (int i = 0; i < rows; ++i) {
                    for (int j = 0; j < cols; ++j) {
                        map[i][j] = dis(gen);
                    }
                }

                // 检查是否有解
                initializeDp();
                solveDp();

                // 检查最小健康值是否在合理范围内
                int minHealth = dp[0][0];
                int reasonableMax = (mapSize > 1000) ? mapSize : mapSize * 2;

                if (minHealth > 0 && minHealth <= reasonableMax) {
                    hasSolution = true;
                    qDebug() << "Map generation successful, attempts:" << attempts + 1;
                }

            } catch (const std::exception& e) {
                qDebug() << "Map generation attempt" << attempts << "failed:" << e.what();
            }

            attempts++;
        }

        // 如果多次尝试仍无合理解，生成简单的可解地图
        if (!hasSolution) {
            qDebug() << "Using fallback map generation";
            generateFallbackMap();
        }

    } catch (const DungeonException& e) {
        throw;
    } catch (const std::exception& e) {
        throw DungeonException(std::string("生成地图时发生错误: ") + e.what());
    }
}

void Dungeon::generateFallbackMap() {
    try {
        validateMapData();

        // 生成一个保证有解的简单地图
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                // 使用简单的模式确保可解性
                if ((i + j) % 4 == 0) {
                    map[i][j] = 1;  // 偶尔的增益
                } else if ((i + j) % 4 == 3) {
                    map[i][j] = -1; // 偶尔的伤害
                } else {
                    map[i][j] = 0;  // 大部分中性
                }
            }
        }

        qDebug() << "Fallback map generated successfully";

    } catch (const std::exception& e) {
        throw DungeonException(std::string("生成后备地图失败: ") + e.what());
    }
}

void Dungeon::initializeDp() {
    try {
        validateMapData();

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                dp[i][j] = INT_MAX;
            }
        }

    } catch (const std::exception& e) {
        throw DungeonException(std::string("初始化DP表失败: ") + e.what());
    }
}

void Dungeon::solveDp() {
    try {
        validateMapData();

        if (rows == 0 || cols == 0) {
            throw DungeonException("地图尺寸为0，无法计算DP");
        }

        // 初始化最后一个位置
        dp[rows-1][cols-1] = std::max(1, 1 - map[rows-1][cols-1]);

        // 填充最后一行
        for (int j = cols - 2; j >= 0; --j) {
            if (j + 1 >= cols) {
                throw DungeonException("列索引越界");
            }
            dp[rows-1][j] = std::max(1, dp[rows-1][j+1] - map[rows-1][j]);
        }

        // 填充最后一列
        for (int i = rows - 2; i >= 0; --i) {
            if (i + 1 >= rows) {
                throw DungeonException("行索引越界");
            }
            dp[i][cols-1] = std::max(1, dp[i+1][cols-1] - map[i][cols-1]);
        }

        // 填充剩余位置
        for (int i = rows - 2; i >= 0; --i) {
            for (int j = cols - 2; j >= 0; --j) {
                if (i + 1 >= rows || j + 1 >= cols) {
                    throw DungeonException("DP计算时索引越界");
                }
                int minHealth = std::min(dp[i+1][j], dp[i][j+1]);
                dp[i][j] = std::max(1, minHealth - map[i][j]);
            }
        }

    } catch (const DungeonException& e) {
        throw;
    } catch (const std::exception& e) {
        throw DungeonException(std::string("DP计算失败: ") + e.what());
    }
}

int Dungeon::calculateMinHealth() {
    try {
        initializeDp();
        solveDp();

        if (dp.empty() || dp[0].empty()) {
            throw DungeonException("DP表为空");
        }

        return dp[0][0];

    } catch (const DungeonException& e) {
        qDebug() << "calculateMinHealth failed:" << e.what();
        throw;
    } catch (const std::exception& e) {
        qDebug() << "calculateMinHealth unexpected error:" << e.what();
        throw DungeonException(std::string("计算最小健康值失败: ") + e.what());
    }
}

std::vector<QPoint> Dungeon::getOptimalPath() {
    try {
        validateMapData();

        // 确保DP表已计算
        if (dp.empty() || dp[0].empty() || dp[0][0] == INT_MAX) {
            calculateMinHealth();
        }

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
                // 检查边界
                if (i + 1 >= rows || j + 1 >= cols) {
                    throw DungeonException("路径计算时索引越界");
                }

                // 选择dp值更小的方向
                if (dp[i+1][j] <= dp[i][j+1]) {
                    i++;
                } else {
                    j++;
                }
            }
        }

        return path;

    } catch (const DungeonException& e) {
        throw;
    } catch (const std::exception& e) {
        throw DungeonException(std::string("计算最优路径失败: ") + e.what());
    }
}

void Dungeon::resetGame(int initialHealth) {
    try {
        validateMapData();

        if (initialHealth <= 0) {
            throw DungeonException("初始健康值必须大于0");
        }

        this->initialHealth = initialHealth;
        this->currentHealth = initialHealth;
        this->playerPos = QPoint(0, 0);
        this->gameState = GameState::PLAYING;
        this->playerPath.clear();
        this->playerPath.push_back(QPoint(0, 0));

        // 应用起始位置的效果
        currentHealth += map[0][0];
        updateGameState();

    } catch (const DungeonException& e) {
        throw;
    } catch (const std::exception& e) {
        throw DungeonException(std::string("重置游戏失败: ") + e.what());
    }
}

bool Dungeon::canMove(int dx, int dy) const {
    try {
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

    } catch (const std::exception& e) {
        qDebug() << "canMove error:" << e.what();
        return false; // 出错时不允许移动
    }
}

bool Dungeon::movePlayer(int dx, int dy) {
    try {
        if (!canMove(dx, dy)) {
            return false;
        }

        // 移动玩家
        int newX = playerPos.x() + dx;
        int newY = playerPos.y() + dy;

        // 再次边界检查
        if (newX < 0 || newX >= cols || newY < 0 || newY >= rows) {
            throw DungeonException("移动越界");
        }

        playerPos.setX(newX);
        playerPos.setY(newY);

        // 记录路径
        playerPath.push_back(playerPos);

        // 应用房间效果
        currentHealth += map[newY][newX];

        // 更新游戏状态
        updateGameState();

        return true;

    } catch (const DungeonException& e) {
        qDebug() << "movePlayer failed:" << e.what();
        return false;
    } catch (const std::exception& e) {
        qDebug() << "movePlayer unexpected error:" << e.what();
        return false;
    }
}

void Dungeon::updateGameState() {
    try {
        if (currentHealth <= 0) {
            gameState = GameState::LOST;
        } else if (playerPos.x() == cols - 1 && playerPos.y() == rows - 1) {
            gameState = GameState::WON;
        } else {
            gameState = GameState::PLAYING;
        }

    } catch (const std::exception& e) {
        qDebug() << "updateGameState error:" << e.what();
        gameState = GameState::LOST; // 出错时设为失败状态
    }
}

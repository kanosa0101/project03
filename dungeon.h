#ifndef DUNGEON_H
#define DUNGEON_H

#include <vector>
#include <QPoint>

enum class GameMode {
    AUTO,   // 自动模式
    MANUAL  // 手动模式
};

enum class GameState {
    PLAYING,    // 游戏中
    WON,        // 胜利
    LOST        // 失败
};

class Dungeon {
public:
    Dungeon(int rows = 5, int cols = 5);

    // 生成随机地图
    void generateMap();

    // 计算最小初始健康点数（自动模式用）
    int calculateMinHealth();

    // 获取最优路径（自动模式用）
    std::vector<QPoint> getOptimalPath();

    // 手动模式相关
    void resetGame(int initialHealth = 100);
    bool movePlayer(int dx, int dy);  // 移动玩家，返回是否成功
    bool canMove(int dx, int dy) const;  // 检查是否可以移动

    // 获取游戏状态
    GameState getGameState() const { return gameState; }
    QPoint getPlayerPosition() const { return playerPos; }
    int getCurrentHealth() const { return currentHealth; }
    const std::vector<QPoint>& getPlayerPath() const { return playerPath; }

    // 获取地图数据
    const std::vector<std::vector<int>>& getMap() const { return map; }
    const std::vector<std::vector<int>>& getDpTable() const { return dp; }

    // 设置地图尺寸
    void setSize(int rows, int cols);

    // 获取尺寸
    int getRows() const { return rows; }
    int getCols() const { return cols; }

private:
    int rows, cols;
    std::vector<std::vector<int>> map;      // 地图数据
    std::vector<std::vector<int>> dp;       // 动态规划表

    // 手动模式相关
    QPoint playerPos;                       // 玩家当前位置
    int currentHealth;                      // 当前健康值
    int initialHealth;                      // 初始健康值
    std::vector<QPoint> playerPath;         // 玩家走过的路径
    GameState gameState;                    // 游戏状态

    void initializeDp();
    void solveDp();
    void updateGameState();
};

#endif // DUNGEON_H

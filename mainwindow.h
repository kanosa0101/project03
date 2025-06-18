#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QTimer>
#include <QButtonGroup>
#include <QRadioButton>
#include <QMessageBox>
#include <QKeyEvent>
#include <QPoint>
#include <vector>
#include "dungeon.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void showGameRules();
    void startGame();
    void returnToMenu();
    void generateNewMap();
    void startAutoMode();
    void startManualMode();
    void showNextPathStep();
    void resetManualGame();

private:
    void setupMainMenu();
    void setupGameInterface();
    void setupUI();
    void updateMapDisplay();
    void clearPathDisplay();
    void showAutoResults();
    void updateManualDisplay();
    void showGameResult();

    // UI组件 - 主菜单
    QWidget *menuWidget;
    QVBoxLayout *menuLayout;
    QLabel *titleLabel;
    QPushButton *startGameBtn;
    QPushButton *gameRulesBtn;
    QPushButton *exitGameBtn;

    // UI组件 - 游戏界面
    QWidget *gameWidget;
    QVBoxLayout *gameLayout;
    QHBoxLayout *controlLayout;
    QGridLayout *mapLayout;

    QSpinBox *rowsSpinBox;
    QSpinBox *colsSpinBox;
    QPushButton *generateBtn;
    QButtonGroup *modeGroup;
    QRadioButton *autoModeBtn;
    QRadioButton *manualModeBtn;
    QPushButton *startBtn;
    QPushButton *returnBtn;
    QPushButton *resetBtn;

    QLabel *resultLabel;
    QLabel *healthLabel;
    QLabel *positionLabel;
    QTextEdit *infoText;

    // 堆叠窗口
    QStackedWidget *stackedWidget;

    // 地图显示标签
    std::vector<std::vector<QLabel*>> mapLabels;

    // 游戏逻辑
    Dungeon dungeon;
    GameMode currentMode;
    std::vector<QPoint> autoPath;
    int pathIndex;
    QTimer *pathTimer;
};

#endif // MAINWINDOW_H

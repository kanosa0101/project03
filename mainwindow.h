#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QTextEdit>
#include <QTimer>
#include <QKeyEvent>
#include <QMessageBox>
#include <stdexcept>
#include "dungeon.h"
#include "dungeonmapmodel.h"
#include "dungeontableview.h"
#include "maptablewindow.h"

class MainWindowException : public std::runtime_error {
public:
    explicit MainWindowException(const std::string& message) : std::runtime_error(message) {}
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void startGame();
    void returnToMenu();
    void showGameRules();
    void generateNewMap();
    void startAutoMode();
    void startManualMode();
    void resetManualGame();
    void showNextPathStep();
    void showTableWindow();

private:
    void setupUI();
    void setupMainMenu();
    void setupGameInterface();
    void updateMapDisplay();
    void updateManualDisplay();
    void showGameResult();
    void clearPathDisplay();
    void showAutoResults();

    // 安全的操作方法
    void safeGenerateNewMap();
    void safeUpdateMapDisplay();
    void safeStartAutoMode();
    void safeStartManualMode();
    void safeShowTableWindow();
    void safeUpdateManualDisplay();
    void handleException(const std::exception& e, const QString& operation);

    // UI组件
    QStackedWidget* stackedWidget;

    // 主菜单
    QWidget* menuWidget;
    QVBoxLayout* menuLayout;
    QLabel* titleLabel;
    QPushButton* startGameBtn;
    QPushButton* gameRulesBtn;
    QPushButton* exitGameBtn;

    // 游戏界面
    QWidget* gameWidget;
    QVBoxLayout* gameLayout;
    QHBoxLayout* controlLayout;

    QSpinBox* rowsSpinBox;
    QSpinBox* colsSpinBox;
    QPushButton* generateBtn;

    QButtonGroup* modeGroup;
    QRadioButton* autoModeBtn;
    QRadioButton* manualModeBtn;

    QPushButton* startBtn;
    QPushButton* resetBtn;
    QPushButton* returnBtn;
    QPushButton* showTableBtn;  // 新增显示表格按钮

    QLabel* resultLabel;
    QLabel* healthLabel;
    QLabel* positionLabel;

    DungeonMapModel* mapModel;
    DungeonTableView* mapTableView;
    QTextEdit* infoText;

    // 游戏逻辑
    Dungeon dungeon;
    std::vector<QPoint> autoPath;
    QTimer* pathTimer;
    int pathIndex;
    GameMode currentMode;

    // 表格窗口
    MapTableWindow* tableWindow;
};

#endif // MAINWINDOW_H

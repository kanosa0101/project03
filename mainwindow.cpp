#include "mainwindow.h"
#include <QApplication>
#include <sstream>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), dungeon(5, 5), pathIndex(0), currentMode(GameMode::AUTO) {
    setupUI();
    pathTimer = new QTimer(this);
    connect(pathTimer, &QTimer::timeout, this, &MainWindow::showNextPathStep);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    // 创建堆叠窗口
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    setupMainMenu();
    setupGameInterface();

    // 默认显示主菜单
    stackedWidget->setCurrentWidget(menuWidget);

    setWindowTitle("地下城游戏 - 骑士拯救公主");
    setMinimumSize(700, 600);
    setFocusPolicy(Qt::StrongFocus);  // 确保能接收键盘事件
}

void MainWindow::setupMainMenu() {
    menuWidget = new QWidget();
    menuLayout = new QVBoxLayout(menuWidget);

    // 添加间距
    menuLayout->addStretch();

    // 游戏标题
    titleLabel = new QLabel("🏰 地下城游戏 🏰");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #2C3E50; margin: 20px;");
    menuLayout->addWidget(titleLabel);

    QLabel *subTitle = new QLabel("骑士拯救公主");
    subTitle->setAlignment(Qt::AlignCenter);
    subTitle->setStyleSheet("font-size: 18px; color: #7F8C8D; margin-bottom: 40px;");
    menuLayout->addWidget(subTitle);

    // 菜单按钮
    startGameBtn = new QPushButton("🎮 开始游戏");
    startGameBtn->setStyleSheet("QPushButton { font-size: 18px; padding: 15px 30px; background-color: #3498DB; color: white; border: none; border-radius: 8px; } QPushButton:hover { background-color: #2980B9; }");
    menuLayout->addWidget(startGameBtn);

    gameRulesBtn = new QPushButton("📖 游戏规则");
    gameRulesBtn->setStyleSheet("QPushButton { font-size: 16px; padding: 12px 25px; background-color: #2ECC71; color: white; border: none; border-radius: 8px; } QPushButton:hover { background-color: #27AE60; }");
    menuLayout->addWidget(gameRulesBtn);

    exitGameBtn = new QPushButton("🚪 退出游戏");
    exitGameBtn->setStyleSheet("QPushButton { font-size: 16px; padding: 12px 25px; background-color: #E74C3C; color: white; border: none; border-radius: 8px; } QPushButton:hover { background-color: #C0392B; }");
    menuLayout->addWidget(exitGameBtn);

    menuLayout->addStretch();

    // 设置按钮间距
    menuLayout->setSpacing(15);
    menuLayout->setContentsMargins(100, 50, 100, 50);

    // 连接信号
    connect(startGameBtn, &QPushButton::clicked, this, &MainWindow::startGame);
    connect(gameRulesBtn, &QPushButton::clicked, this, &MainWindow::showGameRules);
    connect(exitGameBtn, &QPushButton::clicked, this, &QWidget::close);

    stackedWidget->addWidget(menuWidget);
}

void MainWindow::setupGameInterface() {
    gameWidget = new QWidget();
    gameLayout = new QVBoxLayout(gameWidget);

    // 控制面板
    controlLayout = new QHBoxLayout();

    controlLayout->addWidget(new QLabel("行数:"));
    rowsSpinBox = new QSpinBox();
    rowsSpinBox->setRange(3, 15);
    rowsSpinBox->setValue(5);
    controlLayout->addWidget(rowsSpinBox);

    controlLayout->addWidget(new QLabel("列数:"));
    colsSpinBox = new QSpinBox();
    colsSpinBox->setRange(3, 15);
    colsSpinBox->setValue(5);
    controlLayout->addWidget(colsSpinBox);

    generateBtn = new QPushButton("生成地图");
    generateBtn->setStyleSheet("background-color: #3498DB; color: white; padding: 8px 16px; border: none; border-radius: 4px;");
    controlLayout->addWidget(generateBtn);

    // 游戏模式选择
    controlLayout->addWidget(new QLabel("模式:"));
    modeGroup = new QButtonGroup(this);
    autoModeBtn = new QRadioButton("自动");
    manualModeBtn = new QRadioButton("手动");
    autoModeBtn->setChecked(true);
    modeGroup->addButton(autoModeBtn);
    modeGroup->addButton(manualModeBtn);
    controlLayout->addWidget(autoModeBtn);
    controlLayout->addWidget(manualModeBtn);

    startBtn = new QPushButton("开始");
    startBtn->setStyleSheet("background-color: #2ECC71; color: white; padding: 8px 16px; border: none; border-radius: 4px;");
    startBtn->setEnabled(false);
    controlLayout->addWidget(startBtn);

    resetBtn = new QPushButton("重置");
    resetBtn->setStyleSheet("background-color: #F39C12; color: white; padding: 8px 16px; border: none; border-radius: 4px;");
    resetBtn->setEnabled(false);
    controlLayout->addWidget(resetBtn);

    returnBtn = new QPushButton("返回主菜单");
    returnBtn->setStyleSheet("background-color: #E74C3C; color: white; padding: 8px 16px; border: none; border-radius: 4px;");
    controlLayout->addWidget(returnBtn);

    controlLayout->addStretch();
    gameLayout->addLayout(controlLayout);

    // 状态显示
    QHBoxLayout *statusLayout = new QHBoxLayout();
    resultLabel = new QLabel("请先生成地图");
    resultLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #2C3E50;");
    statusLayout->addWidget(resultLabel);

    healthLabel = new QLabel("");
    healthLabel->setStyleSheet("font-size: 14px; color: #E74C3C;");
    statusLayout->addWidget(healthLabel);

    positionLabel = new QLabel("");
    positionLabel->setStyleSheet("font-size: 14px; color: #3498DB;");
    statusLayout->addWidget(positionLabel);

    statusLayout->addStretch();
    gameLayout->addLayout(statusLayout);

    // 地图显示区域
    QWidget *mapWidget = new QWidget();
    mapLayout = new QGridLayout(mapWidget);
    mapLayout->setSpacing(2);
    gameLayout->addWidget(mapWidget);

    // 信息显示
    infoText = new QTextEdit();
    infoText->setMaximumHeight(120);
    infoText->setReadOnly(true);
    infoText->setStyleSheet("background-color: #FFC0CB; border: 1px solid #BDC3C7;");
    gameLayout->addWidget(infoText);

    // 连接信号
    connect(generateBtn, &QPushButton::clicked, this, &MainWindow::generateNewMap);
    connect(startBtn, &QPushButton::clicked, [this]() {
        if (autoModeBtn->isChecked()) {
            startAutoMode();
        } else {
            startManualMode();
        }
    });
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::resetManualGame);
    connect(returnBtn, &QPushButton::clicked, this, &MainWindow::returnToMenu);

    stackedWidget->addWidget(gameWidget);
}

void MainWindow::showGameRules() {
    QString rules =
        "🎯 游戏目标：\n"
        "帮助骑士从左上角(起点)到达右下角(终点)拯救公主\n\n"

        "🎮 游戏模式：\n"
        "• 自动模式：系统计算最优路径并演示\n"
        "• 手动模式：用方向键控制骑士移动\n\n"

        "🗺️ 地图说明：\n"
        "• 绿色房间：增益房间(+健康值)\n"
        "• 红色房间：伤害房间(-健康值)\n"
        "• 灰色房间：中性房间(无影响)\n"
        "• 蓝色边框：起点位置\n"
        "• 红色边框：终点位置\n"
        "• 橙色背景：走过的路径\n\n"

        "🎯 移动规则：\n"
        "• 只能向右→或向下↓移动\n"
        "• 手动模式使用方向键控制\n"
        "• 健康值≤0时游戏失败\n"
        "• 到达终点且健康值>0时获胜\n\n"

        "🎲 操作说明：\n"
        "1. 设置地图尺寸(3×3到15×15)\n"
        "2. 点击'生成地图'创建随机地下城\n"
        "3. 选择游戏模式\n"
        "4. 点击'开始'进行游戏";

    QMessageBox::information(this, "游戏规则", rules);
}

void MainWindow::startGame() {
    stackedWidget->setCurrentWidget(gameWidget);
    generateNewMap();  // 自动生成一个地图
}

void MainWindow::returnToMenu() {
    // 重置游戏状态
    pathTimer->stop();
    clearPathDisplay();
    stackedWidget->setCurrentWidget(menuWidget);
}

void MainWindow::updateMapDisplay() {
    // 清理现有标签
    for (auto& row : mapLabels) {
        for (auto& label : row) {
            mapLayout->removeWidget(label);
            delete label;
        }
    }
    mapLabels.clear();

    int rows = dungeon.getRows();
    int cols = dungeon.getCols();
    const auto& map = dungeon.getMap();

    mapLabels.resize(rows, std::vector<QLabel*>(cols));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QLabel* label = new QLabel(QString::number(map[i][j]));
            label->setAlignment(Qt::AlignCenter);
            label->setMinimumSize(50, 50);
            label->setStyleSheet("border: 2px solid black; font-weight: bold; font-size: 14px;");

            // 根据值设置颜色
            if (map[i][j] > 0) {
                label->setStyleSheet(label->styleSheet() + "background-color: #2ECC71; color: white;");
            } else if (map[i][j] < 0) {
                label->setStyleSheet(label->styleSheet() + "background-color: #E74C3C; color: white;");
            } else {
                label->setStyleSheet(label->styleSheet() + "background-color: #95A5A6; color: white;");
            }

            // 标记起点和终点
            if (i == 0 && j == 0) {
                label->setStyleSheet(label->styleSheet() + "border-color: #3498DB; border-width: 4px;");
            } else if (i == rows-1 && j == cols-1) {
                label->setStyleSheet(label->styleSheet() + "border-color: #3498DB; border-width: 4px;");
            }

            mapLabels[i][j] = label;
            mapLayout->addWidget(label, i, j);
        }
    }
}


void MainWindow::generateNewMap() {
    int rows = rowsSpinBox->value();
    int cols = colsSpinBox->value();

    dungeon.setSize(rows, cols);
    dungeon.generateMap();

    updateMapDisplay();
    clearPathDisplay();

    startBtn->setEnabled(true);
    resetBtn->setEnabled(false);
    resultLabel->setText("地图已生成，选择模式并点击开始");
    healthLabel->setText("");
    positionLabel->setText("");

    // 显示地图信息
    std::ostringstream info;
    info << "地图尺寸: " << rows << "×" << cols << "\n";
    info << "🔵 蓝色边框: 起点(骑士) | 🔴 红色边框: 终点(公主)\n";
    info << "🟢 绿色: 增益房间 | 🔴 红色: 伤害房间 | ⚫ 灰色: 中性房间";
    infoText->setText(QString::fromStdString(info.str()));
}

void MainWindow::startAutoMode() {
    currentMode = GameMode::AUTO;
    int minHealth = dungeon.calculateMinHealth();
    autoPath = dungeon.getOptimalPath();

    resultLabel->setText(QString("自动模式 - 最小初始健康点数: %1").arg(minHealth));

    std::ostringstream info;
    info << "🤖 自动模式计算完成!\n";
    info << "最优路径长度: " << autoPath.size() << " 步\n";
    info << "路径将以动画形式显示... 🔍橙色背景显示路径";
    infoText->setText(QString::fromStdString(info.str()));

    clearPathDisplay();
    pathIndex = 0;
    startBtn->setEnabled(false);
    pathTimer->start(800);
}

void MainWindow::startManualMode() {
    currentMode = GameMode::MANUAL;
    dungeon.resetGame(100);  // 默认初始健康值100

    updateManualDisplay();

    resultLabel->setText("手动模式 - 使用方向键移动骑士");
    startBtn->setEnabled(false);
    resetBtn->setEnabled(true);

    std::ostringstream info;
    info << "🎮 手动模式开始!\n";
    info << "使用方向键 ➡️⬇️ 控制骑士移动\n";
    info << "目标: 到达终点且健康值 > 0";
    infoText->setText(QString::fromStdString(info.str()));

    setFocus();  // 确保能接收键盘事件
}

void MainWindow::resetManualGame() {
    if (currentMode == GameMode::MANUAL) {
        dungeon.resetGame(100);
        updateManualDisplay();

        resultLabel->setText("手动模式 - 游戏已重置");
        std::ostringstream info;
        info << "🔄 游戏已重置!\n";
        info << "使用方向键 ➡️⬇️ 控制骑士移动\n";
        info << "目标: 到达终点且健康值 > 0";
        infoText->setText(QString::fromStdString(info.str()));

        setFocus();
    }
}

void MainWindow::updateManualDisplay() {
    updateMapDisplay();

    // 显示玩家路径（橙色背景）
    const auto& playerPath = dungeon.getPlayerPath();
    for (const auto& pos : playerPath) {
        QLabel* label = mapLabels[pos.y()][pos.x()];
        QString currentStyle = label->styleSheet();
        // 使用橙色背景
        currentStyle = currentStyle.replace(
            QRegularExpression("background-color: [^;]+;"),
            "background-color: #F39C12;"
            );
        label->setStyleSheet(currentStyle);
    }

    // 更新状态显示
    QPoint playerPos = dungeon.getPlayerPosition();
    healthLabel->setText(QString("❤️ 健康值: %1").arg(dungeon.getCurrentHealth()));
    positionLabel->setText(QString("📍 位置: (%1, %2)").arg(playerPos.x()).arg(playerPos.y()));

    // 检查游戏结束
    if (dungeon.getGameState() != GameState::PLAYING) {
        showGameResult();
    }
}

void MainWindow::showGameResult() {
    GameState state = dungeon.getGameState();
    if (state == GameState::WON) {
        resultLabel->setText("🎉 恭喜！骑士成功拯救公主！");
        QMessageBox::information(this, "游戏胜利", "🎉 恭喜！\n骑士成功拯救了公主！\n\n点击'重置'可以重新开始游戏。");
    } else if (state == GameState::LOST) {
        resultLabel->setText("💀 游戏失败！骑士倒下了...");
        QMessageBox::warning(this, "游戏失败", "💀 很遗憾！\n骑士的健康值耗尽了...\n\n点击'重置'可以重新尝试。");
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (currentMode == GameMode::MANUAL && dungeon.getGameState() == GameState::PLAYING) {
        bool moved = false;

        switch (event->key()) {
        case Qt::Key_Right:
            moved = dungeon.movePlayer(1, 0);
            break;
        case Qt::Key_Down:
            moved = dungeon.movePlayer(0, 1);
            break;
        default:
            QMainWindow::keyPressEvent(event);
            return;
        }

        if (moved) {
            updateManualDisplay();
        }
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::showNextPathStep() {
    if (pathIndex >= autoPath.size()) {
        pathTimer->stop();
        showAutoResults();
        return;
    }

    QPoint pos = autoPath[pathIndex];
    QLabel* label = mapLabels[pos.y()][pos.x()];

    // 设置橙色背景显示路径
    QString currentStyle = label->styleSheet();
    currentStyle = currentStyle.replace(
        QRegularExpression("background-color: [^;]+;"),
        "background-color: #F39C12;"
        );
    label->setStyleSheet(currentStyle);

    pathIndex++;
}

void MainWindow::clearPathDisplay() {
    pathTimer->stop();
    updateMapDisplay();
}

void MainWindow::showAutoResults() {
    std::ostringstream info;
    info << "✅ 自动模式路径显示完成!\n";
    info << "🔶 橙色背景: 最优路径\n";
    info << "骑士成功找到拯救公主的最佳路线!";
    infoText->setText(QString::fromStdString(info.str()));

    startBtn->setEnabled(true);
}

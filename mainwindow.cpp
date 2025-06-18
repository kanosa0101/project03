#include "mainwindow.h"
#include <QApplication>
#include <sstream>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), dungeon(5, 5), pathIndex(0), currentMode(GameMode::AUTO),
    tableWindow(nullptr), stackedWidget(nullptr), mapModel(nullptr), mapTableView(nullptr) {
    try {
        setupUI();
        pathTimer = new QTimer(this);
        connect(pathTimer, &QTimer::timeout, this, &MainWindow::showNextPathStep);
    } catch (const std::exception& e) {
        handleException(e, "初始化主窗口");
    }
}

MainWindow::~MainWindow() {
    try {
        if (tableWindow) {
            tableWindow->deleteLater();
        }
        if (pathTimer) {
            pathTimer->stop();
        }
    } catch (const std::exception& e) {
        qDebug() << "Destructor error:" << e.what();
    }
}

void MainWindow::handleException(const std::exception& e, const QString& operation) {
    qDebug() << operation << "发生错误:" << e.what();

    QString message = QString("%1时发生错误:\n%2\n\n程序将尝试继续运行，但可能功能受限。")
                          .arg(operation)
                          .arg(QString::fromStdString(e.what()));

    QMessageBox::warning(this, "错误", message);

    // 尝试重置到安全状态
    try {
        if (pathTimer) {
            pathTimer->stop();
        }
        if (resultLabel) {
            resultLabel->setText("发生错误，请重新操作");
        }
    } catch (...) {
        // 忽略重置过程中的错误
    }
}

void MainWindow::setupUI() {
    try {
        // 创建堆叠窗口
        stackedWidget = new QStackedWidget(this);
        if (!stackedWidget) {
            throw MainWindowException("创建堆叠窗口失败");
        }
        setCentralWidget(stackedWidget);

        setupMainMenu();
        setupGameInterface();

        // 默认显示主菜单
        stackedWidget->setCurrentWidget(menuWidget);

        setWindowTitle("地下城游戏 - 骑士拯救公主");
        setMinimumSize(800, 700);
        setFocusPolicy(Qt::StrongFocus);

    } catch (const std::exception& e) {
        throw MainWindowException(std::string("设置界面失败: ") + e.what());
    }
}

void MainWindow::setupMainMenu() {
    try {
        menuWidget = new QWidget();
        if (!menuWidget) {
            throw MainWindowException("创建菜单组件失败");
        }

        menuLayout = new QVBoxLayout(menuWidget);

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

        menuLayout->setSpacing(15);
        menuLayout->setContentsMargins(100, 50, 100, 50);

        // 连接信号
        connect(startGameBtn, &QPushButton::clicked, this, &MainWindow::startGame);
        connect(gameRulesBtn, &QPushButton::clicked, this, &MainWindow::showGameRules);
        connect(exitGameBtn, &QPushButton::clicked, this, &QWidget::close);

        stackedWidget->addWidget(menuWidget);

    } catch (const std::exception& e) {
        throw MainWindowException(std::string("设置主菜单失败: ") + e.what());
    }
}

void MainWindow::setupGameInterface() {
    try {
        gameWidget = new QWidget();
        if (!gameWidget) {
            throw MainWindowException("创建游戏界面组件失败");
        }

        gameLayout = new QVBoxLayout(gameWidget);

        // 控制面板
        controlLayout = new QHBoxLayout();

        controlLayout->addWidget(new QLabel("行数:"));
        rowsSpinBox = new QSpinBox();
        rowsSpinBox->setRange(3, 100);
        rowsSpinBox->setValue(5);
        controlLayout->addWidget(rowsSpinBox);

        controlLayout->addWidget(new QLabel("列数:"));
        colsSpinBox = new QSpinBox();
        colsSpinBox->setRange(3, 100);
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

        // 显示表格按钮
        showTableBtn = new QPushButton("显示表格");
        showTableBtn->setStyleSheet("background-color: #9B59B6; color: white; padding: 8px 16px; border: none; border-radius: 4px;");
        showTableBtn->setEnabled(false);
        controlLayout->addWidget(showTableBtn);

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
        mapModel = new DungeonMapModel(this);
        if (!mapModel) {
            throw MainWindowException("创建地图模型失败");
        }

        mapTableView = new DungeonTableView(this);
        if (!mapTableView) {
            throw MainWindowException("创建地图视图失败");
        }

        mapTableView->setModel(mapModel);
        mapTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        gameLayout->addWidget(mapTableView, 1);

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
        connect(showTableBtn, &QPushButton::clicked, this, &MainWindow::showTableWindow);
        connect(returnBtn, &QPushButton::clicked, this, &MainWindow::returnToMenu);

        stackedWidget->addWidget(gameWidget);

    } catch (const std::exception& e) {
        throw MainWindowException(std::string("设置游戏界面失败: ") + e.what());
    }
}

void MainWindow::showGameRules() {
    try {
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
            "• 蓝色边框：起点和终点位置\n"
            "• 橙色背景：走过的路径\n\n"

            "🎯 移动规则：\n"
            "• 只能向右→或向下↓移动\n"
            "• 手动模式使用方向键控制\n"
            "• 健康值≤0时游戏失败\n"
            "• 到达终点且健康值>0时获胜\n\n"

            "📊 表格功能：\n"
            "• 所有地图都会生成数据表格\n"
            "• 大于15×15的地图仅显示表格（无可视化操作）\n"
            "• 15×15及以下支持可视化和表格双模式\n"
            "• 表格可导出为CSV文件\n\n"

            "🎲 操作说明：\n"
            "1. 设置地图尺寸(3×3到50×50)\n"
            "2. 点击'生成地图'创建随机地下城\n"
            "3. 选择游戏模式（大地图仅计算结果）\n"
            "4. 点击'显示表格'查看详细数据\n"
            "5. 点击'开始'进行游戏";

        QMessageBox::information(this, "游戏规则", rules);

    } catch (const std::exception& e) {
        handleException(e, "显示游戏规则");
    }
}

void MainWindow::startGame() {
    try {
        if (!stackedWidget || !gameWidget) {
            throw MainWindowException("界面组件未初始化");
        }

        stackedWidget->setCurrentWidget(gameWidget);
        generateNewMap();

    } catch (const std::exception& e) {
        handleException(e, "开始游戏");
    }
}

void MainWindow::returnToMenu() {
    try {
        if (pathTimer) {
            pathTimer->stop();
        }
        clearPathDisplay();
        if (tableWindow) {
            tableWindow->close();
        }
        if (stackedWidget && menuWidget) {
            stackedWidget->setCurrentWidget(menuWidget);
        }

    } catch (const std::exception& e) {
        handleException(e, "返回主菜单");
    }
}

void MainWindow::generateNewMap() {
    try {
        safeGenerateNewMap();
    } catch (const std::exception& e) {
        handleException(e, "生成地图");
    }
}

// void MainWindow::safeGenerateNewMap() {
//     if (!rowsSpinBox || !colsSpinBox) {
//         throw MainWindowException("尺寸控件未初始化");
//     }

//     int rows = rowsSpinBox->value();
//     int cols = colsSpinBox->value();

//     // 设置状态提示
//     if (resultLabel) {
//         resultLabel->setText("正在生成地图，请稍候...");
//     }

//     QApplication::processEvents(); // 刷新界面

//     // 生成地图
//     dungeon.setSize(rows, cols);
//     dungeon.generateMap();

//     safeUpdateMapDisplay();
//     clearPathDisplay();

//     if (showTableBtn) {
//         showTableBtn->setEnabled(true);
//     }

//     // 关闭之前的表格窗口
//     if (tableWindow) {
//         tableWindow->close();
//         tableWindow = nullptr;
//     }

//     if (healthLabel) {
//         healthLabel->setText("");
//     }
//     if (positionLabel) {
//         positionLabel->setText("");
//     }

//     // 计算最小健康值
//     int minHealth = dungeon.calculateMinHealth();

//     if (rows > 15 || cols > 15) {
//         // 大地图模式
//         if (resultLabel) {
//             resultLabel->setText(QString("大地图模式 (%1×%2) - 最小初始健康值: %3").arg(rows).arg(cols).arg(minHealth));
//         }

//         std::ostringstream info;
//         info << "🗺️ 大地图已生成! (" << rows << "×" << cols << ")\n";
//         info << "📊 最小初始健康值: " << minHealth << "\n";
//         info << "点击'显示表格'查看详细数据";

//         if (infoText) {
//             infoText->setText(QString::fromStdString(info.str()));
//         }
//     } else {
//         // 小地图模式
//         if (resultLabel) {
//             resultLabel->setText("地图已生成，选择模式并点击开始");
//         }

//         std::ostringstream info;
//         info << "地图尺寸: " << rows << "×" << cols << " | 最小健康值: " << minHealth << "\n";
//         info << "🔵 蓝色边框: 起点(骑士) | 🔵 蓝色边框: 终点(公主)\n";
//         info << "🟢 绿色: 增益房间 | 🔴 红色: 伤害房间 | ⚫ 灰色: 中性房间";

//         if (infoText) {
//             infoText->setText(QString::fromStdString(info.str()));
//         }
//     }
// }

void MainWindow::safeGenerateNewMap() {
    if (!rowsSpinBox || !colsSpinBox) {
        throw MainWindowException("尺寸控件未初始化");
    }

    int rows = rowsSpinBox->value();
    int cols = colsSpinBox->value();

    // 设置状态提示
    if (resultLabel) {
        resultLabel->setText("正在生成地图，请稍候...");
    }

    QApplication::processEvents(); // 刷新界面

    // 生成地图
    dungeon.setSize(rows, cols);
    dungeon.generateMap();

    safeUpdateMapDisplay();
    clearPathDisplay();

    if (showTableBtn) {
        showTableBtn->setEnabled(true);
    }

    // 关闭之前的表格窗口
    if (tableWindow) {
        tableWindow->close();
        tableWindow = nullptr;
    }

    if (healthLabel) {
        healthLabel->setText("");
    }
    if (positionLabel) {
        positionLabel->setText("");
    }

    // 计算最小健康值
    int minHealth = dungeon.calculateMinHealth();

    if (rows > 15 || cols > 15) {
        // 大地图模式
        if (resultLabel) {
            resultLabel->setText(QString("大地图模式 (%1×%2) - 最小初始健康值: %3").arg(rows).arg(cols).arg(minHealth));
        }

        std::ostringstream info;
        info << "🗺️ 大地图已生成! (" << rows << "×" << cols << ")\n";
        info << "📊 最小初始健康值: " << minHealth << "\n";
        info << "表格窗口已自动打开";  // 修改：提示信息

        if (infoText) {
            infoText->setText(QString::fromStdString(info.str()));
        }

        // 新增：大地图自动弹出表格窗口
        safeShowTableWindow();
    } else {
        // 小地图模式
        if (resultLabel) {
            resultLabel->setText("地图已生成，选择模式并点击开始");
        }

        std::ostringstream info;
        info << "地图尺寸: " << rows << "×" << cols << " | 最小健康值: " << minHealth << "\n";
        info << "🔵 蓝色边框: 起点(骑士) | 🔵 蓝色边框: 终点(公主)\n";
        info << "🟢 绿色: 增益房间 | 🔴 红色: 伤害房间 | ⚫ 灰色: 中性房间";

        if (infoText) {
            infoText->setText(QString::fromStdString(info.str()));
        }
    }
}

void MainWindow::updateMapDisplay() {
    try {
        safeUpdateMapDisplay();
    } catch (const std::exception& e) {
        handleException(e, "更新地图显示");
    }
}

void MainWindow::safeUpdateMapDisplay() {
    if (!mapModel || !mapTableView) {
        throw MainWindowException("地图显示组件未初始化");
    }

    // 根据地图大小决定是否显示可视化界面
    int rows = dungeon.getRows();
    int cols = dungeon.getCols();

    if (rows > 15 || cols > 15) {
        // 大地图：完全断开TableView连接
        mapTableView->setModel(nullptr);
        mapTableView->hide();
        mapModel->setDungeon(nullptr);

        if (autoModeBtn) autoModeBtn->setEnabled(false);
        if (manualModeBtn) manualModeBtn->setEnabled(false);
        if (startBtn) startBtn->setEnabled(false);
        if (resetBtn) resetBtn->setEnabled(false);
    } else {
        // 小地图：正常连接
        mapModel->setDungeon(&dungeon);
        mapTableView->setModel(mapModel);
        mapTableView->show();
        mapTableView->updateCellSize();

        if (autoModeBtn) autoModeBtn->setEnabled(true);
        if (manualModeBtn) manualModeBtn->setEnabled(true);
        if (startBtn) startBtn->setEnabled(true);
        if (resetBtn) resetBtn->setEnabled(false);
    }
}

void MainWindow::startAutoMode() {
    try {
        safeStartAutoMode();
    } catch (const std::exception& e) {
        handleException(e, "启动自动模式");
    }
}

void MainWindow::safeStartAutoMode() {
    currentMode = GameMode::AUTO;
    int minHealth = dungeon.calculateMinHealth();
    autoPath = dungeon.getOptimalPath();

    if (resultLabel) {
        resultLabel->setText(QString("自动模式 - 最小初始健康点数: %1").arg(minHealth));
    }

    std::ostringstream info;
    info << "🤖 自动模式计算完成!\n";
    info << "最优路径长度: " << autoPath.size() << " 步\n";
    info << "路径将以动画形式显示... 🔍橙色背景显示路径";

    if (infoText) {
        infoText->setText(QString::fromStdString(info.str()));
    }

    clearPathDisplay();
    pathIndex = 0;

    if (startBtn) {
        startBtn->setEnabled(false);
    }

    if (pathTimer) {
        pathTimer->start(500);
    }
}

void MainWindow::startManualMode() {
    try {
        safeStartManualMode();
    } catch (const std::exception& e) {
        handleException(e, "启动手动模式");
    }
}

void MainWindow::safeStartManualMode() {
    currentMode = GameMode::MANUAL;
    dungeon.resetGame(100);

    safeUpdateManualDisplay();

    if (resultLabel) {
        resultLabel->setText("手动模式 - 使用方向键移动骑士");
    }

    if (startBtn) {
        startBtn->setEnabled(false);
    }
    if (resetBtn) {
        resetBtn->setEnabled(true);
    }

    std::ostringstream info;
    info << "🎮 手动模式开始!\n";
    info << "使用方向键 ➡️⬇️ 控制骑士移动\n";
    info << "目标: 到达终点且健康值 > 0";

    if (infoText) {
        infoText->setText(QString::fromStdString(info.str()));
    }

    setFocus();
}

void MainWindow::resetManualGame() {
    try {
        if (currentMode == GameMode::MANUAL) {
            dungeon.resetGame(100);
            safeUpdateManualDisplay();

            if (resultLabel) {
                resultLabel->setText("手动模式 - 游戏已重置");
            }

            std::ostringstream info;
            info << "🔄 游戏已重置!\n";
            info << "使用方向键 ➡️⬇️ 控制骑士移动\n";
            info << "目标: 到达终点且健康值 > 0";

            if (infoText) {
                infoText->setText(QString::fromStdString(info.str()));
            }

            setFocus();
        }
    } catch (const std::exception& e) {
        handleException(e, "重置手动游戏");
    }
}

void MainWindow::updateManualDisplay() {
    try {
        safeUpdateManualDisplay();
    } catch (const std::exception& e) {
        handleException(e, "更新手动模式显示");
    }
}

void MainWindow::safeUpdateManualDisplay() {
    if (!mapModel) {
        throw MainWindowException("地图模型未初始化");
    }

    mapModel->setPlayerPath(dungeon.getPlayerPath());

    // 更新状态显示
    QPoint playerPos = dungeon.getPlayerPosition();

    if (healthLabel) {
        healthLabel->setText(QString("❤️ 健康值: %1").arg(dungeon.getCurrentHealth()));
    }
    if (positionLabel) {
        positionLabel->setText(QString("📍 位置: (%1, %2)").arg(playerPos.x()).arg(playerPos.y()));
    }

    // 检查游戏结束
    if (dungeon.getGameState() != GameState::PLAYING) {
        showGameResult();
    }
}

void MainWindow::showGameResult() {
    try {
        GameState state = dungeon.getGameState();
        if (state == GameState::WON) {
            if (resultLabel) {
                resultLabel->setText("🎉 恭喜！骑士成功拯救公主！");
            }
            QMessageBox::information(this, "游戏胜利", "🎉 恭喜！\n骑士成功拯救了公主！\n\n点击'重置'可以重新开始游戏。");
        } else if (state == GameState::LOST) {
            if (resultLabel) {
                resultLabel->setText("💀 游戏失败！骑士倒下了...");
            }
            QMessageBox::warning(this, "游戏失败", "💀 很遗憾！\n骑士的健康值耗尽了...\n\n点击'重置'可以重新尝试。");
        }
    } catch (const std::exception& e) {
        handleException(e, "显示游戏结果");
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    try {
        if (!event) {
            return;
        }

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
                safeUpdateManualDisplay();
            }
        }

        QMainWindow::keyPressEvent(event);

    } catch (const std::exception& e) {
        handleException(e, "处理按键事件");
    }
}

void MainWindow::showNextPathStep() {
    try {
        if (pathIndex >= static_cast<int>(autoPath.size())) {
            if (pathTimer) {
                pathTimer->stop();
            }
            showAutoResults();
            return;
        }

        if (!mapModel) {
            throw MainWindowException("地图模型未初始化");
        }

        // 创建当前路径（到目前为止的所有步骤）
        std::vector<QPoint> currentPath(autoPath.begin(), autoPath.begin() + pathIndex + 1);
        mapModel->setAutoPath(currentPath);

        pathIndex++;

    } catch (const std::exception& e) {
        handleException(e, "显示路径步骤");
        if (pathTimer) {
            pathTimer->stop();
        }
    }
}

void MainWindow::clearPathDisplay() {
    try {
        if (pathTimer) {
            pathTimer->stop();
        }
        if (mapModel) {
            mapModel->clearPaths();
        }
    } catch (const std::exception& e) {
        qDebug() << "clearPathDisplay error:" << e.what();
        // 不显示错误对话框，这是内部清理操作
    }
}

void MainWindow::showAutoResults() {
    try {
        std::ostringstream info;
        info << "✅ 自动模式路径显示完成!\n";
        info << "🔶 橙色背景: 最优路径\n";
        info << "骑士成功找到拯救公主的最佳路线!";

        if (infoText) {
            infoText->setText(QString::fromStdString(info.str()));
        }

        if (startBtn) {
            startBtn->setEnabled(true);
        }

    } catch (const std::exception& e) {
        handleException(e, "显示自动模式结果");
    }
}

void MainWindow::showTableWindow() {
    try {
        safeShowTableWindow();
    } catch (const std::exception& e) {
        handleException(e, "显示表格窗口");
    }
}

void MainWindow::safeShowTableWindow() {
    // 关闭之前的表格窗口
    if (tableWindow) {
        tableWindow->close();
        tableWindow = nullptr;
    }

    // 创建新的表格窗口
    tableWindow = new MapTableWindow(&dungeon, this);
    if (!tableWindow) {
        throw MainWindowException("创建表格窗口失败");
    }

    tableWindow->show();
}

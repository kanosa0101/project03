#include "maptablewindow.h"
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>

MapTableWindow::MapTableWindow(const Dungeon* dungeon, QWidget *parent)
    : QDialog(parent), m_dungeon(dungeon), m_minHealth(0) {
    if (m_dungeon) {
        m_minHealth = const_cast<Dungeon*>(m_dungeon)->calculateMinHealth();
        m_optimalPath = const_cast<Dungeon*>(m_dungeon)->getOptimalPath();  // 新增：获取最优路径
    }
    setupUI();
    populateTable();
    setupTableStyle();
}

void MapTableWindow::setupUI() {
    setWindowTitle(QString("地图表格 - %1×%2").arg(m_dungeon->getRows()).arg(m_dungeon->getCols()));
    setModal(false); // 允许同时打开多个窗口

    // 设置窗口大小
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int windowWidth = qMin(1200, static_cast<int>(screenGeometry.width() * 0.8));
    int windowHeight = qMin(800, static_cast<int>(screenGeometry.height() * 0.8));
    resize(windowWidth, windowHeight);

    // 居中显示
    move((screenGeometry.width() - windowWidth) / 2,
         (screenGeometry.height() - windowHeight) / 2);

    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 信息标签
    m_infoLabel = new QLabel();
    m_infoLabel->setText(QString("地图尺寸: %1×%2 | 最小初始健康值: %3")
                             .arg(m_dungeon->getRows())
                             .arg(m_dungeon->getCols())
                             .arg(m_minHealth));
    m_infoLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2C3E50; padding: 10px;");
    m_infoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_infoLabel);

    // 创建表格
    m_tableWidget = new QTableWidget(m_dungeon->getRows(), m_dungeon->getCols(), this);
    m_tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(m_tableWidget, 1);

    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_exportBtn = new QPushButton("导出为CSV");
    m_exportBtn->setStyleSheet("background-color: #3498DB; color: white; padding: 8px 16px; border: none; border-radius: 4px; font-size: 14px;");
    buttonLayout->addWidget(m_exportBtn);

    buttonLayout->addStretch();

    m_closeBtn = new QPushButton("关闭");
    m_closeBtn->setStyleSheet("background-color: #E74C3C; color: white; padding: 8px 16px; border: none; border-radius: 4px; font-size: 14px;");
    buttonLayout->addWidget(m_closeBtn);

    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(m_exportBtn, &QPushButton::clicked, this, &MapTableWindow::exportToFile);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

// void MapTableWindow::populateTable() {
//     if (!m_dungeon) return;

//     const auto& map = m_dungeon->getMap();

//     // 设置行列标题
//     for (int i = 0; i < m_dungeon->getRows(); ++i) {
//         m_tableWidget->setVerticalHeaderItem(i, new QTableWidgetItem(QString::number(i)));
//     }
//     for (int j = 0; j < m_dungeon->getCols(); ++j) {
//         m_tableWidget->setHorizontalHeaderItem(j, new QTableWidgetItem(QString::number(j)));
//     }

//     // 填充数据
//     for (int i = 0; i < m_dungeon->getRows(); ++i) {
//         for (int j = 0; j < m_dungeon->getCols(); ++j) {
//             QTableWidgetItem* item = new QTableWidgetItem(QString::number(map[i][j]));

//             // 设置文本居中
//             item->setTextAlignment(Qt::AlignCenter);

//             // 设置字体
//             QFont font = item->font();
//             font.setBold(true);
//             font.setPointSize(10);
//             item->setFont(font);

//             // 设置颜色
//             item->setBackground(QBrush(getCellColor(map[i][j])));
//             item->setForeground(QBrush(Qt::white));

//             // 起点和终点特殊标记
//             if ((i == 0 && j == 0) || (i == m_dungeon->getRows()-1 && j == m_dungeon->getCols()-1)) {
//                 // 加粗边框表示起点和终点
//                 font.setPointSize(12);
//                 item->setFont(font);
//             }

//             m_tableWidget->setItem(i, j, item);
//         }
//     }
// }

void MapTableWindow::populateTable() {
    if (!m_dungeon) return;

    const auto& map = m_dungeon->getMap();

    // 设置行列标题
    for (int i = 0; i < m_dungeon->getRows(); ++i) {
        m_tableWidget->setVerticalHeaderItem(i, new QTableWidgetItem(QString::number(i)));
    }
    for (int j = 0; j < m_dungeon->getCols(); ++j) {
        m_tableWidget->setHorizontalHeaderItem(j, new QTableWidgetItem(QString::number(j)));
    }

    // 填充数据
    for (int i = 0; i < m_dungeon->getRows(); ++i) {
        for (int j = 0; j < m_dungeon->getCols(); ++j) {
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(map[i][j]));

            // 设置文本居中
            item->setTextAlignment(Qt::AlignCenter);

            // 设置字体
            QFont font = item->font();
            font.setBold(true);
            font.setPointSize(10);
            item->setFont(font);

            // 检查是否在最优路径中
            bool inOptimalPath = false;
            for (const auto& point : m_optimalPath) {
                if (point.x() == j && point.y() == i) {
                    inOptimalPath = true;
                    break;
                }
            }

            // 设置颜色
            if (inOptimalPath) {
                item->setBackground(QBrush(QColor("#F39C12")));  // 新增：橙色表示最优路径
            } else {
                item->setBackground(QBrush(getCellColor(map[i][j])));
            }
            item->setForeground(QBrush(Qt::white));

            // 起点和终点特殊标记
            if ((i == 0 && j == 0) || (i == m_dungeon->getRows()-1 && j == m_dungeon->getCols()-1)) {
                // 加粗边框表示起点和终点
                font.setPointSize(12);
                item->setFont(font);
            }

            m_tableWidget->setItem(i, j, item);
        }
    }
}

void MapTableWindow::setupTableStyle() {
    // 设置表格样式
    m_tableWidget->setAlternatingRowColors(false);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // 设置表格网格
    m_tableWidget->setShowGrid(true);
    m_tableWidget->setGridStyle(Qt::SolidLine);
    m_tableWidget->setStyleSheet("QTableWidget { gridline-color: #34495E; }");

    // 自适应列宽
    int availableWidth = width() - 100; // 减去边距和滚动条
    int cellWidth = availableWidth / m_dungeon->getCols();
    cellWidth = qMax(cellWidth, 40); // 最小宽度40
    cellWidth = qMin(cellWidth, 80); // 最大宽度80

    for (int j = 0; j < m_dungeon->getCols(); ++j) {
        m_tableWidget->setColumnWidth(j, cellWidth);
    }

    // 自适应行高
    int availableHeight = height() - 150; // 减去标签和按钮的高度
    int cellHeight = availableHeight / m_dungeon->getRows();
    cellHeight = qMax(cellHeight, 30); // 最小高度30
    cellHeight = qMin(cellHeight, 60); // 最大高度60

    for (int i = 0; i < m_dungeon->getRows(); ++i) {
        m_tableWidget->setRowHeight(i, cellHeight);
    }

    // 设置头部样式
    m_tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #34495E; color: white; padding: 4px; }");
    m_tableWidget->verticalHeader()->setStyleSheet("QHeaderView::section { background-color: #34495E; color: white; padding: 4px; }");
}

QColor MapTableWindow::getCellColor(int value) const {
    if (value > 0) {
        return QColor("#2ECC71"); // 绿色增益
    } else if (value < 0) {
        return QColor("#E74C3C"); // 红色伤害
    } else {
        return QColor("#95A5A6"); // 灰色中性
    }
}

void MapTableWindow::exportToFile() {
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "导出地图数据",
                                                    QString("map_%1x%2.csv").arg(m_dungeon->getRows()).arg(m_dungeon->getCols()),
                                                    "CSV Files (*.csv)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "导出失败", "无法创建文件！");
        return;
    }

    QTextStream out(&file);

    // 写入标题信息
    out << "# 地图数据\n";
    out << "# 尺寸: " << m_dungeon->getRows() << "×" << m_dungeon->getCols() << "\n";
    out << "# 最小初始健康值: " << m_minHealth << "\n";
    out << "# 起点: (0,0), 终点: (" << (m_dungeon->getRows()-1) << "," << (m_dungeon->getCols()-1) << ")\n";
    out << "\n";

    // 写入列标题
    for (int j = 0; j < m_dungeon->getCols(); ++j) {
        if (j > 0) out << ",";
        out << j;
    }
    out << "\n";

    // 写入数据
    const auto& map = m_dungeon->getMap();
    for (int i = 0; i < m_dungeon->getRows(); ++i) {
        for (int j = 0; j < m_dungeon->getCols(); ++j) {
            if (j > 0) out << ",";
            out << map[i][j];
        }
        out << "\n";
    }

    file.close();
    QMessageBox::information(this, "导出成功", QString("地图数据已导出到:\n%1").arg(fileName));
}

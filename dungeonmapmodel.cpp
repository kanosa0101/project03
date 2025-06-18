#include "dungeonmapmodel.h"
#include <QFont>
#include <QBrush>
#include <QDebug>

namespace DungeonColors {
const QColor DefaultGray(0x95, 0xA5, 0xA6);    // 默认灰色 #95A5A6
const QColor PathOrange(0xF3, 0x9C, 0x12);     // 路径橙色 #F39C12
const QColor PositiveGreen(0x2E, 0xCC, 0x71);  // 绿色增益 #2ECC71
const QColor NegativeRed(0xE7, 0x4C, 0x3C);    // 红色伤害 #E74C3C
const QColor StartEndBlue(0x34, 0x98, 0xDB);   // 起点/终点蓝 #3498DB
const QColor BorderBlack(0x00, 0x00, 0x00);    // 边框黑色 #000000
}

DungeonMapModel::DungeonMapModel(QObject *parent)
    : QAbstractTableModel(parent), m_dungeon(nullptr) {
}

void DungeonMapModel::validateDungeon() const {
    if (!m_dungeon) {
        return; // 允许空dungeon，返回0行0列
    }

    if (m_dungeon->getRows() <= 0 || m_dungeon->getCols() <= 0) {
        throw MapModelException("地图尺寸无效");
    }
}

void DungeonMapModel::validateIndex(const QModelIndex& index) const {
    if (!index.isValid()) {
        throw MapModelException("无效的模型索引");
    }

    if (!m_dungeon) {
        throw MapModelException("地下城对象为空");
    }

    int row = index.row();
    int col = index.column();

    if (row < 0 || row >= m_dungeon->getRows() || col < 0 || col >= m_dungeon->getCols()) {
        throw MapModelException("索引超出范围");
    }
}

int DungeonMapModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    try {
        validateDungeon();
        return m_dungeon ? m_dungeon->getRows() : 0;
    } catch (const std::exception& e) {
        qDebug() << "rowCount error:" << e.what();
        return 0;
    }
}

int DungeonMapModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    try {
        validateDungeon();
        return m_dungeon ? m_dungeon->getCols() : 0;
    } catch (const std::exception& e) {
        qDebug() << "columnCount error:" << e.what();
        return 0;
    }
}

QVariant DungeonMapModel::data(const QModelIndex &index, int role) const {
    try {
        if (!m_dungeon || !index.isValid()) {
            return QVariant();
        }

        validateIndex(index);

        int row = index.row();
        int col = index.column();

        const auto& map = m_dungeon->getMap();

        // 额外的安全检查
        if (row >= static_cast<int>(map.size()) || col >= static_cast<int>(map[row].size())) {
            throw MapModelException("访问地图数据时索引越界");
        }

        switch (role) {
        case Qt::DisplayRole:
            return QString::number(map[row][col]);

        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;

        case Qt::FontRole: {
            QFont font;
            font.setBold(true);
            font.setPointSize(12);
            return font;
        }

        case Qt::ForegroundRole:
            return QColor(Qt::white);

        case Qt::BackgroundRole:
            return QBrush(getBackgroundColor(row, col));

        case Qt::UserRole: // 用于边框颜色
            return getBorderColor(row, col);

        default:
            return QVariant();
        }

    } catch (const MapModelException& e) {
        qDebug() << "Model data error:" << e.what();
        return QVariant();
    } catch (const std::exception& e) {
        qDebug() << "Model data unexpected error:" << e.what();
        return QVariant();
    }
}

QVariant DungeonMapModel::headerData(int section, Qt::Orientation orientation, int role) const {
    try {
        if (role == Qt::DisplayRole) {
            return QString::number(section);
        }
        return QVariant();
    } catch (const std::exception& e) {
        qDebug() << "headerData error:" << e.what();
        return QVariant();
    }
}

void DungeonMapModel::setDungeon(const Dungeon* dungeon) {
    try {
        beginResetModel();
        m_dungeon = dungeon;

        if (m_dungeon) {
            validateDungeon();
        }

        endResetModel();

    } catch (const std::exception& e) {
        qDebug() << "setDungeon error:" << e.what();
        m_dungeon = nullptr;
        endResetModel();
    }
}

void DungeonMapModel::setPlayerPath(const std::vector<QPoint>& path) {
    try {
        m_playerPath = path;

        if (m_dungeon && m_dungeon->getRows() > 0 && m_dungeon->getCols() > 0) {
            emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
        }

    } catch (const std::exception& e) {
        qDebug() << "setPlayerPath error:" << e.what();
        m_playerPath.clear();
    }
}

void DungeonMapModel::setAutoPath(const std::vector<QPoint>& path) {
    try {
        m_autoPath = path;

        if (m_dungeon && m_dungeon->getRows() > 0 && m_dungeon->getCols() > 0) {
            emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
        }

    } catch (const std::exception& e) {
        qDebug() << "setAutoPath error:" << e.what();
        m_autoPath.clear();
    }
}

void DungeonMapModel::clearPaths() {
    try {
        m_playerPath.clear();
        m_autoPath.clear();

        if (m_dungeon && m_dungeon->getRows() > 0 && m_dungeon->getCols() > 0) {
            emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
        }

    } catch (const std::exception& e) {
        qDebug() << "clearPaths error:" << e.what();
    }
}

bool DungeonMapModel::isInPath(int row, int col, const std::vector<QPoint>& path) const {
    try {
        for (const auto& point : path) {
            if (point.x() == col && point.y() == row) {
                return true;
            }
        }
        return false;

    } catch (const std::exception& e) {
        qDebug() << "isInPath error:" << e.what();
        return false;
    }
}

QColor DungeonMapModel::getBackgroundColor(int row, int col) const {
    try {
        if (!m_dungeon) {
            return DungeonColors::DefaultGray;
        }

        // 检查是否在路径中
        if (isInPath(row, col, m_playerPath) || isInPath(row, col, m_autoPath)) {
            return DungeonColors::PathOrange;
        }

        const auto& map = m_dungeon->getMap();

        // 边界检查
        if (row >= static_cast<int>(map.size()) || col >= static_cast<int>(map[row].size())) {
            return DungeonColors::DefaultGray;
        }

        int value = map[row][col];

        if (value > 0) {
            return DungeonColors::PositiveGreen;
        } else if (value < 0) {
            return DungeonColors::NegativeRed;
        } else {
            return DungeonColors::DefaultGray;
        }

    } catch (const std::exception& e) {
        qDebug() << "getBackgroundColor error:" << e.what();
        return DungeonColors::DefaultGray;
    }
}

QColor DungeonMapModel::getBorderColor(int row, int col) const {
    try {
        if (!m_dungeon) {
            return DungeonColors::BorderBlack;
        }

        // 起点和终点
        if ((row == 0 && col == 0) ||
            (row == m_dungeon->getRows()-1 && col == m_dungeon->getCols()-1)) {
            return DungeonColors::StartEndBlue;
        }

        return DungeonColors::BorderBlack;

    } catch (const std::exception& e) {
        qDebug() << "getBorderColor error:" << e.what();
        return DungeonColors::BorderBlack;
    }
}

#include "dungeonmapmodel.h"
#include <QFont>
#include <QBrush>

DungeonMapModel::DungeonMapModel(QObject *parent)
    : QAbstractTableModel(parent), m_dungeon(nullptr) {
}

int DungeonMapModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_dungeon ? m_dungeon->getRows() : 0;
}

int DungeonMapModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_dungeon ? m_dungeon->getCols() : 0;
}

QVariant DungeonMapModel::data(const QModelIndex &index, int role) const {
    if (!m_dungeon || !index.isValid())
        return QVariant();

    int row = index.row();
    int col = index.column();

    if (row >= m_dungeon->getRows() || col >= m_dungeon->getCols())
        return QVariant();

    const auto& map = m_dungeon->getMap();

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
}

QVariant DungeonMapModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        return QString::number(section);
    }
    return QVariant();
}

void DungeonMapModel::setDungeon(const Dungeon* dungeon) {
    beginResetModel();
    m_dungeon = dungeon;
    endResetModel();
}

void DungeonMapModel::setPlayerPath(const std::vector<QPoint>& path) {
    m_playerPath = path;
    emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
}

void DungeonMapModel::setAutoPath(const std::vector<QPoint>& path) {
    m_autoPath = path;
    emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
}

void DungeonMapModel::clearPaths() {
    m_playerPath.clear();
    m_autoPath.clear();
    emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
}

bool DungeonMapModel::isInPath(int row, int col, const std::vector<QPoint>& path) const {
    for (const auto& point : path) {
        if (point.x() == col && point.y() == row) {
            return true;
        }
    }
    return false;
}

QColor DungeonMapModel::getBackgroundColor(int row, int col) const {
    // 检查是否在路径中
    if (isInPath(row, col, m_playerPath) || isInPath(row, col, m_autoPath)) {
        return QColor("#F39C12"); // 橙色路径
    }

    const auto& map = m_dungeon->getMap();
    int value = map[row][col];

    if (value > 0) {
        return QColor("#2ECC71"); // 绿色增益
    } else if (value < 0) {
        return QColor("#E74C3C"); // 红色伤害
    } else {
        return QColor("#95A5A6"); // 灰色中性
    }
}

QColor DungeonMapModel::getBorderColor(int row, int col) const {
    // 起点
    if (row == 0 && col == 0) {
        return QColor("#3498DB"); // 蓝色
    }
    // 终点
    if (m_dungeon && row == m_dungeon->getRows()-1 && col == m_dungeon->getCols()-1) {
        return QColor("#3498DB"); // 蓝色
    }

    return QColor("#000000"); // 默认黑色
}

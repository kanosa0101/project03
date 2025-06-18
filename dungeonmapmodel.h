#ifndef DUNGEONMAPMODEL_H
#define DUNGEONMAPMODEL_H

#include <QAbstractTableModel>
#include <QColor>
#include <QPoint>
#include <vector>
#include <stdexcept>
#include "dungeon.h"



class MapModelException : public std::runtime_error {
public:
    explicit MapModelException(const std::string& message) : std::runtime_error(message) {}
};

class DungeonMapModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit DungeonMapModel(QObject *parent = nullptr);

    // QAbstractTableModel接口
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // 设置数据
    void setDungeon(const Dungeon* dungeon);
    void setPlayerPath(const std::vector<QPoint>& path);
    void setAutoPath(const std::vector<QPoint>& path);
    void clearPaths();

private:
    const Dungeon* m_dungeon;
    std::vector<QPoint> m_playerPath;
    std::vector<QPoint> m_autoPath;

    bool isInPath(int row, int col, const std::vector<QPoint>& path) const;
    QColor getBackgroundColor(int row, int col) const;
    QColor getBorderColor(int row, int col) const;
    void validateIndex(const QModelIndex& index) const;
    void validateDungeon() const;
};

#endif // DUNGEONMAPMODEL_H

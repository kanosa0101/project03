#ifndef DUNGEONTABLEVIEW_H
#define DUNGEONTABLEVIEW_H

#include <QTableView>
#include <QStyledItemDelegate>
#include <QPainter>

class DungeonItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit DungeonItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

class DungeonTableView : public QTableView {
    Q_OBJECT

public:
    explicit DungeonTableView(QWidget *parent = nullptr);

    void updateCellSize();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    DungeonItemDelegate *m_delegate;
};

#endif // DUNGEONTABLEVIEW_H

#ifndef DUNGEONTABLEVIEW_H
#define DUNGEONTABLEVIEW_H

#include <QTableView>
#include <QStyledItemDelegate>
#include <QPainter>
#include <stdexcept>

class DelegateException : public std::runtime_error {
public:
    explicit DelegateException(const std::string& message) : std::runtime_error(message) {}
};

class TableViewException : public std::runtime_error {
public:
    explicit TableViewException(const std::string& message) : std::runtime_error(message) {}
};

class DungeonItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit DungeonItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    void safePaint(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
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
    void safeUpdateCellSize();
};

#endif // DUNGEONTABLEVIEW_H

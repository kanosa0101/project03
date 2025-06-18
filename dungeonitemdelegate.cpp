#include "dungeontableview.h"
#include <QPainter>
#include <QResizeEvent>
#include <QHeaderView>

DungeonItemDelegate::DungeonItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {
}

void DungeonItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const {
    painter->save();

    // 获取数据
    QString text = index.data(Qt::DisplayRole).toString();
    QColor bgColor = index.data(Qt::BackgroundRole).value<QBrush>().color();
    QColor borderColor = index.data(Qt::UserRole).value<QColor>();
    QFont font = index.data(Qt::FontRole).value<QFont>();

    // 绘制背景
    painter->fillRect(option.rect, bgColor);

    // 绘制边框
    QPen borderPen(borderColor, 2);
    painter->setPen(borderPen);
    painter->drawRect(option.rect.adjusted(1, 1, -1, -1));

    // 绘制文字
    painter->setPen(QPen(Qt::white));
    painter->setFont(font);
    painter->drawText(option.rect, Qt::AlignCenter, text);

    painter->restore();
}

QSize DungeonItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const {
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(40, 40); // 最小尺寸
}

DungeonTableView::DungeonTableView(QWidget *parent)
    : QTableView(parent) {

    m_delegate = new DungeonItemDelegate(this);
    setItemDelegate(m_delegate);

    // 设置表格属性
    setSelectionMode(QAbstractItemView::NoSelection);
    setFocusPolicy(Qt::NoFocus);
    setShowGrid(false);

    // 隐藏表头
    horizontalHeader()->setVisible(false);
    verticalHeader()->setVisible(false);

    // 禁用滚动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void DungeonTableView::updateCellSize() {
    if (!model() || model()->rowCount() == 0 || model()->columnCount() == 0)
        return;

    int rows = model()->rowCount();
    int cols = model()->columnCount();

    // 计算合适的单元格大小
    int availableWidth = viewport()->width();
    int availableHeight = viewport()->height();

    int cellWidth = availableWidth / cols;
    int cellHeight = availableHeight / rows;

    // 保持正方形，取较小值
    int cellSize = qMin(cellWidth, cellHeight);
    cellSize = qMax(cellSize, 20); // 最小20像素
    cellSize = qMin(cellSize, 80); // 最大80像素

    // 设置所有行列的大小
    for (int i = 0; i < rows; ++i) {
        setRowHeight(i, cellSize);
    }
    for (int j = 0; j < cols; ++j) {
        setColumnWidth(j, cellSize);
    }
}

void DungeonTableView::resizeEvent(QResizeEvent *event) {
    QTableView::resizeEvent(event);
    updateCellSize();
}

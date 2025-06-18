#include "dungeontableview.h"
#include <QPainter>
#include <QPen>
#include <QResizeEvent>
#include <QHeaderView>
#include <QDebug>

DungeonItemDelegate::DungeonItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {
}

void DungeonItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const {
    try {
        safePaint(painter, option, index);
    } catch (const std::exception& e) {
        qDebug() << "Delegate paint error:" << e.what();
        // 绘制默认样式作为后备
        QStyledItemDelegate::paint(painter, option, index);
    }
}

void DungeonItemDelegate::safePaint(QPainter *painter, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const {
    if (!painter) {
        throw DelegateException("画笔为空");
    }

    if (!index.isValid()) {
        throw DelegateException("索引无效");
    }

    // 绘制背景
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->save();

    try {
        // 绘制背景色
        QVariant bgData = index.data(Qt::BackgroundRole);
        if (bgData.isValid()) {
            QBrush bgBrush = bgData.value<QBrush>();
            painter->fillRect(opt.rect, bgBrush);
        }

        // 绘制文本
        QVariant textData = index.data(Qt::DisplayRole);
        if (textData.isValid()) {
            QString text = textData.toString();
            QVariant fontData = index.data(Qt::FontRole);
            QFont font = fontData.isValid() ? fontData.value<QFont>() : opt.font;

            QVariant fgData = index.data(Qt::ForegroundRole);
            QColor textColor = fgData.isValid() ? fgData.value<QColor>() : Qt::black;

            painter->setPen(textColor);
            painter->setFont(font);
            painter->drawText(opt.rect, Qt::AlignCenter, text);
        }

        // 绘制边框
        QVariant borderData = index.data(Qt::UserRole);
        if (borderData.isValid()) {
            QColor borderColor = borderData.value<QColor>();
            QPen pen(borderColor, 3);
            painter->setPen(pen);
            painter->drawRect(opt.rect.adjusted(1, 1, -1, -1));
        }

    } catch (const std::exception& e) {
        painter->restore();
        throw DelegateException(std::string("绘制过程中发生错误: ") + e.what());
    }

    painter->restore();
}

QSize DungeonItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const {
    try {
        Q_UNUSED(option)
        Q_UNUSED(index)
        return QSize(50, 50); // 默认单元格大小
    } catch (const std::exception& e) {
        qDebug() << "Delegate sizeHint error:" << e.what();
        return QSize(50, 50); // 返回默认大小
    }
}

DungeonTableView::DungeonTableView(QWidget *parent)
    : QTableView(parent), m_delegate(nullptr) {
    try {
        m_delegate = new DungeonItemDelegate(this);
        setItemDelegate(m_delegate);

        // 设置表格属性
        setShowGrid(false);
        setSelectionMode(QAbstractItemView::NoSelection);
        horizontalHeader()->setVisible(true);
        verticalHeader()->setVisible(true);

        // 设置头部样式
        horizontalHeader()->setDefaultSectionSize(50);
        verticalHeader()->setDefaultSectionSize(50);
        horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    } catch (const std::exception& e) {
        qDebug() << "DungeonTableView constructor error:" << e.what();
        // 确保m_delegate不为空
        if (!m_delegate) {
            m_delegate = new DungeonItemDelegate(this);
            setItemDelegate(m_delegate);
        }
    }
}

void DungeonTableView::updateCellSize() {
    try {
        safeUpdateCellSize();
    } catch (const std::exception& e) {
        qDebug() << "updateCellSize error:" << e.what();
        // 设置默认大小作为后备
        horizontalHeader()->setDefaultSectionSize(50);
        verticalHeader()->setDefaultSectionSize(50);
    }
}

void DungeonTableView::safeUpdateCellSize() {
    if (!model()) {
        return; // 没有模型时直接返回
    }

    int rows = model()->rowCount();
    int cols = model()->columnCount();

    if (rows <= 0 || cols <= 0) {
        return; // 空模型时直接返回
    }

    // 检查视口是否有效
    if (!viewport()) {
        throw TableViewException("视口为空");
    }

    // 计算可用空间
    int availableWidth = viewport()->width() - 20; // 留一些边距
    int availableHeight = viewport()->height() - 20;

    if (availableWidth <= 0 || availableHeight <= 0) {
        return; // 窗口太小时不更新
    }

    // 计算单元格大小
    int cellWidth = availableWidth / cols;
    int cellHeight = availableHeight / rows;

    // 设置最小和最大尺寸
    cellWidth = qBound(30, cellWidth, 80);
    cellHeight = qBound(30, cellHeight, 80);

    // 保持正方形
    int cellSize = qMin(cellWidth, cellHeight);

    if (cellSize <= 0) {
        throw TableViewException("计算出的单元格大小无效");
    }

    // 应用尺寸
    horizontalHeader()->setDefaultSectionSize(cellSize);
    verticalHeader()->setDefaultSectionSize(cellSize);

    // 安全地设置列宽和行高
    try {
        for (int i = 0; i < cols && i < 1000; ++i) { // 限制最大列数
            setColumnWidth(i, cellSize);
        }
        for (int i = 0; i < rows && i < 1000; ++i) { // 限制最大行数
            setRowHeight(i, cellSize);
        }
    } catch (const std::exception& e) {
        throw TableViewException(std::string("设置单元格尺寸时发生错误: ") + e.what());
    }
}

void DungeonTableView::resizeEvent(QResizeEvent *event) {
    try {
        QTableView::resizeEvent(event);
        updateCellSize();
    } catch (const std::exception& e) {
        qDebug() << "resizeEvent error:" << e.what();
        // 调用基类的resizeEvent作为后备
        QTableView::resizeEvent(event);
    }
}

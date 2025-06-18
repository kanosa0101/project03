#ifndef MAPTABLEWINDOW_H
#define MAPTABLEWINDOW_H

#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QFont>
#include "dungeon.h"

class MapTableWindow : public QDialog {
    Q_OBJECT

public:
    explicit MapTableWindow(const Dungeon* dungeon, QWidget *parent = nullptr);

private slots:
    void exportToFile();

private:
    void setupUI();
    void populateTable();
    void setupTableStyle();
    QColor getCellColor(int value) const;

    const Dungeon* m_dungeon;
    QTableWidget* m_tableWidget;
    QLabel* m_infoLabel;
    QPushButton* m_exportBtn;
    QPushButton* m_closeBtn;
    int m_minHealth;

    std::vector<QPoint> m_optimalPath;
};

#endif // MAPTABLEWINDOW_H

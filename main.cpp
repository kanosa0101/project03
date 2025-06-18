#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 设置应用程序图标和信息
    app.setApplicationName("地下城游戏");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("Dungeon Game Studio");

    MainWindow window;
    window.show();

    return app.exec();
}

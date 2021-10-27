#include "ecs.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ECS w{10};
    w.connectElevators(4);
    w.show();
    return a.exec();
}

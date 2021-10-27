#ifndef ECS_H
#include "elevator.h"
#include "floor.h"
#define ECS_H
#include <QMainWindow>
#include <QStackedWidget>
#include <QThreadPool>
QT_BEGIN_NAMESPACE
namespace Ui { class ECS; }
QT_END_NAMESPACE

class ECS : public QMainWindow
{
    Q_OBJECT

public:
    ECS(int numOfFloors, QWidget *parent = nullptr);
    ~ECS();
    void connectElevator();
    void connectElevators(int n);
    void connectFloor();
    void connectFloors(int n);
    void showFloor(int id);
    void showElevator(int id);
    void getElevator(int id);

public slots:
    void requestFloor(void);
    void requestElevator(void);
    void receiveArrivalNotice(int floor);

private:
    QVector<Elevator *> elevators;
    QVector<Floor *> floors;
    QVector<QVector<int>> requestedFloors; //tracks the requested floor handled directly by the elevator
    QVector<bool> requestedElevator; //tracks the floors that have requested an elevator
    Ui::ECS *ui;
    QTabWidget *elevatorsTab;
    void addElevatorToGUI(Elevator *e);
    void writeToConsole(const QString &);
    void commandElevator(Elevator *e);
};
#endif // ECS_H

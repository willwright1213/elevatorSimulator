#ifndef ECS_H
#include "defs.h"
#include "elevator.h"
#include "floor.h"
#include "building.h"
#define ECS_H
#include <QMainWindow>
#include <QStackedWidget>
#include <QThreadPool>
#include <QMutex>
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
    void clearAllRequests();
    bool hasRequests(Elevator *e);
    void changeElevatorDirection(Elevator *e, Direction d);
    void changeElevatorStatus(Elevator *e, Status s);
    void disableOpenCloseButton(Elevator *e);
    void enableOpenCloseButton(Elevator *e);
    void startThreadCommand(Elevator *e, QString const & command);
    void statusUpdated(Elevator *e);
    void pingElevator(Elevator *e);

public slots:
    void requestFloor(int);
    void requestElevator(void);
    void receiveArrivalNotice(int floor);
    void openOrCloseRequest(char doorSignal);
    void receiveAlarmSignal(int signalCode);


private:
    QVector<Elevator *> elevators;
    QVector<Floor *> floors;
    Building *building;
    QVector<QVector<int>> requestedFloors; //tracks the requested floor handled directly by the elevator
    Ui::ECS *ui;
    QTabWidget *elevatorsTab;
    QTabWidget *floorsTab;
    void writeToConsole(const QString &);
    void commandElevator(Elevator *e);
};
#endif // ECS_H

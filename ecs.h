#ifndef ECS_H
#define ECS_H
#include <QMainWindow>
#include <QStackedWidget>
#include <QComboBox>
#include <QThreadPool>
#include "defs.h"
#include "elevator.h"
#include "floor.h"
QT_BEGIN_NAMESPACE
namespace Ui { class ECS; }
QT_END_NAMESPACE

class ECS : public QMainWindow
{
    Q_OBJECT

public:
    ECS(int numOfFloors, int numOfElevators, QWidget *parent = nullptr);
    ~ECS();
    void connectElevator();
    void connectElevators(int n);
    void connectFloor();
    void connectFloors(int n);
    void getElevator(int id);
    void clearAllRequests();
    bool hasRequests(Elevator *e);
    void changeElevatorDirection(Elevator *e, Direction d);
    void changeElevatorStatus(Elevator *e, Status s);
    void disableOpenCloseButton(Elevator *e);
    void enableOpenCloseButton(Elevator *e);
    void startThreadCommand(Elevator *e, QString const & command);


public slots:
    void requestFloor(int);
    void requestElevator(int floorID, Direction direction);
    void receiveArrivalNotice(int floor);
    void openOrCloseRequest(char doorSignal);
    void receiveAlarmSignal(int signalCode, int code);
    void selectElevator(int index);
    void selectFloor(int index);
    void buildingFireAlarm();
    void powerOut();
    void engageWithHelp();


private:
    QVector<Elevator *> elevators;
    QVector<Floor *> floors;
    QVector<QVector<int>> requestedFloors; //tracks the requested floor handled directly by the elevator
    Elevator *currentElevator;
    Floor *currentFloor;
    Ui::ECS *ui;
    void writeToConsole(const QString &);
    void commandElevator(Elevator *e);
};
#endif // ECS_H

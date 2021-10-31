#ifndef ELEVATOR_H
#include "elevatorpanel.h"
#define ELEVATOR_H


#include <QLineEdit>
#include <QLabel>
#include <QTime>
#include <QRunnable>
#include <QTextBrowser>
#include <QSpinBox>
#include "elevatorcomponentfactory.h"



class Elevator : public ElevatorComponentFactory
{
    Q_OBJECT
public:
    Elevator(int, int, QWidget *parent = nullptr);
    ~Elevator();
    bool ECSConnected = false;
    bool doorIsOpen = false;
    ElevatorPanel *panel;
    QTextBrowser *console;
    QSpinBox *weightBox;
    Direction getDirection() const {return direction;}
    Status getStatus() const {return status;}
    bool isDoorOpen() const { return doorIsOpen;}
    void setDirection(Direction d) {direction = d;}
    void setStatus(Status s) {status = s;}
    int getCurrentFloor() const {return currentFloor;}
    void increaseFloor();
    void decreaseFloor();
    void requestOpenDoor();
    void requestCloseDoor();
    bool IsOpenDoorButtonPressed() const {return openDoorButtonPressed;}
    void updateDisplay(const QString& message);
    void sendFireAlarmSignal();
    void sendOverloadSignal();
    void sendHelpSignal();
    void statusUpdated();
    void pinged();

public slots:
    void writeToConsole(const QString & text);
    void pressOpenDoor();
    void releaseOpenDoor();
    void pressFloorButton();
    void updateWeight(int w);



signals:
    void requestFloor(int floor);
    void signalConsoleWrite(const QString & text);
    void arrivalNotice(int floor);
    void requestOpenOrClose(char openDoorSignal);

private:
    int weight = 0;
    int numOfFloors;
    int currentFloor = 0;
    int closingAttempts = 0;
    Direction direction = UP;
    Status status = IDLE;
    QLineEdit *display, *audioMessage;
    QWidget *elevatorPanel;
    bool bellRinging = false;
    bool openDoorButtonPressed = false;

};

/**
 * @brief The ElevatorThreader class manages requests that would normally block the GUI (elevator travel, opening door sequence, etc)
 */
class ElevatorTask: public QRunnable
{
public:
    ElevatorTask(Elevator *e, const QString & command);
    void moveUp();
    void moveDown();
    void doorMonitor();
    void waitForHelp();
    void run() override;
private:
    Elevator *targetElevator;
    QString command;
};

#endif // ELEVATOR_H


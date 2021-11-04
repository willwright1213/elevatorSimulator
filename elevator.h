#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QTime>
#include <QRunnable>
#include <QTextBrowser>
#include <QSpinBox>
#include <QPushButton>

#include "defs.h"

namespace Ui {
class Elevator;
}


class Elevator : public QWidget
{
    Q_OBJECT
public:
    Elevator(int, int, QWidget *parent = nullptr);
    ~Elevator();
    QVector<QPushButton *> buttons;
    QPushButton *openButton, *closeButton, *helpButton, *fireButton;
    bool ECSConnected = false;
    bool doorIsOpen = false;
    bool doorIsBlocked = false;
    int attempts = 0;
    Direction getDirection() const {return direction;}
    int getID() const {return id;};
    Status getStatus() const {return status;}
    bool isDoorOpen() const { return doorIsOpen;}
    bool IsOpenDoorButtonPressed() const {return openDoorButtonPressed;}
    int getCurrentFloor() const {return currentFloor;}
    int getAlarmCode() const {return alarmCode;}
    void setAlarm(int code);
    void resetAlarm();
    void setDirection(Direction d);
    void setStatus(Status s);
    void increaseFloor();
    void decreaseFloor();
    void requestOpenDoor();
    void requestCloseDoor();
    void updateDisplay(const QString& message);
    void updateAudio(const QString &audio);
    void sendFireAlarmSignal();
    void sendOverloadSignal();
    void sendHelpSignal();

public slots:
    void pressOpenDoor();
    void releaseOpenDoor();
    void pressFloorButton();
    void updateWeight(int w);
    void blockDoor(int arg1);



signals:
    void requestFloor(int floor);
    void signalConsoleWrite(const QString & text);
    void arrivalNotice(int floor);
    void requestOpenOrClose(char openDoorSignal);
    void sendAlarmSignal(int code, int id);

private slots:


private:
    Ui::Elevator *ui;
    int id;
    int weight = 0;
    int currentFloor = 0;
    int closingAttempts = 0;
    int alarmCode = RESET_SIGNAL;
    int numOfFloors;
    Direction direction = UP;
    Status status = IDLE;
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


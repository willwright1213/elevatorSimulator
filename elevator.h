#ifndef ELEVATOR_H
#include "elevatorpanel.h"
#define ELEVATOR_H
#include <QMap>
#include <QLineEdit>
#include <QTime>
#include <QRunnable>
#include "elevatorcomponentfactory.h"

typedef enum {UP, DOWN, IDLE} Direction;


class Elevator : public ElevatorComponentFactory
{
    Q_OBJECT
public:
    Elevator(int, int, QWidget *parent = nullptr);
    ~Elevator();
    void connectPanel(QWidget *);
    int floors() const;
    Direction getDirection();
    void setDirection(Direction);
    int getCurrentFloor() const;
    void updateDisplay(const QString& message);
    void moveUp();
    void setFloor(int floor);
    void moveDown();
    bool isDoorOpen();
    void openDoor() {isOpen = true;}
    void closeDoor() {isOpen = false;}
    ElevatorPanel *panel;

public slots:

signals:
    void arrivalNotice(int floor);

private:
    int currentFloor = 0;
    Direction direction = IDLE;
    QLineEdit *display;
    QWidget *elevatorPanel;
    bool isOpen = false;
    bool bellRinging = false;
    int numOfFloors;

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
    void embark();
    void run() override;
private:
    Elevator *targetElevator;
    QString command;
};

#endif // ELEVATOR_H


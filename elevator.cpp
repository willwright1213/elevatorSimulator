#include "elevator.h"
#include <QDebug>

Elevator::Elevator(int id, int floors, QWidget *parent):ElevatorComponentFactory(id, parent),
    numOfFloors(floors)
{
    panel = new ElevatorPanel(id, 3, floors);
    display = new QLineEdit(this);
    display->setReadOnly(true);
    display->insert("This is a test");
    layout->addWidget(display);
    layout->addWidget(panel);
}

int Elevator::getCurrentFloor() const {
    return currentFloor;
}

void Elevator::setFloor(int floor) {
    currentFloor = floor;
}

int Elevator::floors() const {
    return numOfFloors;
}

Direction Elevator::getDirection() {
    return direction;
}

bool Elevator::isDoorOpen() {
    return isOpen;
}

void Elevator::setDirection(Direction d) {
    direction = d;
}

void Elevator::updateDisplay(const QString &message) {
    display->clear();
    display->insert(message);
}
/** Signals **/


Elevator::~Elevator(){

}

typedef void (*task)(void);

ElevatorTask::ElevatorTask(Elevator *e, const QString & command):targetElevator(e),
    command(command)
{
}

void ElevatorTask::moveUp() {
    while(targetElevator->isDoorOpen());
    if(targetElevator->getCurrentFloor() == targetElevator->floors()) return;
    QTime travelTime = QTime::currentTime().addSecs(2);
    while(QTime::currentTime() < travelTime);
    targetElevator->setFloor(targetElevator->getCurrentFloor() + 1);
    emit targetElevator->arrivalNotice(targetElevator->getCurrentFloor());
}

void ElevatorTask::moveDown() {
    if(targetElevator->getCurrentFloor() == targetElevator->floors()) return;
    QTime travelTime = QTime::currentTime().addSecs(2);
    while(QTime::currentTime() < travelTime);
    targetElevator->setFloor(targetElevator->getCurrentFloor() - 1);
    emit targetElevator->arrivalNotice(targetElevator->getCurrentFloor());
}

void ElevatorTask::embark() {
    targetElevator->openDoor();
    QTime travelTime = QTime::currentTime().addSecs(10);
    while(QTime::currentTime() < travelTime);
    targetElevator->closeDoor();
}

void ElevatorTask::run() {
    if(command == "moveUp")
        moveUp();
    if(command == "moveDown")
        moveDown();
    if(command == "embark")
        embark();
}

#include "elevator.h"
#include <QDebug>

Elevator::Elevator(int id, int floors, QWidget *parent):ElevatorComponentFactory(id, parent),
    numOfFloors(floors)
{
    //Define labels
    QLabel *weightLabel = new QLabel("weight: (x 60kg) ");
    QLabel *displayLabel = new QLabel("Display: ");
    QLabel *audioLabel = new QLabel("Audio: ");
    //QLine edit
    audioMessage = new QLineEdit();
    display = new QLineEdit("Floor: 0");
    display->setReadOnly(true);
    weightBox = new QSpinBox();
    weightBox->setDisabled(true);
    //Add button panel and set things up
    panel = new ElevatorPanel(id, 3, floors);
    console = new QTextBrowser();
    //Adding everything to the layout
    layout->addWidget(displayLabel,0, 0);
    layout->addWidget(display, 0, 1, 1, 5);
    layout->addWidget(audioLabel, 1, 0);
    layout->addWidget(audioMessage, 1, 1, 1, 5);
    layout->addWidget(panel, 2, 2, 1, 2);
    layout->addWidget(weightLabel, 3,0);
    layout->addWidget(weightBox, 3, 1);
    layout->addWidget(console,4, 0, 1, 6);
    //Connections
    for(int i = 0; i < numOfFloors; i++) {
        connect(panel->buttons[i], &QPushButton::released, this, &Elevator::pressFloorButton);
    }
    connect(this, &Elevator::signalConsoleWrite, this, &Elevator::writeToConsole);
    connect(panel->openButton, &QPushButton::pressed, this, &Elevator::pressOpenDoor);
    connect(panel->openButton, &QPushButton::released, this, &Elevator::releaseOpenDoor);
    connect(panel->fireButton, &QPushButton::pressed, this, &Elevator::sendFireAlarmSignal);
    connect(panel->helpButton, &QPushButton::pressed, this, &Elevator::sendHelpSignal);
    connect(weightBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Elevator::updateWeight);
}

void Elevator::updateDisplay(const QString &message) {
    display->clear();
    display->insert(message);
}

void Elevator::increaseFloor() {
    emit arrivalNotice(++currentFloor);
}

void Elevator::decreaseFloor() {
    emit arrivalNotice(--currentFloor);
}

void Elevator::updateWeight(int w) {
    weight = w * 100;
    if(weight >= CAPACITY_LIMIT) {
        sendOverloadSignal();
        alarmSent = OVERLOAD_SIGNAL;
    }
    if(weight < CAPACITY_LIMIT && alarmSent == OVERLOAD_SIGNAL) {
        resetAlarm();
    }
}

void Elevator::writeToConsole(const QString & text) {
    QString fullText = ">> ";
    fullText += text;
    console->append(fullText);
}

//These two functions are the slots when the open button is pressed/released
void Elevator::pressOpenDoor(){
    writeToConsole("Open Door button is pressed");
    requestOpenDoor();
    openDoorButtonPressed = true;
}

void Elevator::releaseOpenDoor(){
    writeToConsole("Open Door button is released");
    openDoorButtonPressed = false;
}

void Elevator::pressFloorButton() {
    QPushButton *b = qobject_cast<QPushButton *>(sender());
    emit requestFloor(b->text().toInt());
}

void Elevator::statusUpdated() {
    if(status == IDLE) requestOpenDoor();
}

/**
 * @brief Elevator::pinged when the ECS pings the elevator, it response with its current floor.
 */
void Elevator::pinged() {
    qDebug() << "pinged";
    emit arrivalNotice(currentFloor);
}

/** Signals **/

/**
 * @brief Elevator::requestOpenDoor asks the ECS to open the door
 */
void Elevator::requestOpenDoor() {
    if(!doorIsOpen) emit requestOpenOrClose(OPEN_DOOR);
}

/**
 * @brief Elevator::requestCloseDoor asks the ECS to close the door
 */
void Elevator::requestCloseDoor() {
    if(doorIsOpen) emit requestOpenOrClose(CLOSE_DOOR);
}

void Elevator::sendFireAlarmSignal() {
   emit sendAlarmSignal(FIRE_SIGNAL);
}

void Elevator::sendOverloadSignal() {
    emit sendAlarmSignal(OVERLOAD_SIGNAL);
}

void Elevator::sendHelpSignal() {
    emit sendAlarmSignal(HELP_SIGNAL);
}

Elevator::~Elevator(){

}

typedef void (*task)(void);

ElevatorTask::ElevatorTask(Elevator *e, const QString & command):targetElevator(e),
    command(command)
{
}

void ElevatorTask::moveUp() {
    while(targetElevator->isDoorOpen());
    QTime travelTime = QTime::currentTime().addSecs(2);
    while(QTime::currentTime() < travelTime) {
        if(!targetElevator->ECSConnected) return;
    }
    targetElevator->increaseFloor();
}

void ElevatorTask::moveDown() {
    while(targetElevator->isDoorOpen());
    QTime travelTime = QTime::currentTime().addSecs(2);
    while(QTime::currentTime() < travelTime) {
        if(!targetElevator->ECSConnected) return;
    }
    targetElevator->decreaseFloor();
}

void ElevatorTask::doorMonitor() {
    while(targetElevator->ECSConnected) {
        while(targetElevator->isDoorOpen()) {
            QTime travelTime = QTime::currentTime().addSecs(10);
            while(QTime::currentTime() < travelTime) {
                if(!targetElevator->ECSConnected) return;
                if (!targetElevator->isDoorOpen()) break;
            }
            while(targetElevator->IsOpenDoorButtonPressed()
                  || targetElevator->getLastAlarm() != -1
                  || targetElevator->getStatus() == IDLE) {
                if(!targetElevator->ECSConnected) return;
            }
            targetElevator->requestCloseDoor();
        }
    }
}

void ElevatorTask::waitForHelp() {
    while(targetElevator->ECSConnected) {
        QTime travelTime = QTime::currentTime().addSecs(10);
        while(QTime::currentTime() < travelTime) {
            if(!targetElevator->ECSConnected) return;
            if(targetElevator->getLastAlarm() != HELP_SIGNAL) return;
        }
        targetElevator->sendHelpSignal(); //send the second help signal
    }
}

void ElevatorTask::run() {
    if(command == "moveUp")
        moveUp();
    if(command == "moveDown")
        moveDown();
    if(command == "doorMonitor")
        doorMonitor();
}

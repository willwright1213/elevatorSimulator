#include "elevator.h"
#include "ui_elevator.h"
#include <QDebug>

Elevator::Elevator(int id, int floors, QWidget *parent):
    id(id),
    QWidget(parent),
    ui(new Ui::Elevator),
    numOfFloors(floors)
{
    ui->setupUi(this);
    ui->elevatorLabel->setText("Elevator " + QString::number(id));
    //QLine edit
    //Add button panel and set things up
    int i;
    QPushButton *b;
    for(i = 0; i < floors; i++) {
        QPushButton *b = new QPushButton(QString::number(i));
        ui->panelLayout->addWidget(b, i/3, i%3);
        connect(b, &QPushButton::released, this, &Elevator::pressFloorButton);
        buttons.append(b);
    }
    openButton = new QPushButton("Open");
    ui->panelLayout->addWidget(openButton, buttons.size()/3, buttons.size()%3);
    buttons.append(openButton);
    connect(openButton, &QPushButton::pressed, this, &Elevator::pressOpenDoor);
    connect(openButton, &QPushButton::released, this, &Elevator::releaseOpenDoor);
    closeButton = new QPushButton("Close");
    ui->panelLayout->addWidget(closeButton, buttons.size()/3, buttons.size()%3);
    buttons.append(closeButton);
    //connect(closeButton, &QPushButton::released, this, &Elevator::pressFloorButton);
    helpButton = new QPushButton("Help");
    ui->panelLayout->addWidget(helpButton, buttons.size()/3, buttons.size()%3);
    buttons.append(closeButton);
    connect(helpButton, &QPushButton::pressed, this, &Elevator::sendHelpSignal);
    fireButton = new QPushButton("Fire");
    ui->panelLayout->addWidget(fireButton, buttons.size()/3, buttons.size()%3);
    buttons.append(fireButton);
    connect(fireButton, &QPushButton::pressed, this, &Elevator::sendFireAlarmSignal);

    connect(ui->weightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Elevator::updateWeight);
    connect(ui->doorBlockingCheck, &QCheckBox::stateChanged, this, &Elevator::blockDoor);
}

void Elevator::updateDisplay(const QString &message) {
    ui->display->clear();
    ui->display->insert(message);
}

void Elevator::updateAudio(const QString &audio) {
    ui->audio->clear();
    ui->audio->insert(audio);
}

void Elevator::increaseFloor() {
    currentFloor++;
    setStatus(STOP);
}

void Elevator::decreaseFloor() {
    currentFloor--;
    setStatus(STOP);
}

void Elevator::updateWeight(int w) {
    weight = w * 100;
    if(weight >= CAPACITY_LIMIT) {
        sendOverloadSignal();
        alarmCode = OVERLOAD_SIGNAL;
    }
    if(weight < CAPACITY_LIMIT && alarmCode == OVERLOAD_SIGNAL) {
        resetAlarm();
    }
}

void Elevator::setAlarm(int code) { alarmCode = code;}

void Elevator::resetAlarm() {
    alarmCode = RESET_SIGNAL;
    ui->audio->clear();
    updateDisplay("Floor: " + QString::number(currentFloor));
}

void Elevator::setDirection(Direction d) { direction = d; }

void Elevator::setStatus(Status s) {
    status = s;
    if(status == STOP) {
        emit arrivalNotice(currentFloor);
        ui->weightSpinBox->setDisabled(true);
    }
    if(status == ARRIVAL) {
        ui->weightSpinBox->setDisabled(false);
        requestOpenDoor();
    }
    if(status == IDLE && alarmCode != RESET_SIGNAL) {
        requestOpenDoor();
    }
}


//These two functions are the slots when the open button is pressed/released
void Elevator::pressOpenDoor(){
    requestOpenDoor();
    openDoorButtonPressed = true;
}

void Elevator::releaseOpenDoor(){
    openDoorButtonPressed = false;
}

void Elevator::pressFloorButton() {
    QPushButton *b = qobject_cast<QPushButton *>(sender());
    emit requestFloor(b->text().toInt());
}


/** Signals **/

/**
 * @brief Elevator::requestOpenDoor asks the ECS to open the door
 */
void Elevator::requestOpenDoor() {
    emit requestOpenOrClose(OPEN_DOOR);
}

/**
 * @brief Elevator::requestCloseDoor asks the ECS to close the door
 */
void Elevator::requestCloseDoor() {
    emit requestOpenOrClose(CLOSE_DOOR);
}

void Elevator::sendFireAlarmSignal() {
   emit sendAlarmSignal(FIRE_SIGNAL, id);
}

void Elevator::sendOverloadSignal() {
    emit sendAlarmSignal(OVERLOAD_SIGNAL, id);
}

void Elevator::sendHelpSignal() {
    emit sendAlarmSignal(HELP_SIGNAL, id);
}

void Elevator::blockDoor(int arg1)
{
    doorIsBlocked = arg1;
}

Elevator::~Elevator(){
    delete ui;
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
            QTime doItAgain = QTime::currentTime().addSecs(2);
            if(targetElevator->attempts == 0) {
                while(QTime::currentTime() < travelTime) {
                    if(!targetElevator->ECSConnected) return;
                    if (!targetElevator->isDoorOpen()) break;
                }
            }
            else {
                while(QTime::currentTime() < travelTime) {
                    if(!targetElevator->ECSConnected) return;
                    if (!targetElevator->isDoorOpen()) break;
                }
            }
            while(targetElevator->IsOpenDoorButtonPressed()
                  || targetElevator->getAlarmCode() != -1
                  || targetElevator->getStatus() == IDLE) {
                if(!targetElevator->ECSConnected) return;
            }
            targetElevator->requestCloseDoor();
        }
    }
}

void ElevatorTask::waitForHelp() {
    QTime waitTime = QTime::currentTime().addSecs(5);
    while(QTime::currentTime() < waitTime) {
        if(!targetElevator->ECSConnected) return;
        if(targetElevator->getAlarmCode() != HELP_SIGNAL) return;
    }
    targetElevator->sendHelpSignal(); //send the second help signal
}

void ElevatorTask::run() {
    if(command == "moveUp")
        moveUp();
    if(command == "moveDown")
        moveDown();
    if(command == "doorMonitor")
        doorMonitor();
    if(command == "waitForHelp")
        waitForHelp();
}



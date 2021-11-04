#include "ecs.h"
#include <QDebug>
#include "./ui_ecs.h"

ECS::ECS(int numOfFloors, int numOfElevators, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ECS)
{
    ui->setupUi(this);
    for(int i = 0; i <= numOfFloors; i++){
        connectFloor();
    }
    connectElevators(numOfElevators);
    elevators[0]->show();
    floors[0]->show();
    currentElevator = elevators[0];
    currentFloor = floors[0];
    connect(ui->ElevatorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ECS::selectElevator);
    connect(ui->FloorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ECS::selectFloor);
    connect(ui->fireAlarm, &QPushButton::pressed, this, &ECS::buildingFireAlarm);
    connect(ui->powerOut, &QPushButton::pressed, this, &ECS::powerOut);
    connect(ui->engage, &QPushButton::pressed, this, &ECS::engageWithHelp);

}

ECS::~ECS() {
    for(Elevator *e: qAsConst(elevators)) {
        e->ECSConnected = false;
        e->resetAlarm();
    }
    delete ui;
}

/**
 * @brief Creates and connect and elevator to the ECS
 */
void ECS::connectElevator() {
    Elevator *e = new Elevator(elevators.size(), floors.size());
    elevators.insert(e->getID(), e);
    connect(e, &Elevator::requestFloor, this, &ECS::requestFloor);
    connect(e, &Elevator::arrivalNotice, this, &ECS::receiveArrivalNotice);
    connect(e, &Elevator::requestOpenOrClose, this, &ECS::openOrCloseRequest);
    connect(e, &Elevator::sendAlarmSignal, this, &ECS::receiveAlarmSignal);
    ui->ElevatorComboBox->addItem("Elevator " + QString::number(e->getID()));
    ui->elevatorHolder->addWidget(e);
    e->hide();
    QVector<int> selectedFloors;
    for(int i=0; i<floors.size(); i++) selectedFloors.append(0);
    requestedFloors.append(selectedFloors);
    writeToConsole(QString("Elevator id: ") + QString::number(e->getID()) + " connected to the ECS");
    e->ECSConnected = true;
    startThreadCommand(e, "doorMonitor");
}

/**
 * @brief Creates and connect multiple elevators to the ECS
 * @param n Number of elevators to be created.
 */
void ECS::connectElevators(int n) {
    if (n < 1) return;
    int i = 0;
    for(;i < n; i++)
        connectElevator();
}


/**
 * @brief Creates and connect a floor to the elevator
 */
void ECS::connectFloor() {
    Floor *f = new Floor(floors.size());
    floors.insert(f->getID(), f);
    //This block re-enables the up and down buttons depending on where the floor is situated relative to othe floors
    if(floors.size() != 1) {
        f->enableDownButton(); //re-enable down button
        floors[floors.size() - 2]->enableUpButton(); //re-enable up button for previous floor
    }
    //connect the floor buttons to the ecs
    connect(f, &Floor::requestElevator, this, &ECS::requestElevator);
    ui->FloorComboBox->addItem("Floor " + QString::number(f->getID()));
    ui->floorHolder->addWidget(f);
    f->hide();
    writeToConsole("Floor id: " + QString::number(f->getID()) + " connected to the ECS");
}

/**
 * @brief Creates and connect multiple floors to the ECS.
 * @param n
 */
void ECS::connectFloors(int n) {
    if (n < 1) return;
    int i = 0;
    for(;i <=n; i++)
        connectFloor();
}

void ECS::clearAllRequests() {
    for(QVector<int> &v: requestedFloors) {
        for(int &i : v){
            i = 0;
        }
    }
}

bool ECS::hasRequests(Elevator *e) {
    for(const int &i: qAsConst(requestedFloors[e->getID()]))
        if (i == 1)
            return true;
    return false;
}

void ECS::writeToConsole(const QString & text) {
    QString fullText = ">> ";
    fullText += text;
    ui->consoleBrowser->append(fullText);
}

void ECS::enableOpenCloseButton(Elevator *e){
    e->openButton->setDisabled(false);
    e->closeButton->setDisabled(false);
}

void ECS::disableOpenCloseButton(Elevator *e){
    e->openButton->setDisabled(true);
    e->closeButton->setDisabled(true);
}

void ECS::changeElevatorDirection(Elevator *e, Direction d) {
        e->setDirection(d);
}

void ECS::changeElevatorStatus(Elevator *e, Status s) {
    if(e->doorIsOpen && s == MOVING) return; //Don't want to move while the door is open
    if(s == STOP || s == IDLE || s == ARRIVAL) {
        if(e->getAlarmCode() == -1) enableOpenCloseButton(e);
    }
    else {
        disableOpenCloseButton(e);
    }
        e->setStatus(s);
}


void ECS::startThreadCommand(Elevator *e, const QString &command) {
    ElevatorTask *task = new ElevatorTask(e, command);
    QThreadPool::globalInstance()->start(task);
}


/**
 * @brief Called by the elevator buttons when requesting to go to a given floor
 */
void ECS::requestFloor(int floor) {
    Elevator *e = qobject_cast<Elevator *>(sender());
    int id = e->getID();
    e->buttons[floor]->setDisabled(true);
    requestedFloors[id][floor] = 1;
    if(e->getStatus() == IDLE) changeElevatorStatus(e, STOP);
    writeToConsole("Elevator " + QString::number(id) + " requested floor " + QString::number(floor));
}

/**
 * @brief Controls the movement of the elevator depending on the requested floors it has. Will prioritize its current direction
 * before changing to another direction. Commmand Elevator is either triggered by an arrival notice or when an elevator open/close its doors.
 * @param e: The elevator to control
 */
void ECS::commandElevator(Elevator *e) {
    if(!hasRequests(e)) {
        writeToConsole("Elevator " + QString::number(e->getID()) + " has no more requests");
        changeElevatorStatus(e, IDLE);
    }
    //If floors left and previous status is ARRIVAL, will ring the bell becaus it still has a job to do.
    if(e->getStatus() == ARRIVAL) writeToConsole("Elevator " + QString::number(e->getID()) + " rings the bell");
    if((e->getStatus() == STOP) && !e->doorIsOpen){
        if(e->getDirection() == UP) {
            for(int i=e->getCurrentFloor() + 1; i < floors.size(); i++) {
                if(requestedFloors[e->getID()][i] == 1) {
                    changeElevatorStatus(e, MOVING);
                    startThreadCommand(e, "moveUp");
                    return;
                }
            }
            for(int i=e->getCurrentFloor() - 1; i >= 0; i--) {
                if(requestedFloors[e->getID()][i] == 1) {
                    changeElevatorStatus(e, MOVING);
                    e->setDirection(DOWN);
                    startThreadCommand(e, "moveDown");
                    return;
                }
            }
        }
        else if(e->getDirection() == DOWN) {
            for(int i=e->getCurrentFloor() - 1; i >= 0; i--) {
                if(requestedFloors[e->getID()][i] == 1) {
                    changeElevatorStatus(e, MOVING);
                    startThreadCommand(e, "moveDown");
                    return;
                }
            }
            for(int i=e->getCurrentFloor() + 1; i < floors.size(); i++) {
                if(requestedFloors[e->getID()][i] == 1) {
                    changeElevatorStatus(e, MOVING);
                    e->setDirection(UP);
                    startThreadCommand(e, "moveUp");
                    return;
                }
            }
        }
    }
}

/**
 * @brief When a floor request to get an elevator. Follows an order of priority.
 */
void ECS::requestElevator(int floorID, Direction direction) {
    //1. Check if there's an STOP elevator on the currentFloor
    Elevator *firstPrio = nullptr;
    Elevator *secondPrio = nullptr;
    Elevator *thirdPrio = nullptr;
    Elevator *foundElevator = elevators[0];
    for(Elevator *e: qAsConst(elevators)) {
        if(e->getStatus() == IDLE && e->getCurrentFloor() == floorID) {
            e->requestOpenDoor();
            return;
        }
        if(direction == UP) {
            if(e->getCurrentFloor() < floorID && e->getDirection() == UP) {
                if(!secondPrio)
                    secondPrio = e;
                else if(requestedFloors[e->getID()][floorID] == 0)
                    secondPrio = e;
                else if(abs(floorID - e->getID()) < abs(floorID - secondPrio->getID()))
                    secondPrio = e;
            }
            if(e->getCurrentFloor() > floorID && e->getDirection() == DOWN) {
                if(!thirdPrio)
                    thirdPrio = e;
                else if(requestedFloors[e->getID()][floorID] == 0)
                    thirdPrio = e;
                else if(abs(floorID - e->getID()) < abs(floorID - thirdPrio->getID()))
                    secondPrio = e;
                }
            else {
                foundElevator = e;
            }
        }
        else {
            if(e->getCurrentFloor() > floorID && e->getDirection() == DOWN) {
                if(!secondPrio)
                    secondPrio = e;
                else if(requestedFloors[e->getID()][floorID] == 0)
                    secondPrio = e;
                else if(abs(floorID - e->getID()) < abs(floorID - secondPrio->getID()))
                    secondPrio = e;
            }
            if(e->getCurrentFloor() < floorID && e->getDirection() == UP) {
                if(!thirdPrio)
                    thirdPrio = e;
                else if(requestedFloors[e->getID()][floorID] == 0)
                    thirdPrio = e;
                else if(abs(floorID - e->getID()) < abs(floorID - thirdPrio->getID()))
                    secondPrio = e;
                }
            else {
                foundElevator = e;
            }
        }
    }

        if(firstPrio) foundElevator = firstPrio;
        else if(secondPrio) foundElevator = secondPrio;
        else if(thirdPrio) foundElevator = thirdPrio;
        //Then we signal the elevator that we want this floor by signaling the elevator button
        emit foundElevator->buttons[floorID]->released();
}


void ECS::receiveArrivalNotice(int floor) {
    Elevator *e = qobject_cast<Elevator *>(sender());
    writeToConsole("Elevator " + QString::number(e->getID()) + " arrived at floor " + QString::number(floor));
    if(e->getAlarmCode() == -1) e->updateDisplay("Floor: " + QString::number(e->getCurrentFloor()));
    if(requestedFloors[e->getID()][floor] == 1) {
        //Start the arrival sequence
        writeToConsole("Elevator " + QString::number(e->getID()) + " rings the bell");
        requestedFloors[e->getID()][floor] = 0;
        e->buttons[floor]->setDisabled(false);
        changeElevatorStatus(e, ARRIVAL);
        return;
    }
    commandElevator(e);
}

void ECS::openOrCloseRequest(char doorSignal) {
    Elevator *e = qobject_cast<Elevator *>(sender());
    if(e->getStatus() != STOP && e->getStatus() != IDLE && e->getStatus() != ARRIVAL) return; //Don't want to open door while the elevator is moving
    if(doorSignal == OPEN_DOOR) {
        if(e->doorIsOpen) return;
        e->doorIsOpen = true;
        writeToConsole("Elevator " + QString::number(e->getID()) + " is opening doors");
    }
    else if(doorSignal == CLOSE_DOOR) {
        if(!e->doorIsOpen) return;
        e->attempts++;
        if(e->doorIsBlocked == true) {
            if(e->attempts >= 4) {
                e->updateDisplay("Please clear the path for the doors to close.");
                e->updateAudio("Please clear the path for the doors to close.");
                return;
            }
            writeToConsole("Door is blocked, attempting to close again.");
        }
        else {
            e->doorIsOpen = false;
            writeToConsole("Elevator " + QString::number(e->getID()) + " is closing doors");
            e->attempts = 0;
            changeElevatorStatus(e, STOP);
        }
    }
}


void ECS::buildingFireAlarm() {
    receiveAlarmSignal(FIRE_SIGNAL, -1);
}

void ECS::powerOut() {
    receiveAlarmSignal(POWER_SIGNAL, -1);
}

/**
 * @brief ECS::receiveAlarmSignal
 * @param signalCode maps to one of the defined signal codes in alarmsignalsingleton.h
 * @param id - It is either the id of the elevator or the building that sent the signal.
 * Note that -1 is reserved by the building and everything else are elevators.
 */
void ECS::receiveAlarmSignal(int signalCode, int id) {
    if(signalCode == FIRE_SIGNAL) {
        if(id == -1) {
            for(Elevator *e: qAsConst(elevators)) {
                e->sendFireAlarmSignal();
            }
        }
        else {
            Elevator *e = elevators[id];
            e->setAlarm(FIRE_SIGNAL);
            e->updateDisplay("Fire Alarm issued, please the disembark at the next available floor.");
            e->updateAudio("Fire Alarm issued, please the disembark at the next available floor.");
            for(int i=0; i < floors.size(); i ++)
                requestedFloors[id][i] = 0;
            for(QPushButton *b: qAsConst(e->buttons))
                b->setDisabled(true);
        }
    }
    if(signalCode == OVERLOAD_SIGNAL) {
        Elevator *e = elevators[id];
        if(e->getAlarmCode() == FIRE_SIGNAL || e->getAlarmCode() == HELP_SIGNAL ||
                e->getAlarmCode() == POWER_SIGNAL)
            return; //These signals take precedence and should not be overwritten.
        e->updateDisplay("The elevator will not move until the load has been reduced.");
        e->updateAudio("The elevator will not move until the load has been reduced.");
    }
    if(signalCode == HELP_SIGNAL) {
        Elevator *e = elevators[id];
        if(e->getAlarmCode() == HELP_SIGNAL) { //sent a second time
            writeToConsole("Elevator " + QString::number(e->getID()) + " sent request to call 911");
        }
        else {
            e->setAlarm(HELP_SIGNAL);
            startThreadCommand(e, "waitForHelp");
            writeToConsole("Elevator " + QString::number(id) + " requested help.");
        }
    }
    if(signalCode == POWER_SIGNAL) {
        for(Elevator *e: qAsConst(elevators)) {
            e->setAlarm(POWER_SIGNAL);
            e->updateDisplay("POWER OUT, please disembark on the next available floor.");
            e->updateAudio("Power out, please disembark the next available floor.");
            for(int i=0; i < floors.size(); i ++)
                requestedFloors[e->getID()][i] = 0;
            for(QPushButton *b: qAsConst(e->buttons))
                b->setDisabled(true);
        }
    }
}

void ECS::selectElevator(int index) {
    currentElevator->hide();
    elevators[index]->show();
    currentElevator = elevators[index];
}

void ECS::selectFloor(int index) {
    currentFloor->hide();
    floors[index]->show();
    currentFloor = floors[index];
}

void ECS::engageWithHelp() {
    for(Elevator *e: qAsConst(elevators)){
        if(e->getAlarmCode() == HELP_SIGNAL) {
            e->resetAlarm();
            writeToConsole("Help was received, clearing help signal");
        }
    }
}

#include "ecs.h"
#include <QDebug>
#include "./ui_ecs.h"

ECS::ECS(int numOfFloors, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ECS)
{
    ui->setupUi(this);
    elevatorsTab = new QTabWidget;
    floorsTab = new QTabWidget;
    building = new Building();
    for(int i = 0; i <= numOfFloors; i++){
        connectFloor();
    }
    ui->elevators->layout()->addWidget(elevatorsTab);
    ui->floors->layout()->addWidget(floorsTab);
    ui->building->layout()->addWidget(building);
    connect(building, &Building::sendAlarmSignal, this, &ECS::receiveAlarmSignal);

}

ECS::~ECS() {
    for(Elevator *e: qAsConst(elevators)) {
        e->ECSConnected = false;
        e->resetAlarm();
        qDebug() << e->getLastAlarm();
    }
    delete ui;
}

void ECS::showFloor(int id){
    floors[id]->show();
}

void ECS::showElevator(int id){
    elevators[id]->show();
}

/**
 * @brief Creates and connect and elevator to the ECS
 */
void ECS::connectElevator() {
    Elevator *e = new Elevator(elevators.size(), floors.size());
    elevators.insert(e->getID(), e);
    connect(e, &Elevator::requestFloor, this, &ECS::requestFloor, Qt::UniqueConnection);
    connect(e, &Elevator::arrivalNotice, this, &ECS::receiveArrivalNotice, Qt::UniqueConnection);
    connect(e, &Elevator::requestOpenOrClose, this, &ECS::openOrCloseRequest, Qt::UniqueConnection);
    connect(e, &Elevator::sendAlarmSignal, this, &ECS::receiveAlarmSignal, Qt::UniqueConnection);
    elevatorsTab->addTab(e, QString::number(e->getID()));

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
        f->panel->buttons[1]->setDisabled(false); //re-enable down button
        floors[floors.size() - 2]->panel->buttons[0]->setDisabled(false); //re-enable up button for previous floor
    }
    //connect the floor buttons to the ecs
    connect(f->panel->buttons[0], &QPushButton::released, this, &ECS::requestElevator);
    connect(f->panel->buttons[1], &QPushButton::released, this, &ECS::requestElevator);
    floorsTab->addTab(f, QString::number(f->getID()));
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
    e->panel->openButton->setDisabled(false);
    e->panel->closeButton->setDisabled(false);
}

void ECS::disableOpenCloseButton(Elevator *e){
    e->panel->openButton->setDisabled(true);
    e->panel->closeButton->setDisabled(true);
}

void ECS::changeElevatorDirection(Elevator *e, Direction d) {
        e->setDirection(d);
}

void ECS::changeElevatorStatus(Elevator *e, Status s) {
    if(e->doorIsOpen && s == MOVING) return; //Don't want to move while the door is open
    if(s == STOP || s == IDLE) {
        if(e->getLastAlarm() == -1) enableOpenCloseButton(e);
    }
    else {
        disableOpenCloseButton(e);
    }
        e->setStatus(s);
        statusUpdated(e);
}


void ECS::startThreadCommand(Elevator *e, const QString &command) {
    ElevatorTask *task = new ElevatorTask(e, command);
    QThreadPool::globalInstance()->start(task);
}


void ECS::statusUpdated(Elevator *e) {
    e->statusUpdated();
}

void ECS::pingElevator(Elevator *e) {
    e->pinged();
}


/**
 * @brief Called by the elevator buttons when requesting to go to a given floor
 */
void ECS::requestFloor(int floor) {
    Elevator *e = qobject_cast<Elevator *>(sender());
    int id = e->getID();
    e->panel->buttons[floor]->setDisabled(true);
    requestedFloors[id][floor] = 1;
    if(e->getStatus() == IDLE) {
        pingElevator(e);
    }
    writeToConsole("Elevator " + QString::number(id) + " requested floor " + QString::number(floor));
}

/**
 * @brief Controls the movement of the elevator depending on the requested floors it has. Will prioritize its current direction
 * before changing to another direction. Commmand Elevator is either triggered by an arrival notice or when an elevator open/close its doors.
 * @param e: The elevator to control
 */
void ECS::commandElevator(Elevator *e) {
    qDebug() << "ping";
    if(!hasRequests(e)) {
        writeToConsole("Elevator " + QString::number(e->getID()) + " has no more requests");
        changeElevatorStatus(e, IDLE);
    }
    else if(e->getStatus() == IDLE) {
        changeElevatorStatus(e, STOP);
    }
    if(e->getStatus() == STOP && !e->doorIsOpen){
        if(e->getDirection() == UP) {
            for(int i=e->getCurrentFloor() + 1; i < floors.size(); i++) {
                if(requestedFloors[e->getID()][i] == 1) {
                    e->setStatus(MOVING);
                    startThreadCommand(e, "moveUp");

                    return;
                }
            }
            for(int i=e->getCurrentFloor() - 1; i >= 0; i--) {
                if(requestedFloors[e->getID()][i] == 1) {
                    e->setStatus(MOVING);
                    e->setDirection(DOWN);
                    startThreadCommand(e, "moveDown");

                    return;
                }
            }
        }
        else if(e->getDirection() == DOWN) {
            for(int i=e->getCurrentFloor() - 1; i >= 0; i--) {
                if(requestedFloors[e->getID()][i] == 1) {
                    e->setStatus(MOVING);
                    startThreadCommand(e, "moveDown");

                    return;
                }
            }
            for(int i=e->getCurrentFloor() + 1; i < floors.size(); i++) {
                if(requestedFloors[e->getID()][i] == 1) {
                    e->setStatus(MOVING);
                    e->setDirection(UP);
                    startThreadCommand(e, "moveUp");

                    return;
                }
            }
        }
    }
    changeElevatorStatus(e, IDLE);

}

/**
 * @brief When a floor request to get an elevator. Follows an order of priority.
 */
void ECS::requestElevator(void) {
    QPushButton *b = qobject_cast<QPushButton *>(sender());
    int floorID = qobject_cast<ElevatorComponentFactory *>(b->parentWidget())->getID();
    Direction direction = b->text() == "Up" ? UP : DOWN;
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
        emit foundElevator->panel->buttons[floorID]->released();
}


void ECS::receiveArrivalNotice(int floor) {
    Elevator *e = qobject_cast<Elevator *>(sender());
    e->writeToConsole(" arrived at floor " + QString::number(floor));
    if(e->getLastAlarm() == -1) e->updateDisplay("Floor: " + QString::number(e->getCurrentFloor()));
    changeElevatorStatus(e, STOP);
    if(requestedFloors[e->getID()][floor] == 1) {
        e->writeToConsole("Ding");
        requestedFloors[e->getID()][floor] = 0;
        e->panel->buttons[floor]->setDisabled(false);
        e->requestOpenDoor();
        return;
    }
    // else move along and do the next one
    commandElevator(e);
}

void ECS::openOrCloseRequest(char doorSignal) {
    Elevator *e = qobject_cast<Elevator *>(sender());
    if(e->getStatus() != STOP && e->getStatus() != IDLE) return; //Don't want to open door while the elevator is moving
    if(doorSignal == OPEN_DOOR) {
        if(e->doorIsOpen) return;
        e->doorIsOpen = true;
        e->weightBox->setDisabled(false);
        e->writeToConsole("Opening doors");
    }
    else if(doorSignal == CLOSE_DOOR) {
        if(!e->doorIsOpen) return;
        e->doorIsOpen = false;
        e->weightBox->setDisabled(true);
        e->writeToConsole("closing doors");
        e->writeToConsole("ding");
        commandElevator(e);
    }
}

/**
 * @brief ECS::receiveAlarmSignal
 * @param signalCode maps to one of the defined signal codes in alarmsignalsingleton.h
 * @param id - It is either the id of the elevator or the building that sent the signal.
 * Note that -1 is reserved by the building and everything else are elevators.
 */
void ECS::receiveAlarmSignal(int signalCode) {
    int id = qobject_cast<ElevatorComponentFactory *>(sender())->getID();
    qDebug() << id;
    if(signalCode == FIRE_SIGNAL) {
        if(id == -1) {
            for(Elevator *e: qAsConst(elevators)) {
                e->sendFireAlarmSignal();
            }
        }
        else {
            Elevator *e = elevators[id];
            e->setAlarm(FIRE_SIGNAL);
            e->updateDisplay("Fire Alarm issued, please the disdoorMonitor to the next available floor.");
            for(int i=0; i < floors.size(); i ++)
                requestedFloors[id][i] = 0;
            for(QPushButton *b: qAsConst(e->panel->buttons))
                b->setDisabled(true);
        }
    }
    if(signalCode == OVERLOAD_SIGNAL) {
        Elevator *e = elevators[id];
        if(e->getLastAlarm() == FIRE_SIGNAL || e->getLastAlarm() == HELP_SIGNAL ||
                e->getLastAlarm() == POWER_SIGNAL)
            return; //These signals take precedence and should not be overwritten.
        qDebug() << "Weight signal";
        e->updateDisplay("The elevator will not move until the load has been reduced.");
        e->writeToConsole("Audio message: please reduce the load of the elevator.");
    }
    if(signalCode == HELP_SIGNAL) {
        Elevator *e = elevators[id];
        if(e->getLastAlarm() == HELP_SIGNAL) { //sent a second time
            writeToConsole("THE ECS CALLED 911 for Elevator: " + QString::number(id));
        }
        else {
            e->setAlarm(HELP_SIGNAL);
        }
    }
}


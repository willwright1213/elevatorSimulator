#include "ecs.h"
#include <QDebug>
#include "./ui_ecs.h"

ECS::ECS(int numOfFloors, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ECS)
{
    ui->setupUi(this);
    for(int i = 0; i <= numOfFloors; i++){
        connectFloor();
    }
    elevatorsTab = new QTabWidget;
    ui->elevators->layout()->addWidget(elevatorsTab);
}

ECS::~ECS()
{
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
    connect(e, &Elevator::arrivalNotice, this, &ECS::receiveArrivalNotice);
    addElevatorToGUI(e);
    for(QPushButton *b: qAsConst(e->panel->buttons)) {
        connect(b, &QPushButton::released, this, &ECS::requestFloor);
    }
    QVector<int> selectedFloors;
    for(int i=0; i<floors.size(); i++) selectedFloors.append(0);
    requestedFloors.append(selectedFloors);
    writeToConsole(QString("Elevator id: ") + QString::number(e->getID()) + " connected to the ECS");
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


void ECS::addElevatorToGUI(Elevator *e) {
    elevatorsTab->addTab(e, QString::number(e->getID()));
}

/**
 * @brief Creates and connect a floor to the elevator
 */
void ECS::connectFloor() {
    Floor *f = new Floor(floors.size());
    floors.insert(f->getID(), f);
    requestedElevator.append(false);
    if(floors.size() != 1) {
        f->panel->buttons[0]->setDisabled(false); //re-enable down button
        floors[floors.size() - 2]->panel->buttons[0]->setDisabled(false); //re-enable up button for previous floor
    }
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

void ECS::writeToConsole(const QString & text) {
    QString fullText = ">> ";
    fullText += text;
    ui->consoleBrowser->append(fullText);
}

/**
 * @brief Called by the elevator buttons when requesting to go to a given floor
 */
void ECS::requestFloor(void) {
    QPushButton *b = qobject_cast<QPushButton *>(sender());
    int elevatorID = qobject_cast<ElevatorComponentFactory *>(b->parentWidget())->getID();
    int floor = b->text().toInt();
    elevators[elevatorID]->panel->buttons[floor]->setDisabled(true);
    requestedFloors[elevatorID][floor] = 1;
    if(elevators[elevatorID]->getDirection() == IDLE) {
        if(elevators[elevatorID]->getCurrentFloor() < floor) {
            elevators[elevatorID]->setDirection(UP);
        }
        if(elevators[elevatorID]->getCurrentFloor() > floor) {
            elevators[elevatorID]->setDirection(DOWN);
        }
        commandElevator(elevators[elevatorID]);
    }
    writeToConsole("Elevator " + QString::number(elevatorID) + " requested floor " + QString::number(floor));
}

void ECS::commandElevator(Elevator *e) {
    if(e->getDirection() == UP) {
        for(int i=e->getCurrentFloor() + 1; i < floors.size(); i++) {
            if(requestedFloors[e->getID()][i] == 1) {
                ElevatorTask *task = new ElevatorTask(e, "moveUp");
                QThreadPool::globalInstance()->start(task);
                return;
            }
        }
        for(int i=e->getCurrentFloor() - 1; i >= 0; i--) {
            if(requestedFloors[e->getID()][i] == 1) {
                ElevatorTask *task = new ElevatorTask(e, "moveDown");
                QThreadPool::globalInstance()->start(task);
                return;
            }
        }
    }
    else if(e->getDirection() == DOWN) {
        for(int i=e->getCurrentFloor() - 1; i >= 0; i--) {
            if(requestedFloors[e->getID()][i] == 1) {
                ElevatorTask *task = new ElevatorTask(e, "moveDown");
                QThreadPool::globalInstance()->start(task);
                return;
            }
        }
        for(int i=e->getCurrentFloor() + 1; i < floors.size(); i++) {
            if(requestedFloors[e->getID()][i] == 1) {
                ElevatorTask *task = new ElevatorTask(e, "moveUp");
                QThreadPool::globalInstance()->start(task);
                return;
            }
        }
    }
    writeToConsole("There are no more requested floors");
    e->setDirection(IDLE);
}

void ECS::requestElevator(void) {
    //1. is the elevator already here?
    //2. is an elevator under going up?
    //3. is there an elevator under that is idle?
    //4. is that elevator already full? Then choose another one fucker
}



void ECS::receiveArrivalNotice(int floor) {
    Elevator *e = qobject_cast<Elevator *>(sender());
    writeToConsole("Elevator " + QString::number(e->getID()) + " arrived at floor " + QString::number(floor));
    if(requestedFloors[e->getID()][floor] == 1 || requestedElevator[floor] == true) {
        writeToConsole("It is a requested floor, initiating embarking procedure");
        requestedFloors[e->getID()][floor] = 0;
        requestedElevator[floor] = false;
        ElevatorTask *task = new ElevatorTask(e, "embark");
        QThreadPool::globalInstance()->start(task);

    }
    commandElevator(e);
}

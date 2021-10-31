#include "elevatorcomponentfactory.h"
#include "ui_elevatorcomponentfactory.h"

ElevatorComponentFactory::ElevatorComponentFactory(int id, QWidget *parent) :
    id(id),
    QWidget(parent),
    ui(new Ui::ElevatorComponentFactory)
{
    ui->setupUi(this);
    layout = new QGridLayout();
    setLayout(layout);
}

ElevatorComponentFactory::~ElevatorComponentFactory()
{
    delete ui;
}

int ElevatorComponentFactory::getID() const {
    return id;
}

int ElevatorComponentFactory::getLastAlarm() const {
    return alarmSent;
}

void ElevatorComponentFactory::resetAlarm() {
    alarmSent = RESET_SIGNAL;
}

void ElevatorComponentFactory::setAlarm(int code) {
    alarmSent = code;
}

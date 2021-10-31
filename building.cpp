#include "building.h"
#include <QDebug>

Building::Building():ElevatorComponentFactory(-1)
{
    fireAlarm = new QPushButton("Fire Alarm");
    powerOut = new QPushButton("Power out");
    layout->addWidget(fireAlarm);
    layout->addWidget(powerOut);
    connect(fireAlarm, &QPushButton::pressed, this, &Building::sendFireAlarmSignal);
    connect(powerOut, &QPushButton::pressed, this, &Building::sendPowerOutSignal);
}


void Building::sendFireAlarmSignal(){
    emit sendAlarmSignal(FIRE_SIGNAL);
}

void Building::sendPowerOutSignal() {
    emit sendAlarmSignal(POWER_SIGNAL);
}

#ifndef BUILDING_H
#define BUILDING_H

#include <QPushButton>
#include "elevatorcomponentfactory.h"

class Building : public ElevatorComponentFactory
{
public:
    Building();
    QPushButton *fireAlarm;
    QPushButton *powerOut;
    void sendFireAlarmSignal();
    void sendPowerOutSignal();
};

#endif // BUILDING_H

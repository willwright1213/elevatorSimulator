#ifndef FLOOR_H
#define FLOOR_H


#include "elevatorcomponentfactory.h"
#include "floorpanel.h"

class Floor : public ElevatorComponentFactory
{
public:
    Floor(int floorId);
    FloorPanel *panel;
};

#endif // FLOOR_H

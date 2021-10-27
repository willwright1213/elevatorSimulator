#ifndef FLOOR_H
#define FLOOR_H


#include "elevatorcomponentfactory.h"
#include "floorpanel.h"

class Floor : public ElevatorComponentFactory
{
public:
    Floor(int floorId);
    FloorPanel *panel;
    void upRequest();
    void downRequest();
};

#endif // FLOOR_H

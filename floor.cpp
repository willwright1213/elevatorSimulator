#include "floor.h"

Floor::Floor(int floorId):ElevatorComponentFactory(floorId, nullptr)
{
    panel = new FloorPanel(floorId);
    layout->addWidget(panel);
}

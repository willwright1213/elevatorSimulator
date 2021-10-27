#ifndef ELEVATORPANEL_H
#define ELEVATORPANEL_H
#include "panelfactory.h"
#include <QObject>
#include <QPushButton>

class ElevatorPanel : public PanelFactory
{
public:
    ElevatorPanel(int id, int cols, int floors);
};

#endif // ELEVATORPANEL_H

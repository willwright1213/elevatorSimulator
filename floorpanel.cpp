#include "floorpanel.h"

FloorPanel::FloorPanel(int id):PanelFactory(id, 1)
{
    addButton("Up");
    addButton("Down");
    buttons[0]->setDisabled(true);
    buttons[1]->setDisabled(true);
}

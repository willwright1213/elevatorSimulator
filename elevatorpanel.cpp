#include "elevatorpanel.h"

ElevatorPanel::ElevatorPanel(int id, int cols, int floors):PanelFactory(id, cols, nullptr)
{
  for(int i = 0; i < floors; i++) {
    addButton(QString::number(i));
  }
  addButton("Open");
  addButton("Close");
  addButton("Help");
  addButton("Fire");
}


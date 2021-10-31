#include "elevatorpanel.h"

ElevatorPanel::ElevatorPanel(int id, int cols, int floors):PanelFactory(id, cols, nullptr)
{
  for(int i = 0; i < floors; i++) {
    addButton(QString::number(i));
  }
  addButton("Open");
  openButton = buttons[buttons.size() - 1];
  addButton("Close");
  closeButton = buttons[buttons.size() - 1];
  addButton("Help");
  helpButton = buttons[buttons.size() - 1];
  addButton("Fire");
  fireButton = buttons[buttons.size() - 1];
}


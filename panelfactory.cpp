#include "panelfactory.h"

PanelFactory::PanelFactory(int id, int cols, QWidget *parent):ElevatorComponentFactory(id, parent),
  cols(cols)
{
    panelLayout = new QGridLayout();
    layout->addLayout(panelLayout);
}

void PanelFactory::addButton(const QString & label) {
    QPushButton *b = new QPushButton(label);
    panelLayout->addWidget(b, buttons.size()/cols, buttons.size()%cols);
    buttons.append(b);
}

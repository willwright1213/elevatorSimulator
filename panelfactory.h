#ifndef PANELFACTORY_H

#define PANELFACTORY_H

#include "elevatorcomponentfactory.h"
#include <QPushButton>

class PanelFactory : public ElevatorComponentFactory
{
public:
    PanelFactory(int id, int cols, QWidget *parent = nullptr);
    void addButton(const QString &);
    QVector<QPushButton *> buttons;
protected:
    int cols;
    QGridLayout *panelLayout;

};

#endif // PANELFACTORY_H

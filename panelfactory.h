#ifndef PANELFACTORY_H

#define PANELFACTORY_H

#include "elevatorcomponentfactory.h"
#include <QSpacerItem>
#include <QPushButton>

class PanelFactory : public ElevatorComponentFactory
{
public:
    QVector<QPushButton *> buttons;
protected:
    PanelFactory(int id, int cols, QWidget *parent = nullptr);
    void addButton(const QString &);
    int cols;
    QGridLayout *panelLayout;

};

#endif // PANELFACTORY_H

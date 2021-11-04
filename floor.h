#ifndef FLOOR_H
#define FLOOR_H

#include <QWidget>
#include <QPushButton>
#include "defs.h"

namespace Ui {
class Floor;
}

class Floor : public QWidget
{
    Q_OBJECT
public:
    Floor(int floorId, QWidget *parent = nullptr);
    ~Floor();
    int getID() const {return id;}
    void enableDownButton();
    void enableUpButton();
private slots:
    void pressFloorButton();
signals:
    void requestElevator(int floorID, Direction direction);
private:
    int id;
    Ui::Floor *ui;
};

#endif // FLOOR_H

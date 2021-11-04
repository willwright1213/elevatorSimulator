#include "floor.h"
#include "ui_floor.h"

Floor::Floor(int floorId, QWidget *parent):
    id(floorId),
    QWidget(parent),
    ui(new Ui::Floor)
{
    ui->setupUi(this);
    ui->upButton->setDisabled(true);
    ui->downButton->setDisabled(true);
    ui->floorLabel->setText("Floor " + QString::number(id));
    connect(ui->upButton, &QPushButton::pressed, this, &Floor::pressFloorButton);
    connect(ui->downButton, &QPushButton::pressed, this, &Floor::pressFloorButton);
}


void Floor::pressFloorButton() {
   QPushButton *b = qobject_cast<QPushButton *>(sender());
    if (b->text() == "Up")
        emit requestElevator(id, UP);
    else
        emit requestElevator(id, DOWN);
}

void Floor::enableUpButton() {
    ui->upButton->setDisabled(false);
}

void Floor::enableDownButton() {
    ui->downButton->setDisabled(false);
}

Floor::~Floor() {
    delete ui;
}

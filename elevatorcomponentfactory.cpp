#include "elevatorcomponentfactory.h"
#include "ui_elevatorcomponentfactory.h"

ElevatorComponentFactory::ElevatorComponentFactory(int id, QWidget *parent) :
    id(id),
    QWidget(parent),
    ui(new Ui::ElevatorComponentFactory)
{
    ui->setupUi(this);
    layout = new QVBoxLayout();
    setLayout(layout);
}

ElevatorComponentFactory::~ElevatorComponentFactory()
{
    delete ui;
}

int ElevatorComponentFactory::getID() {
    return id;
}


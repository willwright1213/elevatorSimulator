#ifndef ELEVATORCOMPONENTFACTORY_H
#define ELEVATORCOMPONENTFACTORY_H
#include <QVBoxLayout>

#include <QWidget>
#include "defs.h"

namespace Ui {
class ElevatorComponentFactory;
}

class ElevatorComponentFactory : public QWidget
{
    Q_OBJECT

public:
    virtual ~ElevatorComponentFactory();
    int getID() const;
    int getLastAlarm() const;
    void resetAlarm();
    void setAlarm(int);

protected:
    explicit ElevatorComponentFactory(int, QWidget *parent = nullptr);
    QGridLayout *layout;
    int alarmSent = RESET_SIGNAL;

protected slots:
    //virtual void requestFromECS(const QString *) = 0;

signals:
   void sendAlarmSignal(int code);


protected:
    Ui::ElevatorComponentFactory *ui;

private:
    int id;

};

#endif // ELEVATORCOMPONENTFACTORY_H

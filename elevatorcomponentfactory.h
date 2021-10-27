#ifndef ELEVATORCOMPONENTFACTORY_H
#define ELEVATORCOMPONENTFACTORY_H
#include <QVBoxLayout>

#include <QWidget>

namespace Ui {
class ElevatorComponentFactory;
}

class ElevatorComponentFactory : public QWidget
{
    Q_OBJECT

public:
    explicit ElevatorComponentFactory(int, QWidget *parent = nullptr);
    virtual ~ElevatorComponentFactory();
    int getID();

protected:
    QVBoxLayout *layout;

protected slots:
    //virtual void requestFromECS(const QString *) = 0;

signals:
   // virtual void responseToECS(const QString *) = 0;


protected:
    Ui::ElevatorComponentFactory *ui;

private:
    int id;

};

#endif // ELEVATORCOMPONENTFACTORY_H

#ifndef ALARMSIGNALSINGLETON_H
#define ALARMSIGNALSINGLETON_H

#define RESET_SIGNAL -1
#define FIRE_SIGNAL 0
#define WEIGHT_SIGNAL 1

#include <QObject>

class AlarmSignalSingleton : public QObject
{
    Q_OBJECT
public:

    AlarmSignalSingleton(AlarmSignalSingleton &other) = delete;
    void operator=(const AlarmSignalSingleton&) = delete;
    static AlarmSignalSingleton * getAlarmSignalSingleton();
protected:
    explicit AlarmSignalSingleton(QObject *parent = nullptr);
    static AlarmSignalSingleton* alarmSignal_;

public slots:
    void receiveRequest(int signalCode, int id);
signals:
    void sendAlarmSignal(int signalCode, int id);

};

#endif // ALARMSIGNALSINGLETON_H

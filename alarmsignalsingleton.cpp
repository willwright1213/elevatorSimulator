#include "alarmsignalsingleton.h"

AlarmSignalSingleton * AlarmSignalSingleton::alarmSignal_ = nullptr;

AlarmSignalSingleton::AlarmSignalSingleton(QObject *parent) : QObject(parent) {

}

AlarmSignalSingleton * AlarmSignalSingleton::getAlarmSignalSingleton() {
    if(alarmSignal_ == nullptr) {
        alarmSignal_ = new AlarmSignalSingleton;
    }
    return alarmSignal_;
}

void AlarmSignalSingleton::receiveRequest(int signalCode, int id) {
    emit sendAlarmSignal(signalCode, id);
}

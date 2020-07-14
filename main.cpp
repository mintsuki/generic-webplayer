#include "player.h"
#include <QApplication>
#include <SingleApplication>

void runningInstance() {
    new Player(profileList->getProfile("Default"));
}

int main(int argc, char *argv[]) {
    SingleApplication a(argc, argv);

    QObject::connect(&a, &SingleApplication::instanceStarted, runningInstance);

    trayIcon    = new QSystemTrayIcon;
    profileList = new ProfileList;

    new Player(profileList->getProfile("Default"));

    int ret = a.exec();

    delete profileList;
    delete trayIcon;
    return ret;
}

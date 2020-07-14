#include "player.h"
#include <QApplication>
#include "singleapplication.h"

#include "config.h"

void runningInstance() {
    PlayerPage *page   = new PlayerPage(profileList->getProfile("Default"));
    Player     *player = new Player(page, true);
    page->setUrl(QUrl::fromUserInput(PLAYER_WEBAPP_URL));
    player->show();
}

int main(int argc, char *argv[]) {
    SingleApplication a(argc, argv);

    QObject::connect(&a, &SingleApplication::instanceStarted, runningInstance);

    trayIcon    = new QSystemTrayIcon;
    profileList = new ProfileList;

    PlayerPage *page   = new PlayerPage(profileList->getProfile("Default"));
    Player     *player = new Player(page, true);
    page->setUrl(QUrl::fromUserInput(PLAYER_WEBAPP_URL));
    player->show();

    int ret = a.exec();

    delete profileList;
    delete trayIcon;
    return ret;
}

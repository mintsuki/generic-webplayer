#include "player.h"
#include <QApplication>
#include <QMessageBox>
#include <string>
#include <unistd.h>
#include <cstdio>
#include <csignal>

#include "config.h"

#define LOCK_FNAME  (("/tmp/" PLAYER_NAME "-lock." + std::to_string(getuid())).c_str())

[[noreturn]] static void signal_handler(int s) {
    (void)s;
    delete profileList;
    delete trayIcon;
    remove(LOCK_FNAME);
    _exit(2);
}

int main(int argc, char *argv[]) {
    // Hook handler for SIGINT and SIGTERM, so in case of Ctrl+C or similar we delete the lock.
    struct sigaction sig;
    sig.sa_handler = signal_handler;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;
    sigaction(SIGINT,  &sig, nullptr);
    sigaction(SIGTERM, &sig, nullptr);

    QApplication a(argc, argv);

    // Check the lock for other open instances
    if (access(LOCK_FNAME, F_OK) != -1) {
        // The lock is already acquired
        QMessageBox::critical(nullptr, "Application running",
                                       "Another instance of " PLAYER_NICE_NAME " was detected running.",
                                       QMessageBox::Ok);
        return 1;
    } else {
        FILE *lock = fopen(LOCK_FNAME, "w");
        if (lock == nullptr) {
            // Something happened
            return 2;
        }
        fclose(lock);
    }

    trayIcon    = new QSystemTrayIcon;
    profileList = new ProfileList;

    PlayerPage *page   = new PlayerPage(profileList->getProfile("Default"));
    Player     *player = new Player(page, true);
    page->setUrl(QUrl::fromUserInput(PLAYER_WEBAPP_URL));
    player->show();

    int ret = a.exec();

    delete profileList;
    delete trayIcon;
    remove(LOCK_FNAME);
    return ret;
}

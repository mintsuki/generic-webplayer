#include "player.h"
#include <QApplication>
#include <QMessageBox>
#include <string>
#include <unistd.h>
#include <cstdio>
#include <csignal>

#include "config.h"

std::string LOCK_FNAME;
std::string user_id;
std::string process_id;

[[noreturn]] static void signal_handler(int s) {
    (void)s;
    delete profileList;
    delete trayIcon;
    remove(LOCK_FNAME.c_str());
    _exit(2);
}

int main(int argc, char *argv[]) {
    user_id = std::to_string(getuid());
    process_id = std::to_string(getpid());
    LOCK_FNAME = "/tmp/" PLAYER_NAME "-lock." + user_id;

    // Hook handler for SIGINT and SIGTERM, so in case of Ctrl+C or similar we delete the lock.
    struct sigaction sig;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;
    sig.sa_handler = signal_handler;
    sigaction(SIGINT,  &sig, nullptr);
    sigaction(SIGTERM, &sig, nullptr);

    QApplication a(argc, argv);

    // Check the lock for other open instances
    if (access(LOCK_FNAME.c_str(), F_OK) != -1) {
        // The lock is already acquired
        FILE *lock = fopen(LOCK_FNAME.c_str(), "r");
        if (lock == nullptr) {
            // Something happened
            return 2;
        }
        char pid_s[8];
        fgets(pid_s, 8, lock);
        int pid = std::stoi(pid_s);
        fclose(lock);
        qDebug() << "Sending SIGUSR1 to pid" << pid;
        kill(pid, SIGUSR1);
        return 0;
    } else {
        FILE *lock = fopen(LOCK_FNAME.c_str(), "w");
        if (lock == nullptr) {
            // Something happened
            return 2;
        }
        fputs(process_id.c_str(), lock);
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
    remove(LOCK_FNAME.c_str());
    return ret;
}

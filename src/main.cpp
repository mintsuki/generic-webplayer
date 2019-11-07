#include "player.h"
#include <QApplication>
#include <QMessageBox>
#include <string>
#include <unistd.h>
#include <cstdio>
#include <csignal>

#define LOCK_FNAME       (("/tmp/@@player_name@@-lock." + std::to_string(getuid())).c_str())

[[noreturn]] static void sigint_handler(int a) {
    qInfo() << "Caught signal: " << a;
    remove(LOCK_FNAME);
    exit(1);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Check the lock for other open instances
    if (access(LOCK_FNAME, F_OK) != -1) {
        // The lock is already acquired
        QMessageBox::critical(nullptr, "Application running",
                                       "Another instance of @@player_nice_name@@ was detected running.",
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

    // Hook lock for SIGINT, so in case of Ctrl+C or similar we delete the lock.
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sigint_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);

    player *w;
    const char *baseUrl;
    if (argc > 1) {
        baseUrl = argv[1];
        w = new player(baseUrl, false);
    } else {
        baseUrl = "@@webapp_url@@";
        w = new player(baseUrl, true);
    }

    w->show();
    int ret = a.exec();

    delete w;
    remove(LOCK_FNAME);
    return ret;
}

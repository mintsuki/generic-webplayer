#include "player.h"
#include <QApplication>
#include <QMessageBox>
#include <string>
#include <unistd.h>
#include <cstdio>
#include <csignal>

#define LOCK_FNAME  (("/tmp/@@player_name@@-lock." + std::to_string(getuid())).c_str())

[[noreturn]] static void signal_handler(int s) {
    (void)s;
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

    Player *w;
    if (argc > 1) {
        w = new Player(argv[1], false);
    } else {
        w = new Player("@@webapp_url@@", true);
    }
    w->show();
    int ret = a.exec();
    delete w;

    remove(LOCK_FNAME);
    return ret;
}

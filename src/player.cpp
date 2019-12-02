#include "player.h"
#include "ui_player.h"

#include "playerwebdialog.h"

#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDir>
#include <QWebEngineNotification>
#include <QProcess>
#include <unistd.h>

PlayerPage::PlayerPage(QWebEngineProfile *profile, bool openBrowser, QObject *parent) : QWebEnginePage(profile, parent) {
    this->openBrowser = openBrowser;
}

QWebEnginePage *PlayerPage::createWindow(QWebEnginePage::WebWindowType type) {
    qDebug() << "createWindow type: " << type;

    foreach (DummyPage *p, pagesToDestroy) {
        delete p;
    }
    pagesToDestroy.clear();

    switch (type) {
        case QWebEnginePage::WebDialog: {
            WebDialogPage *p   = new WebDialogPage(profile());
            PlayerWebDialog *w = new PlayerWebDialog(p);
            w->show();
            return p;
        }
        default: {
            if (openBrowser) {
                DummyPage *p = new DummyPage(profile());
                pagesToDestroy.push_back(p);
                return p;
            } else {
                PlayerPage *p  = new PlayerPage(profile(), openBrowser);
                Player *player = new Player(QUrl("@@webapp_url@@"), openBrowser, p);
                player->show();
                return p;
            }
        }
    }
}

PlayerPage::~PlayerPage() {
    foreach (DummyPage *p, pagesToDestroy) {
        delete p;
    }
}

DummyPage::DummyPage(QWebEngineProfile *profile, QObject *parent) : QWebEnginePage(profile, parent) {}

bool DummyPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) {
    qDebug() << "acceptNavigationRequest url: " << url << " type: " << type << " isMainFrame: " << isMainFrame;

    QDesktopServices::openUrl(url);
    return false;
}

QIcon *playerIcon = nullptr;
QSystemTrayIcon *trayIcon = nullptr;

static void showNotification(std::unique_ptr<QWebEngineNotification> n) {
    QProcess p;
    p.start("notify-send",
            QStringList()
            << "--app-name=@@player_nice_name@@"
            << n->title()
            << n->message());
    if (!p.waitForStarted()) {
        trayIcon->showMessage(n->title(), n->message());
    } else {
        p.waitForFinished();
    }
    n->show();
}

ProfileList::ProfileList() {
    QDir profilesDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QString("/QtWebEngine"));
    QStringList profiles = profilesDir.entryList();
    foreach (QString profile, profiles) {
        if (profile == "." || profile == "..") {
            continue;
        } else if (profile == "Default") {
            list.push_back(QWebEngineProfile::defaultProfile());
        } else {
            QWebEngineProfile *newProfile = new QWebEngineProfile(profile);
            list.push_back(newProfile);
        }
    }
    foreach (QWebEngineProfile *p, list) {
        p->setNotificationPresenter(showNotification);
    }
}

ProfileList::~ProfileList() {
    foreach (QWebEngineProfile *p, list) {
        if (p->storageName() != "Default") {
            delete p;
        }
    }
}

QWebEngineProfile *ProfileList::getProfile(const QString &name) {
    foreach (QWebEngineProfile *p, list) {
        if (p->storageName() == name) {
            return p;
        }
    }
    return nullptr;
}

void ProfileList::getProfileList() {
    emit profileListChanged(list);
}

QWebEngineProfile *ProfileList::newProfile(const QString &name) {
    QWebEngineProfile *p = new QWebEngineProfile(name);
    list.push_back(p);
    p->setNotificationPresenter(showNotification);
    emit profileListChanged(list);
    return p;
}

ProfileList *profileList = nullptr;

PlayerPage *Player::buildPage(const QString &profile, PlayerPage *page) {
    PlayerPage *newPage;

    if (page == nullptr) {
        QWebEngineProfile *p = profileList->getProfile(profile);
        if (p == nullptr)
            p = profileList->newProfile(profile);

        newPage = new PlayerPage(p, openBrowser);
    } else {
        newPage = page;
    }

    newPage->settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);

    connect(newPage,
            SIGNAL(featurePermissionRequested(const QUrl &, QWebEnginePage::Feature)),
            this,
            SLOT(grantFeaturePermission(const QUrl &, QWebEnginePage::Feature)));

    return newPage;
}

Player::Player(const QUrl &baseUrl, bool openBrowser, PlayerPage *initialPage, const QString &profile, QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::Player) {
    ui->setupUi(this);

    this->baseUrl     = baseUrl;
    this->openBrowser = openBrowser;

    setAttribute(Qt::WA_DeleteOnClose);
    showMaximized();
    ui->profilesBar->setVisible(false);

    connect(profileList,
            SIGNAL(profileListChanged(std::vector<QWebEngineProfile *>)),
            this,
            SLOT(profileListChanged(std::vector<QWebEngineProfile *>)));

    profileList->getProfileList();

    if (!initialPage) {
        ui->profileTextbox->setText(profile);
        PlayerPage *newPage = buildPage(profile);
        ui->webEngineView->setPage(newPage);
        ui->webEngineView->page()->setUrl(QUrl(baseUrl));
    } else {
        ui->profileTextbox->setText(initialPage->profile()->storageName());
        buildPage("", initialPage);
        ui->webEngineView->setPage(initialPage);
    }

    ui->openBrowserCheckbox->setCheckState(openBrowser ? Qt::Checked : Qt::Unchecked);
}

Player::~Player() {
    PlayerPage *oldPage = static_cast<PlayerPage *>(ui->webEngineView->page());
    delete ui;
    delete oldPage;
}

void Player::profileListChanged(std::vector<QWebEngineProfile *> list) {
    ui->profileListWidget->clear();
    foreach (QWebEngineProfile *profile, list) {
        ui->profileListWidget->addItem(profile->storageName());
    }
}

void Player::grantFeaturePermission(const QUrl &q, QWebEnginePage::Feature f) {
    qDebug() << q << f;

    if (f == QWebEnginePage::Notifications) {
        ui->webEngineView->page()->setFeaturePermission(q, f,
            QWebEnginePage::PermissionGrantedByUser);
        return;
    }

    QString s;
    QDebug d(&s);
    d << "Grant \"" << q << "\" permission to use \"" << f << "\"?";

    QMessageBox::StandardButton r;
    r = QMessageBox::question(this, "Grant permission", s,
            QMessageBox::Yes | QMessageBox::No);

    if (r == QMessageBox::Yes) {
        ui->webEngineView->page()->setFeaturePermission(q, f,
            QWebEnginePage::PermissionGrantedByUser);
    } else {
        ui->webEngineView->page()->setFeaturePermission(q, f,
            QWebEnginePage::PermissionDeniedByUser);
    }
}

void Player::on_webEngineView_titleChanged(const QString &title) {
    QString profile = ui->webEngineView->page()->profile()->storageName();
    setWindowTitle(title +
                   (profile == "Default"
                    ?  ""
                    : (" -- [Profile: " + profile + "]")
                   )
                  );
}

void Player::on_webEngineView_iconChanged(const QIcon &arg1) {
    setWindowIcon(arg1);
}

void Player::on_webEngineView_urlChanged(const QUrl &arg1) {
    ui->urlTextbox->setText(arg1.toString());
}

void Player::on_urlTextbox_returnPressed() {
    PlayerPage *p  = new PlayerPage(ui->webEngineView->page()->profile(), false);
    Player *player = new Player(baseUrl, false, p);
    p->setUrl(QUrl::fromUserInput(ui->urlTextbox->text()));
    player->show();
}

void Player::on_backButton_clicked() {
    ui->webEngineView->back();
}

void Player::on_forwardsButton_clicked() {
    ui->webEngineView->forward();
}

void Player::on_openBrowserCheckbox_stateChanged(int arg1) {
    bool b = (arg1 == Qt::Checked);
    openBrowser = b;
    static_cast<PlayerPage *>(ui->webEngineView->page())->openBrowser = b;
}

void Player::toggleProfilesBar() {
    if (ui->profilesBar->isVisible()) {
        ui->profilesButton->setText("Show profiles");
        ui->profilesBar->setVisible(false);
    } else {
        ui->profilesButton->setText("Hide profiles");
        ui->profilesBar->setVisible(true);
    }
}

void Player::on_profileTextbox_returnPressed() {
    Player *w = new Player(baseUrl, openBrowser, nullptr, ui->profileTextbox->text());
    ui->profileTextbox->setText(ui->webEngineView->page()->profile()->storageName());
    w->show();
    toggleProfilesBar();
}

void Player::on_profilesButton_clicked() {
    toggleProfilesBar();
}

void Player::on_newWindowButton_clicked() {
    Player *w = new Player(baseUrl, openBrowser, nullptr, ui->profileTextbox->text());
    w->show();
}

void Player::on_profileListWidget_itemDoubleClicked(QListWidgetItem *item) {
    Player *w = new Player(baseUrl, openBrowser, nullptr, item->text());
    w->show();
    toggleProfilesBar();
}

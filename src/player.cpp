#include "player.h"
#include "ui_player.h"

#include "playerwebdialog.h"

#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDir>
#include <vector>

PlayerPage::PlayerPage(QWebEngineProfile *profile, QObject *parent) : QWebEnginePage(profile, parent) {}

QWebEnginePage *PlayerPage::createWindow(QWebEnginePage::WebWindowType type) {
    qDebug() << "createWindow type: " << type;
    switch (type) {
        case QWebEnginePage::WebBrowserWindow:
        case QWebEnginePage::WebBrowserTab:
        case QWebEnginePage::WebBrowserBackgroundTab:
            return new PlayerPage(profile(), parent());
        case QWebEnginePage::WebDialog: {
            WebDialogPage *p   = new WebDialogPage(profile());
            PlayerWebDialog *w = new PlayerWebDialog(p);
            w->show();
            return p;
        }
    }
}

bool PlayerPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) {
    qDebug() << "acceptNavigationRequest url: " << url << " type: " << type << " isMainFrame: " << isMainFrame;

    PlayerPage *oldPage = static_cast<PlayerPage *>(static_cast<Player *>(parent())->ui->webEngineView->page());
    if (oldPage != this) {
        if (static_cast<Player *>(parent())->openBrowser) {
            QDesktopServices::openUrl(url);
            delete this;
            return false;
        } else {
            static_cast<Player *>(parent())->ui->webEngineView->setPage(this);
            delete oldPage;
        }
    }

    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
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

std::vector<QWebEngineProfile *> ProfileList::getProfileList() {
    return list;
}

QWebEngineProfile *ProfileList::newProfile(const QString &name) {
    QWebEngineProfile *p = new QWebEngineProfile(name);
    list.push_back(p);
    return p;
}

ProfileList *profileList = nullptr;

void Player::refreshProfileList() {
    ui->listWidget->clear();
    std::vector<QWebEngineProfile *> list = profileList->getProfileList();
    foreach (QWebEngineProfile *profile, list) {
        ui->listWidget->addItem(profile->storageName());
    }
}

PlayerPage *Player::buildPage(const QString &profile) {
    QWebEngineProfile *p = profileList->getProfile(profile);

    if (p == nullptr)
        p = profileList->newProfile(profile);

    PlayerPage *newPage = new PlayerPage(p, this);

    newPage->settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);

    connect(newPage,
            SIGNAL(featurePermissionRequested(const QUrl &, QWebEnginePage::Feature)),
            this,
            SLOT(grantFeaturePermission(const QUrl &, QWebEnginePage::Feature)));

    return newPage;
}

Player::Player(const char *baseUrl_arg, bool openBrowser_arg, QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::Player) {
    ui->setupUi(this);

    baseUrl = baseUrl_arg;
    openBrowser = openBrowser_arg;

    showMaximized();

    isProfileListVisible = false;
    ui->listWidget->setVisible(false);
    refreshProfileList();

    ui->lineEdit->setText("Default");
    PlayerPage *newPage = buildPage(ui->lineEdit->text());
    ui->webEngineView->setPage(newPage);
    ui->webEngineView->page()->setUrl(QUrl(baseUrl));
}

Player::~Player() {
    PlayerPage *oldPage = static_cast<PlayerPage *>(ui->webEngineView->page());
    delete ui;
    delete oldPage;
}

void Player::grantFeaturePermission(const QUrl &q, QWebEnginePage::Feature f) {
    qDebug() << q << f;

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
    setWindowTitle(title);
}

void Player::on_webEngineView_iconChanged(const QIcon &arg1) {
    setWindowIcon(arg1);
}

void Player::on_lineEdit_returnPressed() {
    PlayerPage *oldPage = static_cast<PlayerPage *>(ui->webEngineView->page());

    PlayerPage *newPage = buildPage(ui->lineEdit->text());
    ui->webEngineView->setPage(newPage);
    ui->webEngineView->page()->setUrl(QUrl(baseUrl));

    refreshProfileList();

    delete oldPage;
}

void Player::on_pushButton_pressed() {
    isProfileListVisible = !isProfileListVisible;
    ui->listWidget->setVisible(isProfileListVisible);
}

void Player::on_listWidget_itemDoubleClicked(QListWidgetItem *item) {
    PlayerPage *oldPage = static_cast<PlayerPage *>(ui->webEngineView->page());

    PlayerPage *newPage = buildPage(item->text());
    ui->webEngineView->setPage(newPage);
    ui->webEngineView->page()->setUrl(QUrl(baseUrl));

    ui->lineEdit->setText(item->text());

    delete oldPage;
}

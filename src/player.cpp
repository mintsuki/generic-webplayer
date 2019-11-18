#include "player.h"
#include "ui_player.h"

#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDir>
#include <libgen.h>

PlayerPage::PlayerPage(Player *parentPlayer, QWebEngineProfile *profile, QObject *parent) : QWebEnginePage(profile, parent) {
    this->parentPlayer = parentPlayer;
}

QWebEnginePage *PlayerPage::createWindow(QWebEnginePage::WebWindowType type) {
    qDebug() << "createWindow type: " << type;
    return parentPlayer->globalPageToGetClickUrl;
}

bool PlayerPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) {
    qDebug() << "acceptNavigationRequest url: " << url << " type: " << type << " isMainFrame: " << isMainFrame;
    if (isMainFrame) {
        // Exception for base link
        if (parentPlayer->openBrowser == false || url == QUrl(parentPlayer->baseUrl))
            goto pass;
        QDesktopServices::openUrl(url);
        return false;
    }

pass:
    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

void Player::refreshProfileList() {
    QString profilesPath(dirname(const_cast<char *>(ui->webEngineView->page()->profile()->persistentStoragePath().toStdString().c_str())));
    QDir profilesDir(profilesPath);
    QStringList profiles = profilesDir.entryList();
    ui->listWidget->clear();
    foreach (QString profile, profiles) {
        if (profile == "." || profile == "..")
            continue;
        ui->listWidget->addItem(profile);
    }
}

PlayerPage *Player::buildPage(const QString &profile) {
    QWebEngineProfile *newProfile = new QWebEngineProfile(profile);
    PlayerPage *newPage = new PlayerPage(this, newProfile);

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
    QWebEngineProfile *newProfile = new QWebEngineProfile();
    globalPageToGetClickUrl = new PlayerPage(this, newProfile);

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
    QWebEngineProfile *oldProfile = globalPageToGetClickUrl->profile();
    delete globalPageToGetClickUrl;
    delete oldProfile;
    PlayerPage *oldPage = static_cast<PlayerPage *>(ui->webEngineView->page());
    oldProfile = oldPage->profile();
    delete oldPage;
    delete oldProfile;
    delete ui;
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
    QWebEngineProfile *oldProfile = oldPage->profile();

    PlayerPage *newPage = buildPage(ui->lineEdit->text());
    ui->webEngineView->setPage(newPage);
    ui->webEngineView->page()->setUrl(QUrl(baseUrl));

    refreshProfileList();

    delete oldPage;
    delete oldProfile;
}

void Player::on_pushButton_pressed() {
    isProfileListVisible = !isProfileListVisible;
    ui->listWidget->setVisible(isProfileListVisible);
}

void Player::on_listWidget_itemDoubleClicked(QListWidgetItem *item) {
    PlayerPage *oldPage = static_cast<PlayerPage *>(ui->webEngineView->page());
    QWebEngineProfile *oldProfile = oldPage->profile();

    PlayerPage *newPage = buildPage(item->text());
    ui->webEngineView->setPage(newPage);
    ui->webEngineView->page()->setUrl(QUrl(baseUrl));

    ui->lineEdit->setText(item->text());

    delete oldPage;
    delete oldProfile;
}

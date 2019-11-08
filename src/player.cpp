#include "player.h"
#include "ui_player.h"

#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QDesktopServices>
#include <QMessageBox>

static const char *baseUrl;
static bool openBrowser;

static PlayerPage *globalPageToGetClickUrl;

PlayerPage::PlayerPage(QWebEngineProfile *profile, QObject *parent) : QWebEnginePage(profile, parent) {}
PlayerPage::PlayerPage(QObject *parent) : QWebEnginePage(parent) {}

QWebEnginePage *PlayerPage::createWindow(QWebEnginePage::WebWindowType type) {
    qDebug() << "createWindow type: " << type;
    return globalPageToGetClickUrl;
}

bool PlayerPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) {
    qDebug() << "acceptNavigationRequest url: " << url << " type: " << type << " isMainFrame: " << isMainFrame;
    if (isMainFrame) {
        // Exception for base link
        if (openBrowser == false || url == QUrl(baseUrl))
            goto pass;
        QDesktopServices::openUrl(url);
        return false;
    }

pass:
    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

PlayerPage *player::buildPage(const QString &profile) {
    QWebEngineProfile *newProfile = new QWebEngineProfile(profile);
    PlayerPage *newPage = new PlayerPage(newProfile);

    newPage->settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);

    connect(newPage,
            SIGNAL(featurePermissionRequested(const QUrl &, QWebEnginePage::Feature)),
            this,
            SLOT(grantFeaturePermission(const QUrl &, QWebEnginePage::Feature)));

    return newPage;
}

player::player(const char *baseUrl_arg, bool openBrowser_arg, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::player)
{
    ui->setupUi(this);

    baseUrl = baseUrl_arg;
    openBrowser = openBrowser_arg;
    globalPageToGetClickUrl = new PlayerPage();

    showMaximized();

    ui->lineEdit->setText("Default");
    PlayerPage *newPage = buildPage(ui->lineEdit->text());
    ui->webEngineView->setPage(newPage);
    ui->webEngineView->page()->setUrl(QUrl(baseUrl));
}

player::~player()
{
    delete globalPageToGetClickUrl;
    PlayerPage *oldPage = static_cast<PlayerPage *>(ui->webEngineView->page());
    QWebEngineProfile *oldProfile = oldPage->profile();
    delete oldPage;
    delete oldProfile;
    delete ui;
}

void player::grantFeaturePermission(const QUrl &q, QWebEnginePage::Feature f) {
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

void player::on_webEngineView_titleChanged(const QString &title) {
    setWindowTitle(title);
}

void player::on_webEngineView_iconChanged(const QIcon &arg1) {
    setWindowIcon(arg1);
}

void player::on_lineEdit_returnPressed() {
    PlayerPage *oldPage = static_cast<PlayerPage *>(ui->webEngineView->page());
    QWebEngineProfile *oldProfile = oldPage->profile();

    PlayerPage *newPage = buildPage(ui->lineEdit->text());
    ui->webEngineView->setPage(newPage);
    ui->webEngineView->page()->setUrl(QUrl(baseUrl));

    delete oldPage;
    delete oldProfile;
}

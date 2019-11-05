#include "player.h"
#include "ui_player.h"

#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QDir>
#include <QFile>

static const char *baseUrl;

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
        if (url == QUrl(baseUrl))
            goto pass;
        QDesktopServices::openUrl(url);
        return false;
    }

pass:
    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

player::player(const char *baseUrl_arg, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::player)
{
    baseUrl = baseUrl_arg;

    ui->setupUi(this);

    globalPageToGetClickUrl = new PlayerPage();

    PlayerPage *DPage = new PlayerPage();
    ui->webEngineView->setPage(DPage);

    showMaximized();

    connect(ui->webEngineView->page(),
            SIGNAL(featurePermissionRequested(const QUrl &, QWebEnginePage::Feature)),
            this,
            SLOT(grantFeaturePermission(const QUrl &, QWebEnginePage::Feature)));

    ui->webEngineView->page()->settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);

    ui->webEngineView->page()->setUrl(QUrl(baseUrl));
}

player::~player()
{
    delete globalPageToGetClickUrl;
    delete ui->webEngineView->page();
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

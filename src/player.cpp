#include "player.h"
#include "ui_player.h"

#include "playerwebdialog.h"

#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDir>

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
    ui->profileListWidget->clear();
    std::vector<QWebEngineProfile *> list = profileList->getProfileList();
    foreach (QWebEngineProfile *profile, list) {
        ui->profileListWidget->addItem(profile->storageName());
    }
}

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

    isProfileListVisible = false;
    ui->profileListWidget->setVisible(false);
    refreshProfileList();

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

void Player::on_profileTextbox_returnPressed() {
    PlayerPage *oldPage = static_cast<PlayerPage *>(ui->webEngineView->page());

    PlayerPage *newPage = buildPage(ui->profileTextbox->text());
    ui->webEngineView->setPage(newPage);
    ui->webEngineView->page()->setUrl(QUrl(baseUrl));

    refreshProfileList();

    delete oldPage;
}

void Player::on_profilesButton_clicked() {
    isProfileListVisible = !isProfileListVisible;
    ui->profileListWidget->setVisible(isProfileListVisible);
}

void Player::on_newWindowButton_clicked() {
    Player *w = new Player(baseUrl, openBrowser, nullptr, ui->profileTextbox->text());
    w->show();
}

void Player::on_profileListWidget_itemDoubleClicked(QListWidgetItem *item) {
    PlayerPage *oldPage = static_cast<PlayerPage *>(ui->webEngineView->page());

    PlayerPage *newPage = buildPage(item->text());
    ui->webEngineView->setPage(newPage);
    ui->webEngineView->page()->setUrl(QUrl(baseUrl));

    ui->profileTextbox->setText(item->text());

    delete oldPage;
}

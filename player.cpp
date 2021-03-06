#include "player.h"
#include "ui_player.h"

#include "playerwebdialog.h"

#include <QtGlobal>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDir>
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
#   include <QWebEngineNotification>
#endif
#include <QProcess>
#include <QAction>
#include <QFileDialog>

#include "config.h"

PlayerPage::PlayerPage(QWebEngineProfile *profile, Player *parentPlayer, QObject *parent) : QWebEnginePage(profile, parent) {
    qDebug() << profile;
    this->parentPlayer = parentPlayer;
}

QWebEnginePage *PlayerPage::createWindow(QWebEnginePage::WebWindowType type) {
    qDebug() << "createWindow type: " << type;

    switch (type) {
        case QWebEnginePage::WebDialog: {
            WebDialogPage *p   = new WebDialogPage(profile());
            PlayerWebDialog *w = new PlayerWebDialog(p);
            w->show();
            return p;
        }
        default: {
            if (parentPlayer->ui->openBrowserCheckbox->isChecked()) {
                DummyPage *p = new DummyPage(profile());
                return p;
            } else {
                Player *player = new Player(profile(), QUrl("about:blank"), false);
                player->show();
                return player->ui->webEngineView->page();
            }
        }
    }
}

QStringList PlayerPage::chooseFiles(QWebEnginePage::FileSelectionMode mode,
                                    const QStringList &oldFiles,
                                    const QStringList &acceptedMimeTypes) {
    qDebug() << "chooseFiles" << mode << oldFiles << acceptedMimeTypes;

    if (acceptedMimeTypes.isEmpty()) {
        switch (mode) {
            case QWebEnginePage::FileSelectOpenMultiple:
                return QFileDialog::getOpenFileNames();
            case QWebEnginePage::FileSelectOpen: {
                QStringList l;
                l << QFileDialog::getOpenFileName();
                return l;
            }
        }
    }

    return QWebEnginePage::chooseFiles(mode, oldFiles, acceptedMimeTypes);
}

DummyPage::DummyPage(QWebEngineProfile *profile, QObject *parent) : QWebEnginePage(profile, parent) {}

bool DummyPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) {
    qDebug() << "acceptNavigationRequest url: " << url << " type: " << type << " isMainFrame: " << isMainFrame;

    QDesktopServices::openUrl(url);

    deleteLater();
    return false;
}

QSystemTrayIcon *trayIcon = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)

static void showNotification(std::unique_ptr<QWebEngineNotification> n) {
    QProcess p;
    p.start("notify-send",
            QStringList()
            << "--app-name=" PLAYER_NICE_NAME
            << n->title()
            << n->message());
    if (!p.waitForStarted()) {
        trayIcon->show();
        trayIcon->showMessage(n->title(), n->message());
        trayIcon->hide();
    } else {
        p.waitForFinished();
    }
    n->show();
}

#endif

ProfileList::ProfileList() {
    #if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
        QWebEngineProfile::defaultProfile()->setNotificationPresenter(showNotification);
    #endif
    list.push_back(QWebEngineProfile::defaultProfile());

    QDir profilesDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QString("/QtWebEngine"));
    if (!profilesDir.exists())
        return;

    QStringList profiles = profilesDir.entryList();

    foreach (QString profile, profiles) {
        if (profile != "." && profile != ".." && profile != "Default") {
            QWebEngineProfile *newProfile = new QWebEngineProfile(profile);
            #if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
                newProfile->setNotificationPresenter(showNotification);
            #endif
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

    QWebEngineProfile *p = new QWebEngineProfile(name);
    list.push_back(p);
    #if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
        p->setNotificationPresenter(showNotification);
    #endif
    emit profileListChanged(list);
    return p;
}

void ProfileList::getProfileList() {
    emit profileListChanged(list);
}

ProfileList *profileList = nullptr;

Player::Player(QWebEngineProfile *profile, QUrl url, bool openBrowser, QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::Player) {
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    PlayerPage *page = new PlayerPage(profile, this);
    ui->webEngineView->setPage(page);

    showMaximized();
    ui->profilesBar->setVisible(false);

    connect(profileList, &ProfileList::profileListChanged, this, &Player::profileListChanged);
    profileList->getProfileList();

    ui->profileTextbox->setText(profile->storageName());

    page->settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);

    #if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        page->settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
    #endif

    connect(page, &PlayerPage::featurePermissionRequested, this, &Player::grantFeaturePermission);

    ui->webEngineView->setPage(page);

    ui->openBrowserCheckbox->setCheckState(openBrowser ? Qt::Checked : Qt::Unchecked);

    page->setUrl(url);
}

Player::~Player() {
    PlayerPage *page = static_cast<PlayerPage *>(ui->webEngineView->page());
    delete page;
    delete ui;
}

void Player::profileListChanged(QVector<QWebEngineProfile *> list) {
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
    updateNavigationButtons();
}

void Player::on_webEngineView_iconChanged(const QIcon &arg1) {
    setWindowIcon(arg1);
}

void Player::on_webEngineView_urlChanged(const QUrl &arg1) {
    ui->urlTextbox->setText(arg1.toString());
    updateNavigationButtons();
}

void Player::on_webEngineView_loadFinished(bool) {
    updateNavigationButtons();
}

void Player::on_urlTextbox_returnPressed() {
    new Player(ui->webEngineView->page()->profile(), QUrl::fromUserInput(ui->urlTextbox->text()), false);
}

void Player::on_backButton_clicked() {
    ui->webEngineView->back();
}

void Player::on_forwardsButton_clicked() {
    ui->webEngineView->forward();
}

void Player::updateNavigationButtons() {
    ui->backButton->setEnabled(ui->webEngineView->pageAction(QWebEnginePage::Back)->isEnabled());
    ui->forwardsButton->setEnabled(ui->webEngineView->pageAction(QWebEnginePage::Forward)->isEnabled());
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
    new Player(profileList->getProfile(ui->profileTextbox->text()));
    ui->profileTextbox->setText(ui->webEngineView->page()->profile()->storageName());
    toggleProfilesBar();
}

void Player::on_profilesButton_clicked() {
    toggleProfilesBar();
}

void Player::on_profileListWidget_itemDoubleClicked(QListWidgetItem *item) {
    new Player(profileList->getProfile(item->text()));
    toggleProfilesBar();
}

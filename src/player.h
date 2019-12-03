#ifndef PLAYER_H
#define PLAYER_H

#include <QMainWindow>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QListWidgetItem>
#include <QSystemTrayIcon>
#include <vector>

class ProfileList : public QObject {
    Q_OBJECT

public:
    explicit ProfileList();
    ~ProfileList();

    QWebEngineProfile *getProfile(const QString &name);
    void getProfileList();

private:
    std::vector<QWebEngineProfile *> list;

signals:
    void profileListChanged(std::vector<QWebEngineProfile *> list);
};

extern ProfileList *profileList;
extern QIcon *playerIcon;
extern QSystemTrayIcon *trayIcon;

class DummyPage : public QWebEnginePage {
    Q_OBJECT

public:
    explicit DummyPage(QWebEngineProfile *profile, QObject *parent = nullptr);

protected:
    bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) override;
};

class Player;

class PlayerPage : public QWebEnginePage {
    Q_OBJECT

public:
    explicit PlayerPage(QWebEngineProfile *profile, QObject *parent = nullptr);
    ~PlayerPage() override;

    void setParentPlayer(Player *parentPlayer);

protected:
    QWebEnginePage *createWindow(QWebEnginePage::WebWindowType type) override;

private:
    std::vector<DummyPage *> pagesToDestroy;
    Player *parentPlayer;
};

QT_BEGIN_NAMESPACE
namespace Ui { class Player; }
QT_END_NAMESPACE

class Player : public QMainWindow {
    Q_OBJECT

public:
    explicit Player(PlayerPage *initialPage, bool openBrowser, QWidget *parent = nullptr);
    ~Player();

    Ui::Player *ui;

private slots:
    void on_webEngineView_titleChanged(const QString &title);
    void on_webEngineView_iconChanged(const QIcon &arg1);
    void on_webEngineView_urlChanged(const QUrl &arg1);
    void on_urlTextbox_returnPressed();
    void on_backButton_clicked();
    void on_forwardsButton_clicked();
    void grantFeaturePermission(const QUrl &q, QWebEnginePage::Feature f);
    void on_profileTextbox_returnPressed();
    void on_profilesButton_clicked();
    void on_profileListWidget_itemDoubleClicked(QListWidgetItem *item);
    void profileListChanged(std::vector<QWebEngineProfile *> list);

private:
    void toggleProfilesBar();
};

#endif // PLAYER_H

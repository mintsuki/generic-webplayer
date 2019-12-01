#ifndef PLAYER_H
#define PLAYER_H

#include <QMainWindow>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QListWidgetItem>
#include <vector>

class ProfileList : public QObject {
    Q_OBJECT

public:
    explicit ProfileList();
    ~ProfileList();

    QWebEngineProfile *getProfile(const QString &name);
    void getProfileList();
    QWebEngineProfile *newProfile(const QString &name);

private:
    std::vector<QWebEngineProfile *> list;

signals:
    void profileListChanged(std::vector<QWebEngineProfile *> list);
};

extern ProfileList *profileList;

class DummyPage : public QWebEnginePage {
    Q_OBJECT

public:
    explicit DummyPage(QWebEngineProfile *profile, QObject *parent = nullptr);

protected:
    bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) override;
};

class PlayerPage : public QWebEnginePage {
    Q_OBJECT

public:
    explicit PlayerPage(QWebEngineProfile *profile, bool openBrowser, QObject *parent = nullptr);
    ~PlayerPage() override;

    bool openBrowser;

protected:
    QWebEnginePage *createWindow(QWebEnginePage::WebWindowType type) override;

private:
    std::vector<DummyPage *> pagesToDestroy;
};

QT_BEGIN_NAMESPACE
namespace Ui { class Player; }
QT_END_NAMESPACE

class Player : public QMainWindow {
    Q_OBJECT

public:
    explicit Player(const QUrl &baseUrl,
                    bool openBrowser,
                    PlayerPage *initialPage = nullptr,
                    const QString &profile = "Default",
                    QWidget *parent = nullptr);
    ~Player();

private slots:
    void on_webEngineView_titleChanged(const QString &title);
    void on_webEngineView_iconChanged(const QIcon &arg1);
    void on_webEngineView_urlChanged(const QUrl &arg1);
    void on_urlTextbox_returnPressed();
    void on_backButton_clicked();
    void on_forwardsButton_clicked();
    void on_openBrowserCheckbox_stateChanged(int arg1);
    void grantFeaturePermission(const QUrl &q, QWebEnginePage::Feature f);
    void on_profileTextbox_returnPressed();
    void on_profilesButton_clicked();
    void on_newWindowButton_clicked();
    void on_profileListWidget_itemDoubleClicked(QListWidgetItem *item);
    void profileListChanged(std::vector<QWebEngineProfile *> list);

private:
    PlayerPage *buildPage(const QString &profile, PlayerPage *page = nullptr);
    void toggleProfilesBar();

    Ui::Player *ui;
    QUrl baseUrl;
    bool openBrowser;
};

#endif // PLAYER_H

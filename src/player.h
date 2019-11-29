#ifndef PLAYER_H
#define PLAYER_H

#include <QMainWindow>
#include <QWebEnginePage>
#include <QListWidgetItem>

class ProfileList {
public:
    explicit ProfileList();
    ~ProfileList();

    QWebEngineProfile *getProfile(const QString &name);
    std::vector<QWebEngineProfile *> getProfileList();
    QWebEngineProfile *newProfile(const QString &name);

private:
    std::vector<QWebEngineProfile *> list;
};

extern ProfileList *profileList;

class PlayerPage : public QWebEnginePage {
    Q_OBJECT

public:
    explicit PlayerPage(QWebEngineProfile *profile, QObject *parent);

protected:
    QWebEnginePage *createWindow(QWebEnginePage::WebWindowType type) override;
    bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) override;
};

QT_BEGIN_NAMESPACE
namespace Ui { class Player; }
QT_END_NAMESPACE

class Player : public QMainWindow {
    Q_OBJECT

public:
    explicit Player(const char *baseUrl, bool openBrowser, const QString &profile = "Default", QWidget *parent = nullptr);
    ~Player();

    Ui::Player *ui;
    const char *baseUrl;
    bool openBrowser;

private slots:
    void on_webEngineView_titleChanged(const QString &title);
    void on_webEngineView_iconChanged(const QIcon &arg1);
    void grantFeaturePermission(const QUrl &q, QWebEnginePage::Feature f);
    void on_lineEdit_returnPressed();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

private:
    PlayerPage *buildPage(const QString &profile);
    void refreshProfileList();

    bool isProfileListVisible;
};

#endif // PLAYER_H

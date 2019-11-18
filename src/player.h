#ifndef PLAYER_H
#define PLAYER_H

#include <QMainWindow>
#include <QWebEnginePage>
#include <QListWidgetItem>

class Player;

class PlayerPage : public QWebEnginePage {
    Q_OBJECT
public:
    explicit PlayerPage(Player *parentPlayer, QWebEngineProfile *profile, QObject *parent = Q_NULLPTR);
protected:
    QWebEnginePage *createWindow(QWebEnginePage::WebWindowType type) override;
    bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) override;
private:
    Player *parentPlayer;
};

QT_BEGIN_NAMESPACE
namespace Ui { class Player; }
QT_END_NAMESPACE

class Player : public QMainWindow {
    Q_OBJECT

public:
    explicit Player(const char *baseUrl, bool openBrowser, QWidget *parent = nullptr);
    ~Player();

    PlayerPage *globalPageToGetClickUrl;
    const char *baseUrl;
    bool openBrowser;

private slots:
    void on_webEngineView_titleChanged(const QString &title);
    void on_webEngineView_iconChanged(const QIcon &arg1);
    void grantFeaturePermission(const QUrl &q, QWebEnginePage::Feature f);
    void on_lineEdit_returnPressed();
    void on_pushButton_pressed();
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

private:
    PlayerPage *buildPage(const QString &profile);
    void refreshProfileList();

    Ui::Player *ui;
    bool isProfileListVisible;
};

#endif // PLAYER_H

#ifndef PLAYER_H
#define PLAYER_H

#include <QMainWindow>
#include <QWebEnginePage>

class player;

class PlayerPage : public QWebEnginePage {
    Q_OBJECT
public:
    explicit PlayerPage(player *parentPlayer, QWebEngineProfile *profile, QObject *parent = Q_NULLPTR);
protected:
    QWebEnginePage *createWindow(QWebEnginePage::WebWindowType type) override;
    bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) override;
private:
    player *parentPlayer;
};

namespace Ui {
class player;
}

class player : public QMainWindow {
    Q_OBJECT

public:
    explicit player(const char *baseUrl, bool openBrowser, QWidget *parent = nullptr);
    ~player();

    PlayerPage *globalPageToGetClickUrl;
    const char *baseUrl;
    bool openBrowser;

private slots:
    void on_webEngineView_titleChanged(const QString &title);
    void on_webEngineView_iconChanged(const QIcon &arg1);
    void grantFeaturePermission(const QUrl &q, QWebEnginePage::Feature f);

    void on_lineEdit_returnPressed();

private:
    Ui::player *ui;
    PlayerPage *buildPage(const QString &profile);
};

#endif // PLAYER_H

#ifndef PLAYER_H
#define PLAYER_H

#include <QMainWindow>
#include <QWebEnginePage>

class PlayerPage : public QWebEnginePage {
    Q_OBJECT
public:
    PlayerPage(QWebEngineProfile *profile, QObject *parent = Q_NULLPTR);
    PlayerPage(QObject *parent = Q_NULLPTR);
protected:
    QWebEnginePage *createWindow(QWebEnginePage::WebWindowType type) override;
    bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) override;
};

namespace Ui {
class player;
}

class player : public QMainWindow {
    Q_OBJECT

public:
    explicit player(const char *baseUrl, QWidget *parent = nullptr);
    ~player();

private slots:
    void on_webEngineView_titleChanged(const QString &title);
    void on_webEngineView_iconChanged(const QIcon &arg1);
    void grantFeaturePermission(const QUrl &q, QWebEnginePage::Feature f);

private:
    Ui::player *ui;
};

#endif // PLAYER_H

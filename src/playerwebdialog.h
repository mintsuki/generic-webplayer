#ifndef PLAYERWEBDIALOG_H
#define PLAYERWEBDIALOG_H

#include <QMainWindow>
#include <QWebEnginePage>

class WebDialogPage : public QWebEnginePage {
    Q_OBJECT

public:
    explicit WebDialogPage(QWebEngineProfile *profile, QObject *parent = nullptr);

protected:
    QWebEnginePage *createWindow(QWebEnginePage::WebWindowType type) override;
};

namespace Ui {
class PlayerWebDialog;
}

class PlayerWebDialog : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlayerWebDialog(WebDialogPage *page, QWidget *parent = nullptr);
    ~PlayerWebDialog();

private slots:
    void on_webEngineView_iconChanged(const QIcon &arg1);
    void on_webEngineView_titleChanged(const QString &title);
    void windowCloseRequested();

private:
    Ui::PlayerWebDialog *ui;
};

#endif // PLAYERWEBDIALOG_H

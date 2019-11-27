#ifndef PLAYERWEBDIALOG_H
#define PLAYERWEBDIALOG_H

#include <QMainWindow>
#include <QWebEnginePage>

namespace Ui {
class PlayerWebDialog;
}

class PlayerWebDialog : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlayerWebDialog(QWebEnginePage *page, QWidget *parent = nullptr);
    ~PlayerWebDialog();

private slots:
    void on_webEngineView_iconChanged(const QIcon &arg1);

    void on_webEngineView_titleChanged(const QString &title);

private:
    Ui::PlayerWebDialog *ui;
};

#endif // PLAYERWEBDIALOG_H

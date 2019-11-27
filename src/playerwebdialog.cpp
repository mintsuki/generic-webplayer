#include "playerwebdialog.h"
#include "ui_playerwebdialog.h"

WebDialogPage::WebDialogPage(QWebEngineProfile *profile, QObject *parent) : QWebEnginePage(profile, parent) {}

QWebEnginePage *WebDialogPage::createWindow(QWebEnginePage::WebWindowType type) {
    qDebug() << "(in dialog) createWindow type: " << type;

    WebDialogPage *p   = new WebDialogPage(profile());
    PlayerWebDialog *w = new PlayerWebDialog(p);
    w->show();
    return p;
}

PlayerWebDialog::PlayerWebDialog(QWebEnginePage *page, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PlayerWebDialog)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    ui->webEngineView->setPage(page);
}

PlayerWebDialog::~PlayerWebDialog() {
    delete ui->webEngineView->page();
    delete ui;
}

void PlayerWebDialog::on_webEngineView_iconChanged(const QIcon &arg1) {
    setWindowIcon(arg1);
}

void PlayerWebDialog::on_webEngineView_titleChanged(const QString &title) {
    setWindowTitle(title);
}

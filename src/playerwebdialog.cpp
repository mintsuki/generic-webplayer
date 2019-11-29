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

PlayerWebDialog::PlayerWebDialog(WebDialogPage *page, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PlayerWebDialog)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    connect(
        page,
        SIGNAL(windowCloseRequested()),
        this,
        SLOT(windowCloseRequested())
    );

    ui->webEngineView->setPage(page);
}

PlayerWebDialog::~PlayerWebDialog() {
    WebDialogPage *oldPage = static_cast<WebDialogPage *>(ui->webEngineView->page());
    delete ui;
    delete oldPage;
}

void PlayerWebDialog::on_webEngineView_iconChanged(const QIcon &arg1) {
    setWindowIcon(arg1);
}

void PlayerWebDialog::on_webEngineView_titleChanged(const QString &title) {
    setWindowTitle(title);
}

void PlayerWebDialog::windowCloseRequested() {
    close();
}

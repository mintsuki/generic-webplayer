#include "playerwebdialog.h"
#include "ui_playerwebdialog.h"

PlayerWebDialog::PlayerWebDialog(QWebEnginePage *page, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PlayerWebDialog)
{
    ui->setupUi(this);
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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "panel.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    PanelItemModel *model = new PanelItemModel(this);
    model->setList( ui->panel->getItemList() );
    ui->tableView->setModel(model);

}

MainWindow::~MainWindow()
{
    delete ui;
}

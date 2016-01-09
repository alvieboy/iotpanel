#include "Panel.h"
#include "PanelItemModel.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    PanelItemModel *model = new PanelItemModel(this);
    model->setList( ui->panel->getItemList() );
    ui->tableView->setModel(model);

    connect( ui->panel, SIGNAL(itemGrabbed(int)), this, SLOT(onItemGrabbed(int)));

    QHeaderView *verticalHeader = ui->tableView->verticalHeader();
    verticalHeader->sectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(12);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onItemActivated(const QModelIndex &index)
{
    ui->panel->onItemActivated(index);
}

void MainWindow::onItemGrabbed(int index)
{
    ui->tableView->selectRow(index);
}

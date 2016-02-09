#include "Panel.h"
#include "PanelItemModel.h"
#include "PanelItemPropertyModel.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

PanelItemPropertyModel *pmodel;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    PanelItemModel *model = new PanelItemModel(this);
    model->setList( ui->panel->getItemList() );
    ui->tableView->setModel(model);

    pmodel = new PanelItemPropertyModel(this);
    pmodel->setList( ui->panel->getItemPropertyList() );
    ui->propertyTable->setModel(pmodel);

    connect( ui->panel, SIGNAL(itemGrabbed(int)), this, SLOT(onItemGrabbed(int)));

    QHeaderView *verticalHeader = ui->tableView->verticalHeader();
    verticalHeader->sectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(14);

    verticalHeader = ui->propertyTable->verticalHeader();
    verticalHeader->sectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(14);
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

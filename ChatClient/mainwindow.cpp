#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QFile"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("HyperChat");
    setWindowIcon(QIcon(":/images/icon.png"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

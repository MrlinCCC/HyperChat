#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QFile"
#include "spinner.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->hide();
    setWindowTitle("HyperChat");
    setWindowIcon(QIcon(":/images/icon.png"));
}

void MainWindow::OnConnectedToServer()
{
    ui->spinner->hide();
    ui->tabWidget->show();
    ui->tabWidget->raise();
    ui->tabWidget->activateWindow();
}

void MainWindow::SetChatClient(ChatClient *client)
{
    m_client = client;
    connect(m_client, &ChatClient::connectedToServer, this, &MainWindow::OnConnectedToServer);
    connect(ui->registerButton, &QPushButton::clicked, this, [this]()
            {
                QString username = ui->register_username->text();
                QString password = ui->register_passwd1->text();
                QString confPassword = ui->register_passwd2->text();
                if (username.isEmpty() || password.isEmpty()) {
                    QMessageBox::critical(this,
                          "错误",
                          "用户名或密码为空!"); 
                    return;
                }
                if (password == confPassword)
                {
                    m_client->Register(username.toStdString(), password.toStdString());
                }else{
                        QMessageBox::critical(this,
                          "错误",
                          "两次输入密码不同!"); 
                } });

    connect(ui->loginButton, &QPushButton::clicked, this, [this]()
            {
    QString username = ui->login_username->text();
    QString password = ui->login_passwd->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::critical(this, "错误", "用户名或密码为空!"); 
        return;
    }

    m_client->Login(username.toStdString(), password.toStdString()); });
}

MainWindow::~MainWindow()
{
    delete ui;
}

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "chatclient.h"
#include "QMessageBox"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void OnConnectedToServer();

    void SetChatClient(ChatClient *client);

private:
    Ui::MainWindow *ui;
    ChatClient *m_client;
};
#endif // MAINWINDOW_H

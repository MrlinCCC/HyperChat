
#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "chatclient.h"
#include "filesystem"

void InitLogger()
{
    std::filesystem::path logFile = std::filesystem::path(PROJECT_LOG_ROOT) / "ChatClientLog.log";
    SET_LOGFILE(logFile.string());
    SET_LOGLEVEL(DEBUG);
}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString& locale : uiLanguages)
    {
        const QString baseName = "ChatClient_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName))
        {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    //    if (argc != 3)
    //    {
    //        LOG_ERROR("Usage: chatClient <ip> <port>");
    //        return -1;
    //    }
    //    std::string ip = argv[1];
    //    std::string port = argv[2];

    std::string ip = "127.0.0.1";
    std::string port = "8888";
    InitLogger();
    Host host(ip, port);
    ChatClient client(host);
    w.SetChatClient(&client);
    std::thread clientThread([&client]() {
        client.ConnectServer();
        client.Run();
    });
    clientThread.detach();
    return a.exec();
}

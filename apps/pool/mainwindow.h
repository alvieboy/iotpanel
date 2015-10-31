#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QDebug>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTextStream>
#include <QMutex>
#include <QLabel>

namespace Ui {
class MainWindow;
}

struct PanelSettings
{
    PanelSettings() { Reset(); }

    void Reset() {
        iScore1=-1;
        iScore2=-1;
        player1name.clear();
        player2name.clear();
        layout.clear();
        schedule.clear();
    }

    int iScore1, iScore2;
    QString player1name, player2name;
    QString layout, schedule;
};

#define DIRTY(x,y,field)  (x.field != y.field)


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void onAutoConnection();
    void onManualConnection();
    void onResetScore();
    void broadcastDataReady();
    void checkConnection();
    void onHostConnected();
    void onHostData();
protected:
    void SetupBroadcastListener();
    void HandleIPAddress(const QHostAddress &);
    void SetupDefaults();
    void SaveDefaults();
    void ContactHost();
    void NewIP();
    void handleIncomingData();

    bool Transfer(const QStringList &list, QString &error);
    bool Transfer(const QString &str, QString &error);
    QStringList TransferAndGet(const QString &str, QString &error);

private:
    Ui::MainWindow *ui;
    QUdpSocket *broadcastListenerSocket;
    QTcpSocket *connectionSocket;
    QTextStream *connectionStream;
    bool m_bAutomatic;
    QSettings *m_settings;
    QHostAddress m_ip;
    PanelSettings m_ps;
    PanelSettings m_sentSettings;
    QString m_firmware;
    QMutex m_txmutex;
    QByteArray m_incomeData;
    QLabel *statusLabel;
};

#endif // MAINWINDOW_H

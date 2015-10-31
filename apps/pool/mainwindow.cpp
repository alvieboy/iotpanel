#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_settings = new QSettings("alvie","rgbpanel");
    SetupDefaults();

    SetupBroadcastListener();
    m_bAutomatic = true;
    statusLabel = new QLabel("");
    ui->statusBar->addWidget(statusLabel);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onAutoConnection()
{
}

void MainWindow::onManualConnection()
{
}

void MainWindow::onResetScore()
{
    ui->score1Spin->setValue(0);
    ui->score2Spin->setValue(0);
}

void MainWindow::SetupBroadcastListener()
{
    broadcastListenerSocket = new QUdpSocket();
    connectionSocket = new QTcpSocket();
    connectionStream = new QTextStream(connectionSocket);

    if (!broadcastListenerSocket->bind (
                                   QHostAddress::Any,
                                   8082
                                       )) {
        qDebug()<<"Cannot bind socket\n";
    }


    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkConnection()));
    timer->start(10000);

    connect(broadcastListenerSocket,
            SIGNAL(readyRead()),
            this,
            SLOT(broadcastDataReady()));

    connect(connectionSocket,
            SIGNAL(connected()),
            this,
            SLOT(onHostConnected()));

    connect(connectionSocket,
            SIGNAL(readyRead()),
            this,
            SLOT(onHostData()));
}
void MainWindow::broadcastDataReady()
{
    QByteArray datagram;
    datagram.resize(broadcastListenerSocket->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;

    broadcastListenerSocket->readDatagram(datagram.data(),
                                          datagram.size(),
                                          &sender,
                                          &senderPort);
    qDebug()<<"Incoming data of size "<<datagram.size();
    if (datagram.size()==4) {
        /* Process it */
        const unsigned char *data
            = (const unsigned char *)datagram.data();
        quint32 ip;
        ip = (quint32)data[3]<<24;
        ip += (quint32)data[2]<<16;
        ip += (quint32)data[1]<<8;
        ip += (quint32)data[0];
        QHostAddress qip(ip);

        qDebug()<<"IP Address: "<<qip.toString();
        HandleIPAddress(qip);
    }
}

void MainWindow::NewIP()
{
    QString l;

    qDebug()<<"New IP address found";

    l += "<span style=\"color: #00aa00;\"><b>Painel detectado (IP "+m_ip.toString()+")</b></span>";

    statusLabel->setText(l);

    ContactHost();
}

bool MainWindow::Transfer(const QString &str, QString &error)
{
    QStringList l = TransferAndGet(str, error);
    if (l.size()<2) {
        return false;
    }
    if (l[1] != "OK") {
        return false;
    }
    return true;
}

bool MainWindow::Transfer(const QStringList &list, QString &error)
{
    foreach (QString s, list) {
        if (!Transfer(s,error))
            return false;
    }
    return true;
}

QStringList MainWindow::TransferAndGet(const QString &str, QString &dest)
{
    QString msg = "1 " + str + "\n";
    qDebug()<<">> "<<msg;

    QByteArray array (msg.toStdString().c_str()) ;

    connectionSocket->write(array);

    dest = connectionSocket->readLine();

    qDebug()<<"<< "<<dest;

    return dest.split(" ");
}

void MainWindow::HandleIPAddress(const QHostAddress &ip)
{
    if (m_bAutomatic) {
        if (m_ip !=ip) {
            m_ip = ip;
            NewIP();
        }
    }
}

void MainWindow::checkConnection()
{
}

static const char *getDefaultLayout()
{
    return "PROPSET s1 color white\n"
        "PROPSET s1 font 6x10\n"
        "ADD s0 text u2 0 16\n"
        "PROPSET u2 color green\n"
        "PROPSET u2 text \"Visitante\"\n"
        "ADD s0 text u1 0 0\n"
        "PROPSET u1 color red\n"
        "PROPSET u1 text \"Smiles BAR\"\n"
        "ADD s0 text s2 8 23\n"
        "PROPSET s2 font 6x10\n"
        "PROPSET s2 color white\n"
        "PROPSET s2 text 0\n"
        "NEWSCREEN pub\n"
        "ADD pub text 2u2 12 7\n"
        "PROPSET 2u2 color blue\n"
        "PROPSET 2u2 font 6x10\n"
        "PROPSET 2u2 text \"5 FINOS\"\n"
        "ADD pub text 2u3 6 16\n"
        "PROPSET 2u3 text \"3.5 Euros\"\n"
        "PROPSET 2u3 speed 1\n"
        "PROPSET 2u3 font 6x10\n"
        "PROPSET 2u3 alttext  apenas\n"
        "PROPSET 2u3 color green\n"
        "ADD pub text 2u1 5 0\n"
        "PROPSET 2u1 color white\n"
        "PROPSET 2u1 text \"Aproveita Hoje\"\n";

}

static const char *getDefaultSchedule()
{
    return
        "ADDSCHEDULE SELECT s0\n"
        "ADDSCHEDULE WAIT 20\n"
        "ADDSCHEDULE SELECT pub\n"
        "ADDSCHEDULE WAIT 5\n";
}

void MainWindow::SetupDefaults()
{
    m_bAutomatic = m_settings->value("auto",0).toBool();
    m_ps.iScore1 = m_settings->value("score1",0).toInt();
    m_ps.iScore2 = m_settings->value("score2",0).toInt();
    m_ps.player1name = m_settings->value("player1","Smiles BAR").toString();
    m_ps.player2name = m_settings->value("player2","Visitante").toString();

    m_ps.layout = m_settings->value("layout", getDefaultLayout()).toString();
    m_ps.schedule = m_settings->value("schedule", getDefaultSchedule()).toString();
    m_firmware = m_settings->value("firmware", "poolfw").toString();
}

void MainWindow::onHostData()
{
    char buf[128];
    qint64 r = connectionSocket->read(buf,sizeof(buf));
    if (r>0) {
        m_incomeData.append(buf,r);
        handleIncomingData();
    }
}

void MainWindow::handleIncomingData()
{
    bool retry = false;
    do {
        int pos = m_incomeData.indexOf('\n');
        if (pos>0) {
            QByteArray b = m_incomeData.mid(0, pos-1);
            m_incomeData.remove(0,pos);
            retry = true;
        }
    } while (retry);
}

void MainWindow::onHostConnected()
{
    /* Auth */
    QString error;
    qDebug()<<"Connection established";
    Transfer("LOGIN admin",error);
    Transfer("AUTH admin",error);
    QStringList l = TransferAndGet("FWGET", error);
    Transfer("LOGOUT", error);
}

void MainWindow::ContactHost()
{
    qDebug()<<"Contacting host";
    connectionSocket->close();
//    connectionSocket->connectToHost(m_ip, 8081);
}


void MainWindow::SaveDefaults()
{
    m_settings->setValue("auto", m_bAutomatic);
}

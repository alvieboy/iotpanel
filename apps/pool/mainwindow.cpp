#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>

MainWindow::MainWindow(QApplication&app,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_app(app)
{
    ui->setupUi(this);
    m_settings = new QSettings("alvie","rgbpanel");
    SetupDefaults();

    SetupBroadcastListener();
    m_bAutomatic = true;
    statusLabel = new QLabel("");
    ui->statusBar->addWidget(statusLabel);

    // Load defaults into widgets


    ui->score1Spin->setValue( m_ps.iScore1 );

    ui->score2Spin->setValue(m_ps.iScore2);
    ui->player1Entry->setText(m_ps.player1name);
    ui->player2Entry->setText(m_ps.player2name);

#ifdef __linux__
    m_ip = QHostAddress("127.0.0.1");
    NewIP();
#endif
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
#if 0
    connect(connectionSocket,
            SIGNAL(readyRead()),
            this,
            this,       SLOT(onHostData()));
#endif
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
    qDebug()<<">>" <<"1 "<<str;

    QByteArray array (msg.toStdString().c_str()) ;

    connectionSocket->write(array);

    while ( ! connectionSocket->canReadLine()) {
        m_app.processEvents(0, 1000);
    }
    dest = connectionSocket->readLine();
    if (dest.length()>0) {
        int nl = dest.indexOf('\n');
        if (nl>=0) {
            dest.remove(nl,dest.length()-nl);
        }
        qDebug()<<"<< "<<dest;
        return dest.split(" ");
    }
    return QStringList();
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
    return
        "NEWSCREEN s0\n"
        "ADD s0 text s1 8 6\n"
        "PROPSET s1 text 3\n"
        "PROPSET s1 font 6x10\n"
        "PROPSET s1 color white\n"
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
        "PROPSET 2u1 text \"Aproveita Hoje\"\n"
        "SELECT s0";

}

static const char *getDefaultSchedule()
{
    return
        "ADDSCHEDULE SELECT s0\n"
        "ADDSCHEDULE WAIT 20\n"
        "ADDSCHEDULE SELECT pub\n"
        "ADDSCHEDULE WAIT 5";
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

void MainWindow::SaveSettings()
{
    m_settings->setValue("auto", m_bAutomatic);
    m_settings->setValue("score1",m_ps.iScore1);
    m_settings->setValue("score2",m_ps.iScore2);
    m_settings->setValue("player1",m_ps.player1name);
    m_settings->setValue("player2",m_ps.player2name);

    m_settings->setValue("layout", m_ps.layout);
    m_settings->setValue("schedule", m_ps.schedule);
    m_settings->setValue("firmware", m_firmware);
}
#if 0
void MainWindow::onHostData()
{
    char buf[128];
    qint64 r = connectionSocket->read(buf,sizeof(buf));
    if (r>0) {
        m_incomeData.append(buf,r);
        handleIncomingData();
    }
}

void MainWindow::handleLine(QString b)
{
    QMessageQueue.
}

void MainWindow::handleIncomingData()
{
    bool retry = false;
    do {
        int pos = m_incomeData.indexOf('\n');
        if (pos>0) {
            QByteArray b = m_incomeData.mid(0, pos-1);
            m_incomeData.remove(0,pos);
            handleLine(b);
            retry = true;
        }
    } while (retry);
}
#endif

void MainWindow::onHostConnected()
{
    /* Auth */
    QString error;
    qDebug()<<"Connection established";
    Transfer("LOGIN admin",error);
    Transfer("AUTH admin",error);

    QStringList l = TransferAndGet("FWGET", error);
    if (l.size()>=2) {
        // Check firmware
        qDebug() << "Firmware is "<<l[2];
        if (m_firmware!=l[2]) {
            // Transfer new firmware
            Transfer("WIPE", error);
            QStringList layout = QString( m_ps.layout ).split("\n");
            foreach (QString l, layout) {
                Transfer(l,error);
            }

            QStringList schedule = QString( m_ps.schedule ).split("\n");
            Transfer("NEWSCHEDULE",error);
            foreach (QString l, schedule) {
                Transfer(l,error);
            }
            Transfer("SCHEDULE START",error);

            Transfer(QString("FWSET ") + m_firmware, error);
        }
    }
    // Send queue of commands
    QStringList c = m_queue;
    m_queue.clear();
    foreach (QString l, c) {
        Transfer(l,error);
    }

    Transfer("LOGOUT", error);
    // Close
    connectionSocket->close();
}

void MainWindow::ContactHost()
{
    qDebug()<<"Contacting host";
    connectionSocket->close();
    connectionSocket->connectToHost(m_ip, 8081);
}

void MainWindow::onSendUpdate()
{
    qDebug()<<"Update\n";
    // Save settings.
    // Build update

    m_ps.iScore1 = ui->score1Spin->value();
    m_ps.iScore2 = ui->score2Spin->value();
    m_ps.player1name = ui->player1Entry->text();
    m_ps.player2name = ui->player2Entry->text();

    SaveSettings();

    m_queue.clear();

#if 0
    if (m_sentSettings.layout != m_ps.layout){
        m_queue.append( m_ps.layout );
    }
#endif

    if (m_sentSettings.player1name != m_ps.player1name){
        m_queue.append(QString("PROPSET ")+"u1"+" text "+m_ps.player1name);
    }

    if (m_sentSettings.player2name != m_ps.player2name){
        m_queue.append(QString("PROPSET ")+"u2"+" text "+m_ps.player2name);
    }

    if (m_sentSettings.iScore1 != m_ps.iScore1){
        m_queue.append(QString("PROPSET ")+"s1"+" text "+QString::number(m_ps.iScore1));
    }

    if (m_sentSettings.iScore2 != m_ps.iScore2){
        m_queue.append(QString("PROPSET ")+"s2"+" text "+QString::number(m_ps.iScore2));
    }
#if 0
    if (m_sentSettings.schedule != m_ps.schedule){
        m_queue.append( m_ps.schedule );
    }
#endif
    ContactHost();
    m_sentSettings = m_ps;

}


void MainWindow::SaveDefaults()
{
    m_settings->setValue("auto", m_bAutomatic);
}

void MainWindow::onIncrease1Score()
{
    ui->score1Spin->setValue( ui->score1Spin->value()+1);
}

void MainWindow::onIncrease2Score()
{
    ui->score2Spin->setValue( ui->score2Spin->value()+1);
}


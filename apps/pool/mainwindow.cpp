#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <stdexcept>
#include <inttypes.h>
#include <QMessageBox>
#include "PanelLayout.h"
#include <QFileDialog>
#include <QErrorMessage>
#include <QTime>

typedef uint8_t macaddress_t[6];

struct ConnectionError: public std::exception
{
    ConnectionError(const char *what): err(what) {}
    ConnectionError(const std::string &s): err(s) {}
    ConnectionError(const QString &s): err(s.toStdString()) {}

    virtual ~ConnectionError() throw() {}


    virtual const char *what() const throw() {
        return err.c_str();
    }
private:
    std::string err;
};

struct ReplyError: public std::exception
{
};

MainWindow::MainWindow(QApplication&app,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_app(app)
{
    ui->setupUi(this);

    QStringList cmdline_args = m_app.arguments();

    if (cmdline_args.length()>1) {
        qDebug()<<"Using custom settings of"<<cmdline_args[1];
        m_settings = new QSettings("alvie",cmdline_args[1]);
    } else {
        m_settings = new QSettings("alvie","rgbpanel");
    }
    SetupDefaults();

    SetupBroadcastListener();
    statusLabel = new QLabel("");
    ui->statusBar->addWidget(statusLabel);

    // Load defaults into widgets


    ui->score1Spin->setValue( m_ps.iScore1 );

    ui->score2Spin->setValue(m_ps.iScore2);
    ui->player1Entry->setText(m_ps.player1name);
    ui->player2Entry->setText(m_ps.player2name);

    ui->layoutText->setText(m_ps.layout);
    ui->scheduleText->setText(m_ps.schedule);
    ui->firmwareEntry->setText(m_firmware);
    ui->brightnessSpin->setValue(m_ps.brightness);

    qDebug()<<" Automatic is"<<m_bAutomatic;

    if (m_bAutomatic) {
        ui->connAutomatic->setChecked(true);
        ui->ipAddressEntry->setEnabled(false);
    } else {
        ui->connManual->setChecked(true);
        ui->ipAddressEntry->setEnabled(true);
    }
    ui->ipAddressEntry->setText( m_ip.toString() );

#if 0
    m_ip = QHostAddress("127.0.0.1");
    NewIP();
#endif
}

void MainWindow::onReconfigure()
{
    m_bAutomatic = ui->connAutomatic->isChecked();
    if (m_bAutomatic) {
        m_ip = QHostAddress("127.0.0.1");
    } else {
        QHostAddress a = QHostAddress( ui->ipAddressEntry->text() );
        if (!a.isNull()) {
            m_ip = a;
            NewIP();
        } else {
            QMessageBox box(QMessageBox::Critical,
                            "Invalid entry",
                            "Invalid IP address.",
                            QMessageBox::Ok,
                            this);
            box.exec();
            ui->ipAddressEntry->setText( m_ip.toString() );

        }
    }
    SaveSettings();
}


MainWindow::~MainWindow()
{
    delete m_settings;
    delete ui;
}

void MainWindow::onAutoConnection()
{
    ui->ipAddressEntry->setEnabled(false);
    ui->ipAddressEntry->setText(m_ip.toString());
}

void MainWindow::onManualConnection()
{
    qDebug()<<"Manual";
    ui->ipAddressEntry->setEnabled(true);
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
    if (datagram.size()==10) {
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
    QTime timeout = QTime::currentTime().addSecs(10);


    QByteArray array (msg.toStdString().c_str()) ;
    int r = connectionSocket->write(array);
    if (r<=0) {
        QString err;
        err.sprintf("Short write: %d", r);
        throw ConnectionError(err);
    }

    while ( ! connectionSocket->canReadLine()) {
        m_app.processEvents(0, 10000);

        QTime end = QTime::currentTime();

        if (end>timeout) {
            throw ConnectionError("Timeout");
        }
    }

    dest = connectionSocket->readLine();

    if (dest.length()>0) {
        int nl = dest.indexOf('\n');
        if (nl>=0) {
            dest.remove(nl,dest.length()-nl);
        }
        qDebug()<<"<< "<<dest;
        return dest.split(" ");
    } else {
        throw ConnectionError("Short read");
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
        "ADDSCHEDULE SELECT s0\n";
}

void MainWindow::SetupDefaults()
{
    m_bAutomatic = m_settings->value("auto",0).toBool();
    m_ip = QHostAddress( m_settings->value("ip","127.0.0.1").toString());
    m_ps.iScore1 = m_settings->value("score1",0).toInt();
    m_ps.iScore2 = m_settings->value("score2",0).toInt();
    m_ps.player1name = m_settings->value("player1","Casa").toString();
    m_ps.player2name = m_settings->value("player2","Visitante").toString();

    m_ps.layout = m_settings->value("layout", getDefaultLayout()).toString();
    m_ps.schedule = m_settings->value("schedule", getDefaultSchedule()).toString();
    m_firmware = m_settings->value("firmware", "poolfw").toString();
    m_ps.brightness = m_settings->value("brightness",32).toInt();

    QString layoutname = m_settings->value("layoutfile").toString();
    if (layoutname.length())
        openDesign(layoutname);
}

void MainWindow::SaveSettings()
{
    m_settings->setValue("auto", m_bAutomatic);
    m_settings->setValue("ip", m_ip.toString());
    m_settings->setValue("score1",m_ps.iScore1);
    m_settings->setValue("score2",m_ps.iScore2);
    m_settings->setValue("player1",m_ps.player1name);
    m_settings->setValue("player2",m_ps.player2name);
    m_settings->setValue("brightness", m_ps.brightness);

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
    try {
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

        m_sentSettings = m_ps;

    } catch (ConnectionError &e) {
        statusLabel->setText(QString("Connection error ") +e.what());
    }
}

void MainWindow::ContactHost()
{
    qDebug()<<"Contacting host at"<<m_ip;
    connectionSocket->close();
    connectionSocket->connectToHost(m_ip, 8081);
}

void MainWindow::onSendUpdate()
{
    qDebug()<<"Update\n";
    // Save settings.
    // Build update
    bool force = true;

    m_ps.iScore1 = ui->score1Spin->value();
    m_ps.iScore2 = ui->score2Spin->value();
    m_ps.player1name = ui->player1Entry->text();
    m_ps.player2name = ui->player2Entry->text();
    m_ps.brightness = ui->brightnessSpin->value();
    m_ps.layout = ui->layoutText->toPlainText();
    m_ps.schedule = ui->scheduleText->toPlainText();
    SaveSettings();

    m_queue.clear();

#if 1
    if (m_sentSettings.layout != m_ps.layout){
        QStringList list = m_ps.layout.split("\n");
        foreach (QString item, list) {
            m_queue.append( item );
        }
    }
#endif

    if (force || m_sentSettings.player1name != m_ps.player1name){
        m_queue.append(QString("PROPSET ")+"u1"+" text \""+m_ps.player1name+"\"");
    }

    if (force || m_sentSettings.player2name != m_ps.player2name){
        m_queue.append(QString("PROPSET ")+"u2"+" text \""+m_ps.player2name+"\"");
    }

    if (force || m_sentSettings.iScore1 != m_ps.iScore1){
        m_queue.append(QString("PROPSET ")+"s1"+" text "+QString::number(m_ps.iScore1));
    }
        
    if (force || m_sentSettings.iScore2 != m_ps.iScore2){
        m_queue.append(QString("PROPSET ")+"s2"+" text "+QString::number(m_ps.iScore2));
    }

    if (force || m_sentSettings.brightness != m_ps.brightness){
        m_queue.append(QString("BLANK ")+QString::number(m_ps.brightness));
    }
#if 1

    if (m_sentSettings.schedule != m_ps.schedule){
        QStringList list = m_ps.schedule.split("\n");
        foreach (QString item, list) {
            m_queue.append( item );
        }
    }
#endif
    ContactHost();

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

void MainWindow::openDesign(const QString &filename)
{
    PanelLayout layout;

    QFile file(filename);
    if (layout.createFromFile( file )<0) {
        QErrorMessage *errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage("Erro ao abrir");
    } else {
        QStringList l;
        layout.serialize(l);
        qDebug()<<l;
        m_layout = layout;
        ui->layoutText->setText(l.join("\n"));
        onLayoutUpdated();
        m_settings->setValue("layoutfile", file.fileName());
    }
}

void MainWindow::onOpenDesign()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Abrir desenho"), "", tr("Ficheiros XML (*.xml)"));
    //ui->File1Path->setText(file1Name);
    if (filename.length()) {
        openDesign(filename);
    }
}

void MainWindow::onLayoutUpdated()
{
    ui->screenComboBox->clear();

    foreach (LayoutScreen s, m_layout.screens) {
        ui->screenComboBox->addItem(s.description, QVariant(s.name));
    }
}

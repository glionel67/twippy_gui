#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <string>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createActions();
    createMenus();

    qSerialPort_ = nullptr;
    serialPortName_ = QString();
    serialPortBaudRate_ = QSerialPort::Baud115200;
    refreshSerialPortList();

    qUdpSocket_ = nullptr;
    qHostAddress_ = QHostAddress::LocalHost;
    udpPort_ = 8888;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonOpenSerialPort_clicked()
{
    if (qSerialPort_ == nullptr) {
        bool res = openSerialPort();
        if (res)
            ui->buttonOpenSerialPort->setText(QString("Close"));
    }
    else {
        bool res = closeSerialPort();
        if (res)
            ui->buttonOpenSerialPort->setText(QString("Open"));
    }
}

void MainWindow::on_buttonRefresh_clicked()
{
    refreshSerialPortList();
}

void MainWindow::on_pushButtonExit_clicked()
{
    close();
}

void MainWindow::serialPortRoutine()
{
    //static QVector<QString> lines;
    static unsigned nLines = 0;
    //std::cerr << "serialPortRoutine\n";
    qint64 nBytesToRead = qSerialPort_->bytesAvailable();
    QVector<char> data(nBytesToRead);
    if (nBytesToRead > 0) {
        qint64 res = qSerialPort_->read(data.data(), nBytesToRead);
        if (res == nBytesToRead) {
            //std::cerr << "Serial port reading " <<  nBytesToRead << " bytes successfully\n";

            for (const auto& c : data) {
                if (c == '\n')
                    nLines++;
            }
            QString qstr(data.data());
            QString prevText = ui->labelInputConsole->text();
            QString newText = prevText + qstr;
            while (nLines > 8) {
                int idx = newText.indexOf('\n');
                //std::cerr << "idx=" << idx << std::endl;
                newText = newText.right(newText.size() - idx - 1);
                nLines--;
            }
            ui->labelInputConsole->setText(newText);
        }
    }
}

void MainWindow::on_lineEditOutputConsole_returnPressed()
{
    QString text = ui->lineEditOutputConsole->text();
    QChar* qChar = text.data();
    qint64 res = qSerialPort_->write((char*)qChar, text.size());
    if (res == 1) {
        std::cerr << "serialPort write successfull\n";
    }
}

void MainWindow::readUdpDatagrams()
{
    while (qUdpSocket_->hasPendingDatagrams()) {
        qint64	nData = qUdpSocket_->pendingDatagramSize();
        char data[512] = { 0, };
        qint64 ret = qUdpSocket_->readDatagram(data, nData, &qHostAddress_, &udpPort_);
        if (ret == nData) {
            processUdpDatagram(data, nData);
        }
        else {
            std::cerr << "MainWindow::readUdpDatagrams: readDatagram failed\n";
        }
    }
}

void MainWindow::openSerialPortSlot()
{
    bool res = openSerialPort();
    if (!res) {
        std::cerr << "MainWindow::openSerialPortSlot: failed to open serial port\n";
    }
    else {
        std::cerr << "MainWindow::openSerialPortSlot: serial port opened\n";
    }
}

void MainWindow::openUdpSocketSlot()
{
    bool res = openUdpSocket();
    if (!res) {
        std::cerr << "MainWindow::openUdpSocketSlot: failed to open udp socket\n";
    }
    else {
        std::cerr << "MainWindow::openUdpSocketSlot: udp socket opened\n";
    }
}

void MainWindow::createActions()
{
    openSerialPortAct_ = new QAction(tr("&Open serial port"), this);
    //openSerialPortAct_->setShortcuts(QKeySequence::Open);
    openSerialPortAct_->setStatusTip(tr("Open serial port"));
    connect(openSerialPortAct_, SIGNAL(triggered(bool)), this, SLOT(openSerialPortSlot()));

    openUdpSocketAct_ = new QAction(tr("&Open udp socket"), this);
    //openUdpSocketAct_->setShortcuts(QKeySequence::Open);
    openUdpSocketAct_->setStatusTip(tr("Open udp socket"));
    connect(openUdpSocketAct_, SIGNAL(triggered(bool)), this, SLOT(openUdpSocketSlot()));

    exitAct_ = new QAction(tr("E&xit"), this);
    exitAct_->setShortcuts(QKeySequence::Quit);
    exitAct_->setStatusTip(tr("Exit the application"));
    connect(exitAct_, SIGNAL(triggered(bool)), this, SLOT(close()));
}

void MainWindow::createMenus()
{
    menuBar()->setNativeMenuBar(false);

    fileMenu_ = menuBar()->addMenu(tr("&File"));
    fileMenu_->addAction(exitAct_);

    ioMenu_ = menuBar()->addMenu(tr("&I/O"));
    ioMenu_->addAction(openSerialPortAct_);
    ioMenu_->addAction(openUdpSocketAct_);
}

bool MainWindow::openSerialPort()
{
    if (qSerialPort_ != nullptr) {
        std::cerr << "MainWindow::openSerialPort: serial port already opened\n";
        return false;
    }

    // Get serial port name
    serialPortName_ = ui->comboBoxSerialPortName->currentText();
    std::cerr << "MainWindow::openSerialPort: serialPortName="
              << serialPortName_.toStdString() << std::endl;

    if (serialPortName_.isEmpty()) {
        std::cerr << "MainWindow::openSerialPort: no serial port name given\n";
        return false;
    }

    qSerialPort_ = new QSerialPort();
    qSerialPort_->setPortName(serialPortName_);
    qSerialPort_->setParity(QSerialPort::NoParity); // NoParity, EvenParity, OddParity, SpaceParity, MarkParity, UnknownParity
    qSerialPort_->setStopBits(QSerialPort::OneStop); // OneStop, OneAndHalfStop, TwoStop, UnknownStopBits
    qSerialPort_->setFlowControl(QSerialPort::NoFlowControl); // NoFlowControl, HardwareControl, SoftwareControl, UnknownFlowControl
    qSerialPort_->setDataBits(QSerialPort::Data8); // Data5, Data6, Data7, Data8, UnknownDataBits
    qSerialPort_->setBaudRate(serialPortBaudRate_); // Baud9600, Baud57600,

    bool res = qSerialPort_->open(QIODevice::ReadWrite);
    if (!res) {
        int errorCode = qSerialPort_->error();
        if (errorCode == QSerialPort::DeviceNotFoundError) {
            std::cerr << "MainWindow::openSerialPort: DeviceNotFoundError\n";
        }
        else if (errorCode == QSerialPort::PermissionError) {
            std::cerr << "MainWindow::openSerialPort: PermissionError\n";
        }

        qSerialPort_ = nullptr;

        return false;
    }

    std::cerr << "MainWindow::openSerialPort: serial port opened\n";

    qTimerSerialPort_ = new QTimer();
    qTimerSerialPort_->setInterval(5);
    connect(qTimerSerialPort_, SIGNAL(timeout()), this, SLOT(serialPortRoutine()));
    qTimerSerialPort_->start();

    return true;
}

bool MainWindow::closeSerialPort()
{
    if (qSerialPort_ == nullptr) {
        std::cerr << "MainWindow::closeSerialPort: serial port already closed\n";
        return false;
    }

    qTimerSerialPort_->stop();
    qSerialPort_->close();

    qSerialPort_ = nullptr;

    std::cerr << "MainWindow::closeSerialPort: serial port closed\n";

    return true;
}

void MainWindow::refreshSerialPortList()
{
    serialPorts_ = QSerialPortInfo::availablePorts();
    std::cerr << "MainWindow::refreshSerialPortList: "
              << serialPorts_.size() << " serial ports detected\n";

    ui->comboBoxSerialPortName->clear();

    for (const auto& serialPort : serialPorts_) {
        QString descr = serialPort.description();
        QString manuf = 	serialPort.manufacturer();
        QString	 name = serialPort.portName();
        quint16 id = serialPort.productIdentifier();
        QString sn = serialPort.serialNumber();
        QString	 sysLoc = serialPort.systemLocation();
        quint16 vid = serialPort.vendorIdentifier();
        std::cerr << name.toStdString() << ": id=" << id << ", vid=" << vid
                  << ", sn=" << sn.toStdString() << ", sysLoc=" << sysLoc.toStdString()
                  << ", manuf=" << manuf.toStdString() << ", descr=" << descr.toStdString()
                  << std::endl;

        ui->comboBoxSerialPortName->addItem(sysLoc);
    }
}

bool MainWindow::openUdpSocket()
{
    qUdpSocket_ = new QUdpSocket();
    bool ok = qUdpSocket_->bind(qHostAddress_, udpPort_, QAbstractSocket::DefaultForPlatform);
    if (!ok) {
        std::cerr << "MainWindow::openUdpSocket: failed to bind UDP socket\n";
        return false;
    }

    connect(qUdpSocket_, SIGNAL(readyRead()), this, SLOT(readUdpDatagrams()));

    return true;
}

bool MainWindow::closeUdpSocket()
{
    if (qUdpSocket_ == nullptr)
        return false;

    qUdpSocket_->disconnectFromHost();

    disconnect(qUdpSocket_, SIGNAL(readyRead()), this, SLOT(readUdpDatagrams()));

    qUdpSocket_ = nullptr;

    return true;
}

void MainWindow::processUdpDatagram(char* _data, qint64 _len)
{
    std::cerr << "MainWindow::processUdpDatagram: data=";
    for (qint64 i=0;i<_len;i++)
        std::cerr << _data[i];
    std::cerr << std::endl;
}


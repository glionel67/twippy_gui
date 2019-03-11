#pragma once

#include <QMainWindow>

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QUdpSocket>

#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_buttonOpenSerialPort_clicked();

    void on_buttonRefresh_clicked();

    void on_pushButtonExit_clicked();

    void on_lineEditOutputConsole_returnPressed();

    void serialPortRoutine();

    void readUdpDatagrams();

    void openSerialPortSlot();

    void openUdpSocketSlot();

private:
    void createActions();
    void createMenus();

    bool openSerialPort();
    bool closeSerialPort();
    void refreshSerialPortList();

    bool openUdpSocket();
    bool closeUdpSocket();
    void processUdpDatagram(char* _data, qint64 _len);

private:
    Ui::MainWindow *ui;

    QMenu* fileMenu_;
    QMenu* ioMenu_;

    QAction* openSerialPortAct_;
    QAction* openUdpSocketAct_;

    QAction* exitAct_;

    // Serial port
    QList<QSerialPortInfo> serialPorts_;
    QSerialPort* qSerialPort_;
    QString serialPortName_;
    qint32 serialPortBaudRate_;

    //bool isSerialPortOpened_;
    QTimer* qTimerSerialPort_;

    // Udp socket
    QUdpSocket* qUdpSocket_;
    QHostAddress qHostAddress_;
    quint16 udpPort_;
}; // class MainWindow


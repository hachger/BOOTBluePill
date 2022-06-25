#ifndef QSERIALSETUP_H
#define QSERIALSETUP_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

namespace Ui {
class QSerialSetup;
}

class QSerialSetup : public QDialog
{
    Q_OBJECT

public:
    explicit QSerialSetup(QWidget *parent = nullptr, QSerialPort *AQSerialPort = nullptr);
    ~QSerialSetup();

    QString _PortName;
    QSerialPort::BaudRate _BaudRate;
    QSerialPort::DataBits _Bits;
    QSerialPort::StopBits _StopBits;
    QSerialPort::Parity _Parity;
    QSerialPort::FlowControl _FlowControl;

    QString getSerialPortConfig(void);
    void setSerialPort(QSerialPort *AComPort);
    void getAvailablePorts(void);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::QSerialSetup *ui;

    QString qSerialConfiguration;

    QSerialPort *qSerial;
};

#endif // QSERIALSETUP_H

#include "qserialsetup.h"
#include "ui_qserialsetup.h"


QSerialSetup::QSerialSetup(QWidget *parent, QSerialPort *AQSerialPort) :
    QDialog(parent),
    ui(new Ui::QSerialSetup)
{
    ui->setupUi(this);

    QSerialPortInfo SerialPortInfo1;

    qSerial = AQSerialPort;

    for(int i=0;i<SerialPortInfo1.availablePorts().count();i++)
        ui->comboBox->addItem(SerialPortInfo1.availablePorts().at(i).portName());
    if(SerialPortInfo1.availablePorts().count()>0)
        ui->comboBox->setCurrentIndex(0);
    _PortName = ui->comboBox->currentText();

    for(int i=1200;i<=115200;i*=2){
        if(i==(38400*2))
            i=57600;
        ui->comboBox_2->addItem(QString().number(i));

    }
    ui->comboBox_2->setCurrentIndex(3);
    _BaudRate = QSerialPort::Baud9600;

//    ui->comboBox->addItem("5", QSerialPort::Data5);
//    _Bits = QSerialPort::DataBits(ui->comboBox->currentData().toInt());

    ui->comboBox_3->addItem("5");
    ui->comboBox_3->addItem("6");
    ui->comboBox_3->addItem("7");
    ui->comboBox_3->addItem("8");
    ui->comboBox_3->setCurrentIndex(3);
    _Bits = QSerialPort::Data8;

    ui->comboBox_4->addItem("1");
    ui->comboBox_4->addItem("1.5");
    ui->comboBox_4->addItem("2");
    ui->comboBox_4->setCurrentIndex(0);
    _StopBits = QSerialPort::OneStop;

    ui->comboBox_5->addItem("NONE");
    ui->comboBox_5->addItem("ODD");
    ui->comboBox_5->addItem("EVEN");
    ui->comboBox_5->setCurrentIndex(0);
    _Parity = QSerialPort::NoParity;

    ui->comboBox_6->addItem("NONE");
    ui->comboBox_6->addItem("HARDWARE");
    ui->comboBox_6->addItem("XON/XOFF");
    ui->comboBox_6->setCurrentIndex(0);
    _FlowControl = QSerialPort::NoFlowControl;

    qSerialConfiguration = ";;;;";

}

QSerialSetup::~QSerialSetup()
{
    delete ui;
}

QString QSerialSetup::getSerialPortConfig()
{
    return qSerialConfiguration;
}

void QSerialSetup::getAvailablePorts(){
    QSerialPortInfo SerialPortInfo1;

    ui->comboBox->clear();
    for(int i=0; i<SerialPortInfo1.availablePorts().count(); i++)
        ui->comboBox->addItem(SerialPortInfo1.availablePorts().at(i).portName());
    if(SerialPortInfo1.availablePorts().count() > 0)
        ui->comboBox->setCurrentIndex(0);
}

void QSerialSetup::setSerialPort(QSerialPort *AComPort){
    AComPort->setPortName(_PortName);
    AComPort->setBaudRate(_BaudRate);
    AComPort->setDataBits(_Bits);
    AComPort->setStopBits(_StopBits);
    AComPort->setParity(_Parity);
    AComPort->setFlowControl(_FlowControl);
}

void QSerialSetup::on_buttonBox_accepted()
{
    bool ok;

    _PortName=ui->comboBox->currentText();

    switch(ui->comboBox_2->currentIndex()){
        case 0:
            _BaudRate=QSerialPort::Baud1200;
        break;
        case 1:
            _BaudRate=QSerialPort::Baud2400;
        break;
        case 2:
            _BaudRate=QSerialPort::Baud4800;
        break;
        case 3:
            _BaudRate=QSerialPort::Baud9600;
        break;
        case 4:
            _BaudRate=QSerialPort::Baud19200;
        break;
        case 5:
            _BaudRate=QSerialPort::Baud38400;
        break;
        case 6:
            _BaudRate=QSerialPort::Baud57600;
        break;
        case 7:
            _BaudRate=QSerialPort::Baud115200;
        break;
    }

    switch(ui->comboBox_3->currentIndex()){
        case 0:
            _Bits=QSerialPort::Data5;
        break;
        case 1:
            _Bits=QSerialPort::Data6;
        break;
        case 2:
            _Bits=QSerialPort::Data7;
        break;
        case 3:
            _Bits=QSerialPort::Data8;
        break;
    }

    switch(ui->comboBox_4->currentIndex()){
        case 0:
            _StopBits=QSerialPort::OneStop;
        break;
        case 1:
            _StopBits=QSerialPort::OneAndHalfStop;
        break;
        case 2:
            _StopBits=QSerialPort::TwoStop;
    }

    switch(ui->comboBox_5->currentIndex()){
        case 0:
            _Parity=QSerialPort::NoParity;
        break;
        case 1:
            _Parity=QSerialPort::OddParity;
        break;
        case 2:
            _Parity=QSerialPort::EvenParity;
    }

    switch(ui->comboBox_6->currentIndex()){
        case 0:
            _FlowControl=QSerialPort::NoFlowControl;
        break;
        case 1:
            _FlowControl=QSerialPort::HardwareControl;
        break;
        case 2:
            _FlowControl=QSerialPort::SoftwareControl;
    }

    if(qSerial != nullptr){
        qSerial->setPortName(_PortName);
        qSerial->setBaudRate(_BaudRate);
        qSerial->setDataBits(_Bits);
        qSerial->setStopBits(_StopBits);
        qSerial->setParity(_Parity);
        qSerial->setFlowControl(_FlowControl);
    }

    qSerialConfiguration = _PortName+";";

    if(_BaudRate == QSerialPort::Baud1200)
        qSerialConfiguration = qSerialConfiguration + "1200;";
    if(_BaudRate == QSerialPort::Baud2400)
        qSerialConfiguration = qSerialConfiguration + "2400;";
    if(_BaudRate == QSerialPort::Baud4800)
        qSerialConfiguration = qSerialConfiguration + "4800;";
    if(_BaudRate == QSerialPort::Baud9600)
        qSerialConfiguration = qSerialConfiguration + "9600;";
    if(_BaudRate == QSerialPort::Baud19200)
        qSerialConfiguration = qSerialConfiguration + "19200;";
    if(_BaudRate == QSerialPort::Baud38400)
        qSerialConfiguration = qSerialConfiguration + "38400;";
    if(_BaudRate == QSerialPort::Baud57600)
        qSerialConfiguration = qSerialConfiguration + "57600;";
    if(_BaudRate == QSerialPort::Baud115200)
        qSerialConfiguration = qSerialConfiguration + "115200;";

    if(_Bits == QSerialPort::Data5)
        qSerialConfiguration = qSerialConfiguration + "5;";
    if(_Bits == QSerialPort::Data6)
        qSerialConfiguration = qSerialConfiguration + "6;";
    if(_Bits == QSerialPort::Data7)
        qSerialConfiguration = qSerialConfiguration + "7;";
    if(_Bits == QSerialPort::Data8)
        qSerialConfiguration = qSerialConfiguration + "8;";

    if(_StopBits == QSerialPort::OneStop)
        qSerialConfiguration = qSerialConfiguration + "1;";
    if(_StopBits == QSerialPort::OneAndHalfStop)
        qSerialConfiguration = qSerialConfiguration + "1,5;";
    if(_StopBits == QSerialPort::TwoStop)
        qSerialConfiguration = qSerialConfiguration + "2;";

    if(_Parity == QSerialPort::NoParity)
        qSerialConfiguration = qSerialConfiguration + "None;";
    if(_Parity == QSerialPort::OddParity)
        qSerialConfiguration = qSerialConfiguration + "Odd;";
    if(_Parity == QSerialPort::EvenParity)
        qSerialConfiguration = qSerialConfiguration + "Even;";

    if(_FlowControl == QSerialPort::NoFlowControl)
        qSerialConfiguration = qSerialConfiguration + "None;";
    if(_FlowControl == QSerialPort::HardwareControl)
        qSerialConfiguration = qSerialConfiguration + "Hardware;";
    if(_FlowControl == QSerialPort::SoftwareControl)
        qSerialConfiguration = qSerialConfiguration + "Software;";

}

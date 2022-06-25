#include "qform1.h"
#include "ui_qform1.h"

QForm1::QForm1(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QForm1)
{
    ui->setupUi(this);

    QTimer1 = new QTimer(this);
    QSerialPort1 = new QSerialPort(this);

    QSerialPort1->setParity(QSerialPort::EvenParity);
    QSerialPort1->setBaudRate(115200);

    connect(QTimer1, &QTimer::timeout, this, &QForm1::OnQTimer1);
    connect(QSerialPort1, &QSerialPort::readyRead, this, &QForm1::OnDataQSerialPort1);

//    QSerialPortInfo SerialPortInfo1;

//    for(int i=0;i<SerialPortInfo1.availablePorts().count();i++)
//        ui->comboBox->addItem(SerialPortInfo1.availablePorts().at(i).portName());
//    if(SerialPortInfo1.availablePorts().count()>0)
//        ui->comboBox->setCurrentIndex(0);

    ui->comboBox->installEventFilter(this);

    ui->pushButton_2->setEnabled(false);
    ui->pushButton_3->setEnabled(false);
    ui->pushButton_4->setEnabled(false);

    timeOutTryingBoot = 0;
    waitingNBytes = 0;
    cmdWorking = false;
    checkFlash = false;
    readMemorySize = false;
    indexrxr = 0;
    indexrxw = 0;
    sequence = SEQ_IDLE;
    cmdList.clear();
    QTimer1->start(50);
}

QForm1::~QForm1()
{
    delete ui;
}

bool QForm1::eventFilter(QObject *watched, QEvent *event){
    if(watched == ui->comboBox) {
        if (event->type() == QEvent::MouseButtonPress) {
            ui->comboBox->clear();
            QSerialPortInfo SerialPortInfo1;

            for(int i=0;i<SerialPortInfo1.availablePorts().count();i++)
                ui->comboBox->addItem(SerialPortInfo1.availablePorts().at(i).portName());
//            ui->comboBox->showPopup();
//            if(SerialPortInfo1.availablePorts().count()>0)
//                ui->comboBox->setCurrentIndex(0);

            return QMainWindow::eventFilter(watched, event);
        }
        else {
            return false;
        }
    }
    else{
         // pass the event on to the parent class
         return QMainWindow::eventFilter(watched, event);
    }
}

void QForm1::OnQTimer1(){

    if(timeOutCMD){
        timeOutCMD--;
        if(!timeOutCMD){
            sequence = SEQ_IDLE;
            ui->plainTextEdit_3->appendPlainText("ERROR");
            cmdWorking = false;
            readMemorySize = false;
            checkFlash = false;
            cmdList.clear();
        }
    }

    if(timeOutTryingBoot){
        timeOutTryingBoot--;
        if(!timeOutTryingBoot){
            sequence = SEQ_IDLE;
            ui->plainTextEdit_3->appendPlainText("TESTING BOOT ERROR");
            cmdList.clear();
            cmdWorking = false;
        }
        else
            cmdList.append(IN_BOOT);
    }

    if(cmdList.count()>0){
        lastCMD = cmdList.takeFirst();
        if(!QSerialPort1->isOpen()){
            sequence = SEQ_IDLE;
            ui->plainTextEdit_3->appendPlainText("OPEN a COM PORT");
        }
        else{
            tx[0] = lastCMD;
            if(lastCMD == IN_BOOT)
                QSerialPort1->write((char *)tx, 1);
            else{
                tx[1] = ~tx[0];
                QSerialPort1->write((char *)tx, 2);
            }
            cmdWorking = true;
            sequence = SEQ_WAIT_ACK;
            timeOutCMD = 2000/50;
            if(lastCMD!=READ_MEMORY && lastCMD!=ERASE_PAGE && lastCMD!=WRITE_MEMORY)
                ui->plainTextEdit_3->appendPlainText("SEND CMD 0x" + QString("%1").arg(lastCMD, 2, 16, QChar('0')) + "...");
        }
    }
}

void QForm1::OnDataQSerialPort1(){
    int count;
    uint8_t *buf;
    QString str;
    work w;

    count = QSerialPort1->bytesAvailable();
    if(count <= 0)
        return;

    buf = new uint8_t[count];
    QSerialPort1->read((char *)buf, count);
    for (int i=0; i<count; i++) {
        rx[indexrxw++] = buf[i];
        indexrxw &= 1023;

        switch (sequence) {
        case SEQ_IDLE:
            break;
        case SEQ_SEND_CMD:
            break;
        case SEQ_WAIT_ACK:
            timeOutCMD = 0;
            if(buf[i] == NACK){
                sequence = SEQ_IDLE;
                ui->plainTextEdit_3->appendPlainText("NACK");
                break;
            }
            if(buf[i] != ACK){
                sequence = SEQ_IDLE;
                ui->plainTextEdit_3->appendPlainText("ERROR CMD");
                break;
            }
            switch(lastCMD){
            case IN_BOOT:
                timeOutTryingBoot = 0;
                ui->plainTextEdit_3->appendPlainText("OK");
                cmdList.append(GET_CMD_VERSION);
                break;
            case GET_CMD_VERSION:
            case GET_CHIP_ID:
                sequence = SEQ_WAIT_NBYTES;
                break;
            case GET_BOOT_VERSION:
                sequence = SEQ_WAIT_BYTES;
                waitingNBytes = 4;
                indexrxw = 0;
                break;
            case READ_MEMORY:
                w.u32 = memAddress;
                tx[0] = w.u8[3];
                tx[1] = w.u8[2];
                tx[2] = w.u8[1];
                tx[3] = w.u8[0];
                tx[4] = w.u8[3] ^ w.u8[2] ^ w.u8[1] ^ w.u8[0];
                QSerialPort1->write((char *)tx, 5);
                ui->label_3->setText("READ 0x" + QString("%1").arg(memAddress, 8, 16, QChar('0')));
                sequence = SEQ_MEM_ADDRESS_ACK;
                break;
            case ERASE_PAGE:
                tx[0] = 0;
                tx[1] = numPage;
                tx[2] = tx[0] ^ tx[1];
                sequence = SEQ_ERASEPAGE_NPAGES_ACK;
                QSerialPort1->write((char *)tx, 3);
                ui->label_3->setText("ERASING PAGE " + QString("%1").arg(numPage, 3, 10, QChar('0')));
                break;
            case WRITE_MEMORY:
                w.u32 = memAddress;
                tx[0] = w.u8[3];
                tx[1] = w.u8[2];
                tx[2] = w.u8[1];
                tx[3] = w.u8[0];
                tx[4] = w.u8[3] ^ w.u8[2] ^ w.u8[1] ^ w.u8[0];
                QSerialPort1->write((char *)tx, 5);
                ui->label_3->setText("WRITE 0x" + QString("%1").arg(memAddress, 8, 16, QChar('0')));
                sequence = SEQ_WRMEN_ADDRES_ACK;
                break;
            }
            break;
        case SEQ_WAIT_NBYTES:
                waitingNBytes = buf[i] + 1 + 1;
                indexrxw = 0;
                sequence = SEQ_WAIT_BYTES;
            break;
        case SEQ_WAIT_BYTES:
                waitingNBytes--;
                if(!waitingNBytes){
                    timeOutCMD = 0;
                    if(rx[--indexrxw] == ACK){
                        switch(lastCMD){
                        case GET_CMD_VERSION:
                            str = "0x";
                            for(int i=0; i<12; i++)
                                str = str +  QString("%1").arg(rx[i], 2, 16, QChar('0'));
                            ui->plainTextEdit_3->appendPlainText(str);
                            cmdList.append(GET_CHIP_ID);
                            break;
                        case GET_CHIP_ID:
                            str = QString("%1").arg(rx[0], 2, 16, QChar('0'));
                            str = str + QString("%1").arg(rx[1], 2, 16, QChar('0'));
                            str = str + " OK";
                            ui->plainTextEdit_3->appendPlainText(str);
                            cmdList.append(GET_BOOT_VERSION);
                            break;
                        case GET_BOOT_VERSION:
                            str = QString("%1").arg(rx[0], 2, 16, QChar('0'));
                            str = str + QString("%1").arg(rx[1], 2, 16, QChar('0'));
                            str = str + QString("%1").arg(rx[2], 2, 16, QChar('0'));
                            str = str + " OK";
                            ui->plainTextEdit_3->appendPlainText(str);
                            readMemorySize = true;
                            memAddress = 0x1FFFF7E0;
                            cmdList.append(READ_MEMORY);
                            break;
                        }
                    }
                    else
                        ui->plainTextEdit_3->appendPlainText("ERROR");
                }
            break;
        case SEQ_MEM_ADDRESS_ACK:
            timeOutCMD = 0;
            if(buf[i] == ACK){
                if(readMemorySize)
                    nBytesMem = 2;
                else
                    nBytesMem = 256;
                waitingNBytes = nBytesMem;
                tx[0] = nBytesMem - 1;
                tx[1] = ~tx[0];
                QSerialPort1->write((char *)tx, 2);
//                ui->label_3->setText("READ N Bytes: " + QString("%1").arg(nBytesMem, 3, 10, QChar('0')));
                sequence = SEQ_MEM_NBYTES_ACK;
                timeOutCMD = 2000/50;
            }
            if(buf[i] == NACK){
                ui->plainTextEdit_3->appendPlainText("READ ADDRESS ERROR 0x" + QString("%1").arg(memAddress, 8, 16, QChar('0')));
                cmdWorking = false;
            }
            break;
        case SEQ_MEM_NBYTES_ACK:
            if(buf[i] == ACK){
                sequence = SEQ_MEM_NBYTES;
                indexrxw = 0;
            }
            if(buf[i] == NACK){
                ui->plainTextEdit_3->appendPlainText("READ BYTES ADDRESS ERROR 0x" + QString("%1").arg(memAddress, 8, 16, QChar('0')));
                cmdWorking = false;
            }
            break;
        case SEQ_MEM_NBYTES:
            waitingNBytes--;
            if(!waitingNBytes){
                timeOutCMD = 0;
                if(readMemorySize){
                    readMemorySize = false;
                    w.u8[0] = rx[0];
                    w.u8[1] = rx[1];
                    str = QString("0x%1").arg(w.u16[0], 4, 16, QChar('0')) + str;
                    memFlashSize = w.u16[0]*1024;
                    ui->plainTextEdit_3->appendPlainText(str);
                    str = "FLASH SIZE: " + QString().number(memFlashSize) + " Bytes";
                    ui->plainTextEdit_3->appendPlainText(str);
                    cmdWorking = false;
                    ui->pushButton_2->setEnabled(true);
                    ui->pushButton_3->setEnabled(true);
                    ui->pushButton_4->setEnabled(true);
                    break;
                }

                str = "0x";
                for(int i=0; i<nBytesMem; i++){
                    str = str + QString("%1").arg(rx[i], 2, 16, QChar('0'));
                    bufferMCUFLASH[(memAddress+i-0x08000000)/1024][(memAddress+i-0x08000000)%1024] = rx[i];
                }
                memAddress += 256;
                ui->progressBar->setValue(((memAddress-0x08000000)*100)/memFlashSize);
                if((memAddress-0x08000000) == memFlashSize){
                    cmdWorking = false;
                    ShowMCUFLASH();
                    ui->plainTextEdit_3->appendPlainText("READ FLASH OK");
                    if(checkFlash){
                        ui->plainTextEdit_3->appendPlainText("VERIFYING FLASH ...");
                        ui->progressBar->setValue(0);
                        for (uint32_t i=0; i<(numLastPage+1); i++) {
                            for (int j=0; j<1024; j++) {
                                if(bufferFFLASH[i][j] != bufferMCUFLASH[i][j]){
                                    str = "VERIFYING FLASH ERROR ADDRESS 0x";
                                    str = str + QString("%1").arg(i*1024+j+0x08000000, 8, 16, QChar('0'));
                                    ui->plainTextEdit_3->appendPlainText(str);
                                    i = 128;
                                    j = 1024;
                                    checkFlash = false;
                                    ui->progressBar->setValue(i*100/128);
                                }
                            }
                        }
                        if(checkFlash)
                            ui->plainTextEdit_3->appendPlainText("VERIFYING FLASH OK");
                        checkFlash = false;
                    }
                    sequence = SEQ_IDLE;
                }
                else{
                    cmdList.append(READ_MEMORY);
                }
            }
            break;
        case SEQ_ERASEPAGE_NPAGES_ACK:
            if(buf[i] == ACK){
                timeOutCMD = 0;
                quarterPageNumber = 0;
                cmdList.append(WRITE_MEMORY);
                break;
            }
            if(buf[i] == NACK){
                ui->plainTextEdit_3->appendPlainText("ERASE PAGE ERROR 0x" + QString("%1").arg(memAddress, 8, 16, QChar('0')));
                cmdWorking = false;
            }
            break;
        case SEQ_ERASEPAGE_NPAGES:
            break;
        case SEQ_WRMEN_ADDRES_ACK:
            timeOutCMD = 0;
            if(buf[i] == ACK){
                nBytesMem = 256;
                waitingNBytes = nBytesMem;
                tx[0] = nBytesMem - 1;
                w.u8[0] = tx[0];
                for(int i=0; i<256; i++){
                    tx[1+i] = bufferFFLASH[(memAddress+i-0x08000000)/1024][(memAddress+i-0x08000000)%1024];
                    w.u8[0] ^= tx[1+i];
                }
                tx[257] = w.u8[0];
                QSerialPort1->write((char *)tx, 258);
                sequence = SEQ_WRMEN_NBYTES_ACK;
                timeOutCMD = 2000/50;
                break;
            }
            if(buf[i] == NACK){
                ui->plainTextEdit_3->appendPlainText("WRITE FLASH ERROR 0x" + QString("%1").arg(memAddress, 8, 16, QChar('0')));
                cmdWorking = false;
            }
            break;
        case SEQ_WRMEN_NBYTES_ACK:
            if(buf[i] == ACK){
                timeOutCMD = 0;
                memAddress += 256;
                quarterPageNumber++;
                if(quarterPageNumber == 4){
                    numPage++;
                    ui->progressBar->setValue(numPage*100/(numLastPage+1));
//                    ui->plainTextEdit_3->appendPlainText(QString().asprintf("PAGE %3d de %3d", numPage, numLastPage));
                    if(numPage == (numLastPage+1)){
                        cmdWorking = false;
                        ui->plainTextEdit_3->appendPlainText("WRITE FLASH FINISHED");
                    }
                    else
                        cmdList.append(ERASE_PAGE);
                }
                else
                    cmdList.append(WRITE_MEMORY);
                break;
            }
            if(buf[i] == NACK){
                ui->plainTextEdit_3->appendPlainText("WRITE FLASH ERROR 0x" + QString("%1").arg(memAddress, 8, 16, QChar('0')));
                cmdWorking = false;
            }
            break;
        case SEQ_WRMEN_NBYTES:
            break;
        case SEQ_TEST_BOOT:
            break;
        }
    }
}

QString QForm1::IntToHex(int Value, int Digits)
{
    return QString("%1").arg(Value, Digits, 16, QChar('0'));
}

QString QForm1::IntToStr(int Value)
{
    return QString().number(Value);
}

bool QForm1::FileFLASHtoBUF(QString AFileFlash){
    char hexRead[5];
    unsigned char size,mode,value,cks;
    int ads,address,auxaddress, firstpage;
    QString str;
    work w;

    QFile *hexFile = new QFile();
    hexFile->setFileName(AFileFlash);
    if(!hexFile->open(QFile::ReadOnly)){
        delete hexFile;
        return false;
    }

    if(hexFile->size()==0)
        return false;

    for(int i=0;i<128;i++){
        for(int j=0;j<1024;j++)
            bufferFFLASH[i][j]=0xFF;
    }


    address=0;

    hexRead[0]='0';
    hexRead[1]='x';
    hexRead[4]='\0';

    firstpage = -1;

    while(!hexFile->atEnd()){

//        QCoreApplication::processEvents(QEventLoop::AllEvents,100);

        hexFile->read(&hexRead[2],1);

        if(hexRead[2]==':'){

            hexFile->read(&hexRead[2],2);
            size = strtol(hexRead,NULL,16);

            hexFile->read(&hexRead[2],2);
            ads = strtol(hexRead,NULL,16)*256;
            hexFile->read(&hexRead[2],2);
            ads += strtol(hexRead,NULL,16);

            hexFile->read(&hexRead[2],2);
            mode = strtol(hexRead,NULL,16);

            switch(mode){
            case 0x00://DATOS
                cks = size+mode+(ads & 0x000000FF)+((ads >> 8) & 0x000000FF);
                auxaddress = address+ads;
                for(int j=0; j<size; j++){
                    if(firstpage == -1){
                        firstpage = (auxaddress-0x08000000)/1024;
                    }
                    numLastPage = (auxaddress-0x08000000)/1024;
                    hexFile->read(&hexRead[2],2);
                    value = strtol(hexRead,NULL,16);
                    bufferFFLASH[numLastPage][(auxaddress-0x08000000)%1024] = value;
                    cks += value;
                    auxaddress++;
                }
                cks = (~(cks)+1);
                hexFile->read(&hexRead[2],2);
                value = strtol(hexRead,NULL,16);
                if(value != cks)
                    return false;
                break;
            case 0x01://FIN
                ui->plainTextEdit->clear();
                pageindex = -1;
                ui->progressBar->setValue(0);
                ui->plainTextEdit_3->appendPlainText("LOADING File FLASH ...");
                for(int i1=0; i1<128; i1++){
                    ui->progressBar->setValue(i1*100/128);
                    str = QString().asprintf("LOADING File FLASH %%%2d", i1*100/128);
                    ui->label_3->setText(str);
                    address = i1*1024;
                    str = IntToHex(address+0x08000000, 8) + ":  ";
                    for(int j=0; j<1024; j++){
                        str = str+IntToHex(bufferFFLASH[i1][j],2)+" ";
                        if((j%8) == 7)
                            str = str+" ";
                        if((j%16) == 15){
                            if(pageindex != i1){
                                str = str+" -> PAGE "+IntToStr(i1);
                                if(i1 == (int)numLastPage){
                                    str = str + " LP";
//                                    ui->plainTextEdit->appendPlainText("FP: "+IntToStr(firstpage));
//                                    ui->plainTextEdit->appendPlainText("LP: "+IntToStr(numLastPage));
                                }
                                pageindex = i1;
                            }
                            ui->plainTextEdit->appendPlainText(str);
                            address += 16;
                            str = IntToHex(address+0x08000000, 8)+":  ";
                        }
                    }
//                    QCoreApplication::processEvents(QEventLoop::AllEvents,100);
                }
                str = QString().asprintf("LOADING File FLASH %%%2d", 100);
                ui->label_3->setText(str);
                ui->plainTextEdit_3->appendPlainText("LOADING File FLASH ... OK");
                ui->progressBar->setValue(0);
                break;
            case 0x02://Dirección Extendida de Segmento (ads<<4)
                hexFile->read(&hexRead[2],2);
                address = strtol(hexRead,NULL,16);
                hexFile->read(&hexRead[2],2);
                address += (strtol(hexRead,NULL,16)*256);
                address *= 16;
                break;
            case 0x03://Dirección de Cominenzo de Segmento (CS:IP)
                break;
            case 0x04://Dirección Lineal Extendidad (ads<<8)
                hexFile->read(&hexRead[2],2);
                address = strtol(hexRead,NULL,16)*256;
                hexFile->read(&hexRead[2],2);
                address += strtol(hexRead,NULL,16);
                address <<= 16;
                break;
            case 0x05://Comienzo Dirección Lineal
                hexFile->read(&hexRead[2],2);
                w.u8[3] = strtol(hexRead,NULL,16);
                hexFile->read(&hexRead[2],2);
                w.u8[2] = strtol(hexRead,NULL,16);
                hexFile->read(&hexRead[2],2);
                w.u8[1] = strtol(hexRead,NULL,16);
                hexFile->read(&hexRead[2],2);
                w.u8[0] = strtol(hexRead,NULL,16);
                address = w.u32;
                break;
            }
        }
    }
    delete hexFile;

    return true;
}

bool QForm1::FileFLASHtoBUFBIN(QString AFileFlash){
    int address;
    QString str;
    work w;

    QFile *binFile = new QFile();
    binFile->setFileName(AFileFlash);
    if(!binFile->open(QFile::ReadOnly)){
        delete binFile;
        return false;
    }

    if(binFile->size()==0)
        return false;

    numLastPage = binFile->size()/1024;

    for(int i=0;i<128;i++){
        for(int j=0;j<1024;j++)
            bufferFFLASH[i][j]=0xFF;
    }


    address=0;

    while(!binFile->atEnd()){
        binFile->read((char *)&w.u32, 4);
        bufferFFLASH[address/1024][address%1024] = w.u8[0];
        address++;
        bufferFFLASH[address/1024][address%1024] = w.u8[1];
        address++;
        bufferFFLASH[address/1024][address%1024] = w.u8[2];
        address++;
        bufferFFLASH[address/1024][address%1024] = w.u8[3];
        address++;
    }

    ui->plainTextEdit->clear();
    pageindex = -1;
    ui->progressBar->setValue(0);
    ui->plainTextEdit_3->appendPlainText("LOADING File FLASH ...");
    for(int i1=0; i1<128; i1++){
        ui->progressBar->setValue(i1*100/128);
        address = i1*1024;
        str = IntToHex(address+0x08000000, 8) + ":  ";
        for(int j=0; j<1024; j++){
            str = str+IntToHex(bufferFFLASH[i1][j],2)+" ";
            if((j%8) == 7)
                str = str+" ";
            if((j%16) == 15){
                if(pageindex != i1){
                    str = str+" -> PAGE "+IntToStr(i1);
                    if(i1 == (int)numLastPage){
                        str = str + " LP";
                    }
                    pageindex = i1;
                }
                ui->plainTextEdit->appendPlainText(str);
                address += 16;
                str = IntToHex(address+0x08000000, 8)+":  ";
            }
        }
//                    QCoreApplication::processEvents(QEventLoop::AllEvents,100);
    }
    ui->plainTextEdit_3->appendPlainText("LOADING File FLASH ... OK");
    ui->progressBar->setValue(0);


    delete binFile;

    return true;

}

void QForm1::ShowMCUFLASH(){
    uint32_t address;
    QString str;

    ui->plainTextEdit_2->clear();
    pageindex = -1;
    for(int i1=0; i1<128; i1++){
        address = i1*1024;
        str = IntToHex(address+0x08000000, 8) + ":  ";
        for(int j=0; j<1024; j++){
            str = str+IntToHex(bufferMCUFLASH[i1][j], 2)+" ";
            if((j%8) == 7)
                str = str+" ";
            if((j%16) == 15){
                if(pageindex != i1){
                    str = str+" -> PAGE "+IntToStr(i1);
                    if(i1 == (int)numLastPage){
                        str = str + " LP";
                    }
                    pageindex = i1;
                }
                ui->plainTextEdit_2->appendPlainText(str);
                address += 16;
                str = IntToHex(address+0x08000000, 8)+":  ";
            }
        }
    }
}



void QForm1::on_pushButton_clicked()
{
    QString fileName;

    fileName = "";
    fileName = QFileDialog::getOpenFileName(this,
                tr("Open HEX or BIN file"), "", tr("HEX File (*.hex *.bin)"));
    if(fileName != ""){
        if(fileName.contains(".bin"))
            FileFLASHtoBUFBIN(fileName);
        if(fileName.contains(".hex"))
            FileFLASHtoBUF(fileName);
        ui->lineEdit->setText(fileName);
    }
}

void QForm1::on_pushButton_6_clicked()
{
    if(cmdWorking){
        ui->plainTextEdit_3->appendPlainText("There is a CMD RUNNING!");
        return;
    }

    if(QSerialPort1->isOpen()){
        QSerialPort1->close();
        ui->pushButton_6->setText("OPEN PORT");
        ui->statusbar->showMessage("SET a COM PORT");
        ui->pushButton_2->setEnabled(false);
        ui->pushButton_3->setEnabled(false);
        ui->pushButton_4->setEnabled(false);
    }
    else{
        if(ui->comboBox->currentText() != ""){
            QSerialPort1->setPortName(ui->comboBox->currentText());
            QSerialPort1->setParity(QSerialPort::EvenParity);
            QSerialPort1->setDataBits(QSerialPort::Data8);
            QSerialPort1->setStopBits(QSerialPort::OneStop);
            if(QSerialPort1->open(QSerialPort::ReadWrite)){
                ui->statusbar->showMessage("OPENED: "+QSerialPort1->portName()+" 8, 1, EVEN");
                ui->pushButton_6->setText("CLOSE PORT");
            }
            else
                ui->plainTextEdit_3->appendPlainText("ERROR OPENING PORT " + QSerialPort1->portName());
        }
        else
            ui->statusbar->showMessage("SET a COM PORT");
    }
}

void QForm1::on_pushButton_5_clicked()
{
    if(timeOutTryingBoot)
        return;

    if(cmdWorking){
        ui->plainTextEdit_3->appendPlainText("There is a CMD RUNNING!");
        return;
    }

    if(ui->comboBox->currentText() == ""){
        QMessageBox::information(this, "BLUEPILL PROGRAMER", "Please SELECT a COM PORT");
        return;

    }
    if(!QSerialPort1->isOpen()){
        QMessageBox::information(this, "BLUEPILL PROGRAMER", "Please open a COM PORT");
        return;
    }

    ui->pushButton_2->setEnabled(false);
    ui->pushButton_3->setEnabled(false);
    ui->pushButton_4->setEnabled(false);
    timeOutTryingBoot = 2000/50;
    ui->plainTextEdit_3->appendPlainText("TESTING BOOT ...");
}

void QForm1::on_pushButton_4_clicked()
{
    if(cmdWorking){
        ui->plainTextEdit_3->appendPlainText("There is a CMD RUNNING!");
        return;
    }

    if(ui->lineEdit->text() == ""){
        QMessageBox::information(this, "BLUEPILL PROGRAMMER", "Please SELECT A FILE");
        return;
    }
    if(!QSerialPort1->isOpen()){
        QMessageBox::information(this, "BLUEPILL PROGRAMER", "Please open a COM PORT");
        return;
    }

    for (int i=0; i<128; i++) {
        for (int j=0; j<1024; j++)
            bufferMCUFLASH[i][j] = 0xFF;
    }

    checkFlash = true;
    ui->progressBar->setValue(0);
    numPage = 0;
    memAddress = 0x08000000;
    nBytesMem = 16;
    ui->plainTextEdit_3->appendPlainText("READING FLASH ...");
    cmdList.append(READ_MEMORY);

}

void QForm1::on_pushButton_3_clicked()
{
    if(cmdWorking){
        ui->plainTextEdit_3->appendPlainText("There is a CMD RUNNING!");
        return;
    }

    if(ui->lineEdit->text() == ""){
        QMessageBox::information(this, "BLUEPILL PROGRAMMER", "Please SELECT A FILE");
        return;
    }
    if(!QSerialPort1->isOpen()){
        QMessageBox::information(this, "BLUEPILL PROGRAMER", "Please open a COM PORT");
        return;
    }

    numPage = 0;
    memAddress = 0x08000000;
    ui->progressBar->setValue(0);
    ui->plainTextEdit_3->appendPlainText("WRITING FLASH ...");
    cmdList.append(ERASE_PAGE);
}

void QForm1::on_pushButton_2_clicked()
{
    if(cmdWorking){
        ui->plainTextEdit_3->appendPlainText("There is a CMD RUNNING!");
        return;
    }

    if(!QSerialPort1->isOpen()){
        QMessageBox::information(this, "BLUEPILL PROGRAMER", "Please open a COM PORT");
        return;
    }

    for (int i=0; i<128; i++) {
        for (int j=0; j<1024; j++)
            bufferMCUFLASH[i][j] = 0xFF;
    }
    checkFlash = false;
    ui->progressBar->setValue(0);
    numPage = 0;
    memAddress = 0x08000000;
    nBytesMem = 16;
    ui->plainTextEdit_3->appendPlainText("READING FLASH ...");
    cmdList.append(READ_MEMORY);
}




#ifndef QFORM1_H
#define QFORM1_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <QSerialPortInfo>

typedef union{
    unsigned char u8[4];
    unsigned short u16[2];
    short i16[2];
    unsigned int u32;
    int i32;
    float f;
}work;

typedef enum{
    SEQ_TEST_BOOT,
    SEQ_SEND_CMD,
    SEQ_WAIT_ACK,
    SEQ_WAIT_NBYTES,
    SEQ_WAIT_BYTES,
    SEQ_MEM_ADDRESS_ACK,
    SEQ_MEM_NBYTES_ACK,
    SEQ_MEM_NBYTES,
    SEQ_WRMEN_ADDRES_ACK,
    SEQ_WRMEN_NBYTES_ACK,
    SEQ_WRMEN_NBYTES,
    SEQ_ERASEPAGE_NPAGES_ACK,
    SEQ_ERASEPAGE_NPAGES,
    SEQ_IDLE,
}_eSequence;

#define IN_BOOT                 0x7F
#define GET_CMD_VERSION         0x00
#define GET_BOOT_VERSION        0x01
#define GET_CHIP_ID             0x02
#define READ_MEMORY             0x11
#define GO_FLASH                0x21
#define WRITE_MEMORY            0x31
#define ERASE_PAGE              0x43
#define EXTENDED_ERASE          0x44
#define WRITE_PROTECT           0x63
#define WRITE_UNPROTECT         0x73
#define READOUT_PROTECT         0x82
#define READOUT_UNPROTECT       0x92
#define ACK                     0x79
#define NACK                    0x1F
#define ADDRESS_MEMORY          0xF0
#define N_BYTES                 0xF1




QT_BEGIN_NAMESPACE
namespace Ui { class QForm1; }
QT_END_NAMESPACE

class QForm1 : public QMainWindow
{
    Q_OBJECT

public:
    QForm1(QWidget *parent = nullptr);
    ~QForm1();

private slots:
    void OnQTimer1();
    void OnDataQSerialPort1();

    bool eventFilter(QObject *watched, QEvent *event) override;

    void on_pushButton_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_2_clicked();


private:
    Ui::QForm1 *ui;

    QSerialPort *QSerialPort1;
    QTimer *QTimer1;

    QList<uint8_t> cmdList;

    QString IntToHex(int Value, int Digits);
    QString IntToStr(int Value);


    uint8_t bufferFFLASH[128][1024];
    uint8_t bufferMCUFLASH[128][1024];
    int pageindex;
    bool FileFLASHtoBUF(QString AFileFlash);
    bool FileFLASHtoBUFBIN(QString AFileFlash);
    void ShowMCUFLASH();

    uint8_t rx[1024], tx[1024], indexrxr, indexrxw;

    int waitingNBytes, timeOutCMD, timeOutTryingBoot, timeOutACK, timeOutNBytes;
    bool tryingBoot, checkFlash, readMemorySize, cmdWorking;
    uint8_t lastCMD;
    uint16_t nBytesMem;
    uint32_t memAddress, numPage, quarterPageNumber, memFlashSize, numLastPage;
    _eSequence sequence;

};
#endif // QFORM1_H

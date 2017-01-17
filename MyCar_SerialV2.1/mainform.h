#ifndef MAINFORM_H
#define MAINFORM_H

#include <QMainWindow>
#include <QCheckBox>
#include <QWidget>
#include <QTimer>
#include "qextserialport.h"
#include "qextserialenumerator.h"
#include "hled.h"
#include <QCloseEvent>

namespace Ui {
class mainform;
}

class mainform : public QMainWindow
{
    Q_OBJECT

public:
    explicit mainform(QWidget *parent = 0);
    ~mainform();
    void CloseCom();

private:
    Ui::mainform *ui;
    QextSerialEnumerator *m_Com_Monitor;
    QextSerialPort *m_Com;
    HLed *m_led;
    QTimer *autoTimer;
    QStringList m_Command;

    void writeCommand(QString str);
    int currentCommand;
    void checkCommand();
    QList<QCheckBox *> sendHexCheck;
    QList<QCheckBox *> readHexCheck;
    QList<QCheckBox *> isuseCheck;

    void AutoReply(QByteArray data);

    QString toBlueText(QString str);
    QString toBlackText(QString str);

    quint64 m_number_send;
    quint64 m_number_recive;
    void showCountNumber();
    void stringToHtmlFilter(QString &str);
    void stringToHtml(QString &str,QColor crl);

    void Leather_Data_Receive(QByteArray data);
    void Leather_Data_Send(bool my_switch_1,bool my_switch_2,quint16 speed,quint8 flag);

private slots:
    void on_pushButton_openClose_clicked();

    void portName_changed(QString name);
    void buadRate_changed(int idx);
    void dataBits_changed(int idx);
    void stopBits_changed(int idx);
    void parity_changed(int idx);


    void on_pushButton_3_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_toolButton_2_clicked();

    void on_toolButton_clicked();

    void on_checkBox_3_clicked(bool checked);

    void autoWrite();

    void hasComDiscovered(const QextPortInfo &info);
    void hasComRemoved(const QextPortInfo &info);

    void on_pushButton_4_clicked();

    void readMyCom();
    void on_openMotorButton_clicked();
    void on_closeMotorButton_clicked();
    void on_verticalSlider_sliderReleased();
};

#endif // MAINFORM_H

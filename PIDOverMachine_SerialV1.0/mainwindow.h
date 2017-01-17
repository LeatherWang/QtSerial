#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCheckBox>
#include <QWidget>
#include <QTimer>
#include "qextserialport.h"
#include "qextserialenumerator.h"
#include <QCloseEvent>

namespace Ui {
class MainWindow;
}

//@TODO 使用共用体传输浮点型数据
struct PID_param_st_pk
{
    quint16 kp;			 //比例系数
    quint16 ki;			 //积分系数
    quint16 kd;		 	 //微分系数
    quint16 ur;          //限幅值

};
struct _save_param_st_pk
{
    PID_param_st_pk PID_1; //12字节，3个float
    PID_param_st_pk PID_2;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QextSerialEnumerator *m_Com_Monitor;
    QextSerialPort *m_Com;
    struct _save_param_st_pk lEATHER_Param;

private slots:
    void portName_changed(QString name);
    void buadRate_changed(int idx);
    void dataBits_changed(int idx);
    void stopBits_changed(int idx);
    void parity_changed(int idx);

    void hasComDiscovered(const QextPortInfo &info);
    void hasComRemoved(const QextPortInfo &info);

    void readMyCom();
    void on_closeMyComBtn_2_clicked();
    void on_openMyComBtn_2_clicked();

    void on_sendMsgBtn_3_clicked();
    void on_Msg_PID_P1_3_textChanged(const QString &arg1);
    void on_Msg_PID_I1_3_textChanged(const QString &arg1);
    void on_Msg_PID_D1_3_textChanged(const QString &arg1);
    void on_Msg_PID_U1_3_textChanged(const QString &arg1);
    void on_sendMsgBtn_4_clicked();
    bool checkMaxMin(float data);
    void on_Msg_PID_P1_4_textChanged(const QString &arg1);
    void on_Msg_PID_I1_4_textChanged(const QString &arg1);
    void on_Msg_PID_D1_4_textChanged(const QString &arg1);
    void on_Msg_PID_U1_4_textChanged(const QString &arg1);
    void on_clear_Button_clicked();
    void chang(quint8 flag,const PID_param_st_pk *PID_param);
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
};

#endif // MAINWINDOW_H

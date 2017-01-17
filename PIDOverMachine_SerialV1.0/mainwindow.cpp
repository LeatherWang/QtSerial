#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "qextserialenumerator.h"
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(726,478);
    setWindowTitle("调参上位机ByPigeWang--V2.0");
    ui->closeMyComBtn_2->setEnabled(false);
    ui->sendMsgBtn_3->setEnabled(false);
    ui->sendMsgBtn_4->setEnabled(false);
    ui->pushButton->setEnabled(false);
    ui->pushButton_2->setEnabled(false);
    ui->label->setText("串口状态：未打开！");

    m_Com = new QextSerialPort(QextSerialPort::EventDriven,this);
    m_Com->setPortName("-1");
    m_Com_Monitor = new QextSerialEnumerator();//串口监视器，发布串口增加、移除等信号
    m_Com_Monitor->setUpNotifications();
    //设置串口所需的Items
    {
    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())//利用此循环将serialList显示在portBox中
        ui->portBox->addItem(info.portName);

    ui->portBox->setEditable(false);//set ture to make sure user can input their own port name!

    ui->baudRateBox->addItem("110", BAUD110);
    ui->baudRateBox->addItem("300", BAUD300);
    ui->baudRateBox->addItem("600", BAUD600);
    ui->baudRateBox->addItem("1200", BAUD1200);
    ui->baudRateBox->addItem("2400", BAUD2400);
    ui->baudRateBox->addItem("4800", BAUD4800);
    ui->baudRateBox->addItem("9600", BAUD9600);
    ui->baudRateBox->addItem("19200", BAUD19200);
    ui->baudRateBox->addItem("38400", BAUD38400);
    ui->baudRateBox->addItem("57600", BAUD57600);
    ui->baudRateBox->addItem("115200", BAUD115200);
    #ifdef Q_WS_WIN
        ui->baudRateBox->addItem("14400", BAUD14400);
        ui->baudRateBox->addItem("56000", BAUD56000);
        ui->baudRateBox->addItem("128000", BAUD128000);
        ui->baudRateBox->addItem("256000", BAUD256000);
    #endif
    ui->baudRateBox->setCurrentIndex(10);

    ui->parityBox->addItem("NONE", PAR_NONE);
    ui->parityBox->addItem("ODD", PAR_ODD);
    ui->parityBox->addItem("EVEN", PAR_EVEN);
    ui->parityBox->setCurrentIndex(0);

    ui->dataBitsBox->addItem("5", DATA_5);
    ui->dataBitsBox->addItem("6", DATA_6);
    ui->dataBitsBox->addItem("7", DATA_7);
    ui->dataBitsBox->addItem("8", DATA_8);
    ui->dataBitsBox->setCurrentIndex(3);

    ui->stopBitsBox->addItem("1", STOP_1);
    ui->stopBitsBox->addItem("2", STOP_2);
    ui->stopBitsBox->setCurrentIndex(0);
    }

    connect(ui->baudRateBox,SIGNAL(currentIndexChanged(int)),this,SLOT(buadRate_changed(int)));
    connect(ui->portBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(portName_changed(QString)));
    connect(ui->dataBitsBox,SIGNAL(currentIndexChanged(int)),this,SLOT(dataBits_changed(int)));
    connect(ui->stopBitsBox,SIGNAL(currentIndexChanged(int)),this,SLOT(stopBits_changed(int)));
    connect(ui->parityBox,SIGNAL(currentIndexChanged(int)),this,SLOT(parity_changed(int)));

    connect(m_Com,SIGNAL(readyRead()),this,SLOT(readMyCom()));
    connect(m_Com_Monitor,SIGNAL(deviceDiscovered(const QextPortInfo&)),this,SLOT(hasComDiscovered(const QextPortInfo&)));
    connect(m_Com_Monitor,SIGNAL(deviceRemoved(const QextPortInfo&)),this,SLOT(hasComRemoved(const QextPortInfo&)));

    /****************************** Start user code for include. **********************************/
    lEATHER_Param.PID_1.kp = 0;lEATHER_Param.PID_1.ki = 0;lEATHER_Param.PID_1.kd = 0;lEATHER_Param.PID_1.ur = 0;
    lEATHER_Param.PID_2.kp = 0;lEATHER_Param.PID_2.ki = 0;lEATHER_Param.PID_2.kd = 0;lEATHER_Param.PID_2.ur = 0;
    /********************************* End user code. *********************************************/
}

MainWindow::~MainWindow()
{
    if(m_Com->isOpen())
        m_Com->close();
    delete m_Com;
    delete ui;
}

/*******************************************
  *类型：槽
  *对应信号：串口名下拉列表值改变
  *功能：   如果串口打开，关闭串口
  *****************************************/
void MainWindow::portName_changed(QString name)
{
    if (m_Com->isOpen()&&m_Com->portName()!=name)
    {
        m_Com->close();
    }
}
//设置当前波特率
void MainWindow::buadRate_changed(int idx)
{
    m_Com->setBaudRate((BaudRateType)ui->baudRateBox->itemData(idx).toInt());
}
//设置当前停止位
void MainWindow::stopBits_changed(int idx)
{
    m_Com->setStopBits((StopBitsType)ui->stopBitsBox->itemData(idx).toInt());
}
//设置当前数据位
void MainWindow::dataBits_changed(int idx)
{
    m_Com->setDataBits((DataBitsType)ui->dataBitsBox->itemData(idx).toInt());
}
//设置当前校验位
void MainWindow::parity_changed(int idx)
{
    m_Com->setParity((ParityType)ui->parityBox->itemData(idx).toInt());
}

/************************************************
  *类型：槽函数
  *对应信号：有新的串口设备被发现
  *功能：在串口下拉列表中加入改串口设备
  *************************************************/
void MainWindow::hasComDiscovered(const QextPortInfo &info)
{
    ui->portBox->addItem(info.portName);
    if(!m_Com->isOpen())
        ui->portBox->setCurrentIndex(ui->portBox->count()-1);
}

/************************************************
  *类型：槽函数
  *对应信号：有串口设备断开连接
  *功能：如果断开的设备是当前打开的设备，就关闭改串口及定时器
        提示用户
        如果不是当前设备，就在串口下拉列表中删除该设备
  *************************************************/
void MainWindow::hasComRemoved(const QextPortInfo &info)
{
    if(info.portName==ui->portBox->currentText())
    {
    }
    for(int i=0;i<ui->portBox->count();i++)
    {
        if(ui->portBox->itemText(i)==info.portName)
            ui->portBox->removeItem(i);
    }
}

bool reflag = false;
void MainWindow::on_closeMyComBtn_2_clicked()
{
  m_Com->close();
  ui->closeMyComBtn_2->setEnabled(false);
  ui->openMyComBtn_2->setEnabled(true);
  ui->sendMsgBtn_3->setEnabled(false);
  ui->sendMsgBtn_4->setEnabled(false);
  ui->pushButton->setEnabled(false);
  ui->pushButton_2->setEnabled(false);
  ui->label->setText("串口状态：未打开！");
  ui->textBrowser->clear();
  reflag = false;
}

/*************************************************
  *类型：公有函数
  *功能:检查串口是否关闭，如果未关闭关闭串口
        释放timer、com等的内存
  *注：由于在tabWidget中remove不能释放内存，故写此函数
      关闭串口，释放内存
  ***********************************************/
void MainWindow::on_openMyComBtn_2_clicked()
{
    m_Com->setPortName(ui->portBox->itemText(ui->portBox->currentIndex()));
    m_Com->setBaudRate((BaudRateType)ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
    m_Com->setDataBits((DataBitsType)ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());
    m_Com->setParity((ParityType)ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
    m_Com->setStopBits((StopBitsType)ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());

    if(m_Com->open(QIODevice::ReadWrite))
    {
        qDebug()<<"open Port succeed!";
        ui->label->setText("串口状态：<font color='blue'>打开成功！</font>");
        ui->openMyComBtn_2->setEnabled(false);
        ui->closeMyComBtn_2->setEnabled(true);
        ui->sendMsgBtn_3->setEnabled(true);
        ui->sendMsgBtn_4->setEnabled(true);
        ui->pushButton->setEnabled(true);
        ui->pushButton_2->setEnabled(true);
    }
    else
    {
        qDebug()<<"open Port failed";
        ui->label->setText("串口状态：<font color='red'>打开失败！！！</font>");
    }

}





/****************************** Start user code for include. **********************************/
/*************************************************
  *类型：槽函数
  *功能: 读串口数据，系统自动调用
  *注：  有数据就调用该函数，不能保证一帧完整数据!!
  ***********************************************/
void MainWindow::readMyCom()
{
    QByteArray temp = m_Com->readAll();
    QString temp_1 = temp;
    //读取串口缓冲区的所有数据给临时变量temp
    if(reflag == false)
    {
    ui->textBrowser->insertPlainText("接收到数据！！！"); //提示收到数据
    reflag = true;
    }
}

bool MainWindow::checkMaxMin(float data)
{
    if(!((-32768.0<data)&&(data<32767.0)))
    {
        QMessageBox::information(this,QStringLiteral("错误信息"),QStringLiteral("数据大小超出范围！"));
        return false;
    }
    else
    {
        return true;
    }
}
void MainWindow::on_sendMsgBtn_3_clicked()
{
    chang(0x01,&lEATHER_Param.PID_1);
}
void MainWindow::on_Msg_PID_P1_3_textChanged(const QString &arg1)
{
    if(checkMaxMin(arg1.toFloat()))
        lEATHER_Param.PID_1.kp = arg1.toInt();

}
void MainWindow::on_Msg_PID_I1_3_textChanged(const QString &arg1)
{
    if(checkMaxMin(arg1.toFloat()))
        lEATHER_Param.PID_1.ki = arg1.toInt();
}
void MainWindow::on_Msg_PID_D1_3_textChanged(const QString &arg1)
{
    if(checkMaxMin(arg1.toFloat()))
        lEATHER_Param.PID_1.kd = arg1.toInt();
}
void MainWindow::on_Msg_PID_U1_3_textChanged(const QString &arg1)
{
    if(checkMaxMin(arg1.toFloat()))
        lEATHER_Param.PID_1.ur = arg1.toInt();
}
void MainWindow::on_sendMsgBtn_4_clicked()
{
    chang(0x02,&lEATHER_Param.PID_2);
}
void MainWindow::on_Msg_PID_P1_4_textChanged(const QString &arg1)
{
    if(checkMaxMin(arg1.toFloat()))
        lEATHER_Param.PID_2.kp = arg1.toInt();
}
void MainWindow::on_Msg_PID_I1_4_textChanged(const QString &arg1)
{
    if(checkMaxMin(arg1.toFloat()))
        lEATHER_Param.PID_2.ki = arg1.toInt();
}
void MainWindow::on_Msg_PID_D1_4_textChanged(const QString &arg1)
{
    if(checkMaxMin(arg1.toFloat()))
        lEATHER_Param.PID_2.kd = arg1.toInt();
}
void MainWindow::on_Msg_PID_U1_4_textChanged(const QString &arg1)
{
    if(checkMaxMin(arg1.toFloat()))
        lEATHER_Param.PID_2.ur = arg1.toInt();
}
#define BYTE0(dwTemp)       ( *( (quint8 *)(&dwTemp)))
#define BYTE1(dwTemp)       ( *( (quint8 *)(&dwTemp) + 1) )
void MainWindow::chang(quint8 flag, const PID_param_st_pk *PID_param)
{
    QByteArray array;
    unsigned char _cnt=0;
    unsigned char i=0;
    unsigned char sum = 0;

    array[_cnt++]=0xAA;
    array[_cnt++]=0xAF;
    array[_cnt++]=flag;//PID标识位
    array[_cnt++]=0;

    array[_cnt++]=BYTE1(PID_param->kp);
    array[_cnt++]=BYTE0(PID_param->kp);
    array[_cnt++]=BYTE1(PID_param->ki);
    array[_cnt++]=BYTE0(PID_param->ki);
    array[_cnt++]=BYTE1(PID_param->kd);
    array[_cnt++]=BYTE0(PID_param->kd);
    array[_cnt++]=BYTE1(PID_param->ur);
    array[_cnt++]=BYTE0(PID_param->ur);
    array[3] = _cnt-4;

     for(i=0;i<_cnt;i++)
        sum += array[i];

    array[_cnt++]=sum;
    m_Com->write(array);
}
void MainWindow::on_clear_Button_clicked()
{
    ui->textBrowser->clear();
}

//舵机1开关
quint8 flag=0x01;
void MainWindow::on_pushButton_clicked()
{

    QByteArray array;
    unsigned char _cnt=0;
    unsigned char i=0;
    unsigned char sum = 0;
    if(flag == 0x01)
    {
        flag = 0x02;
        ui->pushButton->setText("打开");
    }
    else if(flag ==0x02)
    {
        flag =0x01;
        ui->pushButton->setText("闭合");
    }
    array[_cnt++]=0xAA;
    array[_cnt++]=0xAF;
    array[_cnt++]=0x01;
    array[_cnt++]=0;

    array[_cnt++]=flag;
    array[3] = _cnt-4;

     for(i=0;i<_cnt;i++)
        sum += array[i];

    array[_cnt++]=sum;
    m_Com->write(array);
}

//舵机2开关
quint8 flag_2=0x01;
void MainWindow::on_pushButton_2_clicked()
{
    QByteArray array;
    unsigned char _cnt=0;
    unsigned char i=0;
    unsigned char sum = 0;
    if(flag_2 == 0x01)
    {
        flag_2 = 0x02;
        ui->pushButton_2->setText("打开");
    }
    else if(flag_2 ==0x02)
    {
        flag_2 =0x01;
        ui->pushButton_2->setText("闭合");
    }
    array[_cnt++]=0xAA;
    array[_cnt++]=0xAF;
    array[_cnt++]=0x01;
    array[_cnt++]=0;

    array[_cnt++]=flag_2;
    array[3] = _cnt-4;

     for(i=0;i<_cnt;i++)
        sum += array[i];

    array[_cnt++]=sum;
    m_Com->write(array);
}
/********************************* End user code. *********************************************/













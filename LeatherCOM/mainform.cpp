#include "mainform.h"
#include "ui_mainform.h"
#include <QDebug>
#include "qextserialenumerator.h"
#include <QDebug>
#include <QMessageBox>

mainform::mainform(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::mainform)
{
    ui->setupUi(this);

    setWindowTitle("LeatherWangSerial");
    m_number_send = 0;
    m_number_recive =0;
    showCountNumber();

    ui->closeMotorButton->setEnabled(false);
    ui->openMotorButton->setEnabled(true);

    m_Com = new QextSerialPort(QextSerialPort::EventDriven,this);
    m_Com->setPortName("-1");
    m_Com_Monitor = new QextSerialEnumerator();
    m_Com_Monitor->setUpNotifications();

    ui->textBrowser->setOpenExternalLinks(true);
    //设置表格列宽
    ui->tableWidget->setColumnWidth(0,30);
    ui->tableWidget->setColumnWidth(1,200);
    ui->tableWidget->setColumnWidth(2,30);
    ui->tableWidget->setColumnWidth(3,200);
    ui->tableWidget->setColumnWidth(4,30);
    //设置表格整行选取
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);


    //初始化led灯
    m_led = new HLed;
    m_led->turnOff();
    ui->horizontalLayout_6->insertWidget(0,m_led);
    ui->horizontalLayout_6->setStretch(0,1);
    ui->horizontalLayout_6->setStretch(1,4);

    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
        ui->portBox->addItem(info.portName);
    //make sure user can input their own port name!
    ui->portBox->setEditable(true);

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

    ui->dataBitsBox->addItem("5", DATA_5);
    ui->dataBitsBox->addItem("6", DATA_6);
    ui->dataBitsBox->addItem("7", DATA_7);
    ui->dataBitsBox->addItem("8", DATA_8);
    ui->dataBitsBox->setCurrentIndex(3);

    ui->stopBitsBox->addItem("1", STOP_1);
    ui->stopBitsBox->addItem("2", STOP_2);

    connect(ui->baudRateBox,SIGNAL(currentIndexChanged(int)),this,SLOT(buadRate_changed(int)));
    connect(ui->portBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(portName_changed(QString)));
    connect(ui->dataBitsBox,SIGNAL(currentIndexChanged(int)),this,SLOT(dataBits_changed(int)));
    connect(ui->stopBitsBox,SIGNAL(currentIndexChanged(int)),this,SLOT(stopBits_changed(int)));
    connect(ui->parityBox,SIGNAL(currentIndexChanged(int)),this,SLOT(parity_changed(int)));

    //初始化自动发送定时器
    autoTimer = new QTimer(this);
    connect(autoTimer,SIGNAL(timeout()),this,SLOT(autoWrite()));

    //初始化自动发送的16进制选择按钮
    ui->tableWidget->setRowCount(10);
    ui->tableWidget->verticalHeader()->setVisible(false); //隐藏行表头
    for(int j=0;j<10;j++)
    {
        QCheckBox *sendCheck = new QCheckBox("");
        sendCheck->setChecked(true);
        ui->tableWidget->setCellWidget(j,2,sendCheck);
        sendHexCheck.append(sendCheck);

        QCheckBox *readCheck = new QCheckBox("");
        readCheck->setChecked(true);
        ui->tableWidget->setCellWidget(j,0,readCheck);
        readHexCheck.append(readCheck);

        QTableWidgetItem *itemAt1 = new QTableWidgetItem("");
        ui->tableWidget->setItem(j,1,itemAt1);
        QTableWidgetItem *itemAt3 = new QTableWidgetItem("");
        ui->tableWidget->setItem(j,3,itemAt3);

        QCheckBox *useCheck = new QCheckBox("");
        useCheck->setChecked(true);
        ui->tableWidget->setCellWidget(j,4,useCheck);
        isuseCheck.append(useCheck);
    }

    currentCommand = -1;
    checkCommand();

    connect(m_Com,SIGNAL(readyRead()),this,SLOT(readMyCom()));
    connect(m_Com_Monitor,SIGNAL(deviceDiscovered(const QextPortInfo&)),this,SLOT(hasComDiscovered(const QextPortInfo&)));
    connect(m_Com_Monitor,SIGNAL(deviceRemoved(const QextPortInfo&)),this,SLOT(hasComRemoved(const QextPortInfo&)));
}

mainform::~mainform()
{
    delete ui;
}

/********************************************
  *类型：槽
  *对应信号：读数据定时器时间到
  *功能：从串口读取数据，并显示在界面
        显示时要根据是否16进制而做改变
  *******************************************/
QByteArray byteArray;
QByteArray zhentou(2,0XAA);
void mainform::readMyCom()    //读串口发来的数据，并显示出来
{
    if(m_Com->bytesAvailable()>0)
    {
        //保持滚动条在最下方
        QByteArray data;
        ui->textBrowser->moveCursor(QTextCursor::End);
        if(ui->checkBox->isChecked())
        {
            QString str;
            data = m_Com->readAll();
            m_number_recive += data.size();
            for(int i=0;i<data.size();i++)
            {
                if(QString::number(quint8(data.at(i)),16).toUpper().length()==1)
                    str.append(QString("0")+QString::number(quint8(data.at(i)),16).toUpper()+QString(" "));
                else
                    str.append(QString::number(quint8(data.at(i)),16).toUpper()+QString(" "));
            }
            ui->textBrowser->insertHtml(toBlueText(str));

            byteArray.append(data);
            if(byteArray.size() >= 7)//TODO
            {
               // qDebug()<<byteArray;
                if(byteArray.contains(zhentou))
                {
                    QByteArray byteArray_1 =  byteArray.mid(byteArray.indexOf(zhentou),7);
                    if(byteArray_1.at(2) == 0x05)
                        byteArray_1 = byteArray.mid(byteArray.indexOf(zhentou),11);
                    Leather_Data_Receive(byteArray_1);
                }
                byteArray.clear();
            }
        }
        else
        {
            data = m_Com->readAll();
            m_number_recive += data.size();
            ui->textBrowser->insertHtml(toBlueText(QString::fromLocal8Bit(data)));
        }
        AutoReply(data);
        showCountNumber();
    }
}

void mainform::Leather_Data_Receive(QByteArray data)
{

        quint8 sum = 0;
        quint8 num = data.size();
        quint16 data_temp = 0;
        qDebug()<<data;
        for(quint8 i=0;i<(num-1);i++)
            sum += (quint8(data.at(i)));
        if(!(sum == quint8(data.at(num-1))))	return;		//判断sum，校验--Leather
        if(!(quint8(data.at(0))==0xAA && quint8(data.at(1))==0xAA))		return;		//判断帧头--Leather
        if(quint8(data.at(2)) == 0X01)//温度
        {
            data_temp = quint8(data.at(4));
            data_temp <<= 8;
            data_temp +=quint8(data.at(5));
            ui->label_6->setText(QString("室内温度：%1°C").arg(data_temp));
        }
        if(quint8(data.at(2)) == 0X02)//湿度
        {
            data_temp = quint8(data.at(4));
            data_temp <<= 8;
            data_temp +=quint8(data.at(5));
            ui->label_7->setText(QString("空气湿度：%1%").arg(data_temp));
        }
        if(quint8(data.at(2)) == 0X03)//光照
        {

            data_temp = quint8(data.at(4));
            data_temp <<= 8;
            data_temp +=quint8(data.at(5));
            ui->label_8->setText(QString("光照强度：%1cd").arg(data_temp));
        }
        if(quint8(data.at(2)) == 0X04)//土壤湿度
        {
            data_temp = quint8(data.at(4));
            data_temp <<= 8;
            data_temp +=quint8(data.at(5));
            ui->label_9->setText(QString("土壤湿度：%1%").arg((1050-data_temp)/10));
        }
        if(quint8(data.at(2)) == 0X05)//时间
        {
//            data_temp = (quint8(data.at(4)));
//            data_temp <<= 8;
//            data_temp +=quint8(data.at(5));
            ui->label_10->setText(QString("时间：%1时%2分%3秒").arg((quint8(data.at(4)))<<8 | (quint8(data.at(5))))
                                  .arg((quint8(data.at(6)))<<8 | (quint8(data.at(7))))
                                  .arg((quint8(data.at(8)))<<8 | (quint8(data.at(9)))));//(quint8(data.at(6)))<<8 | (quint8(data.at(7)))
        }
}
/****************************************
  *类型：       槽
  *对应信号：    开关按钮点击
  *功能：       如果串口已打开，将其关闭,关闭定时器。
               否则设置串口属性，打开串口，打开定时器。
  ***************************************/
void mainform::on_pushButton_openClose_clicked()
{
    if(m_Com->isOpen())
    {
        m_Com->close();
        m_led->turnOff();
    }else
    {
        m_Com->setPortName(ui->portBox->itemText(ui->portBox->currentIndex()));
        m_Com->setBaudRate((BaudRateType)ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
        m_Com->setDataBits((DataBitsType)ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());
        m_Com->setParity((ParityType)ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
        m_Com->setStopBits((StopBitsType)ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());

        if(m_Com->open(QIODevice::ReadWrite))
        {
            m_led->turnOn();
            qDebug()<<"open Port succeed!";
        }else
            qDebug()<<"open Port failed";
    }
}

/*******************************************
  *类型：槽
  *对应信号：串口名下拉列表值改变
  *功能：   如果串口打开，关闭串口
  *****************************************/
void mainform::portName_changed(QString name)
{
    if (m_Com->isOpen()&&m_Com->portName()!=name)
    {
        m_Com->close();
        m_led->turnOff();
    }
}

//设置当前波特率
void mainform::buadRate_changed(int idx)
{
    m_Com->setBaudRate((BaudRateType)ui->baudRateBox->itemData(idx).toInt());
}

//设置当前停止位
void mainform::stopBits_changed(int idx)
{
    m_Com->setStopBits((StopBitsType)ui->stopBitsBox->itemData(idx).toInt());
}

//设置当前数据位
void mainform::dataBits_changed(int idx)
{
    m_Com->setDataBits((DataBitsType)ui->dataBitsBox->itemData(idx).toInt());
}

//设置当前校验位
void mainform::parity_changed(int idx)
{
    m_Com->setParity((ParityType)ui->parityBox->itemData(idx).toInt());
}

/******************************************
  *类型：槽
  *对应信号：发送按钮点击
  *功能：根据是否16进制发送来发送数据
        清空发送栏
        并将发送的数据保存至命令列表
        命令前加有#######,代表是16进制发送的
  *****************************************/
void mainform::on_pushButton_3_clicked()
{
    if(!ui->textEdit->toPlainText().isEmpty())
    {
        if(ui->checkBox_2->isChecked())
            m_Command.append(QString("#######")+ui->textEdit->toPlainText());
        else
            m_Command.append(ui->textEdit->toPlainText());
        writeCommand(m_Command.value(m_Command.size()-1));
        //剔除重复项
        if(m_Command.value(m_Command.size()-1)==m_Command.value(m_Command.size()-2))
            m_Command.removeAt(m_Command.size()-1);
        currentCommand = m_Command.size();
        checkCommand();
    }
}

/**********************************************************
  *类型：私有函数
  *功能：通过串口发送一调数据，如果数据以#######开头表示16进制发送
        否则表示正常发送
  *参数:待发送的QString
  ********************************************************/
void mainform::writeCommand(QString str)
{
    //保持滚动条在最下方
    ui->textBrowser->moveCursor(QTextCursor::End);
    if(str.left(7)=="#######")
    {
        str = str.right(str.size()-7);
        bool ok;
        QByteArray data;
        QStringList list = str.split(" ");
        for(int i =0;i<list.size();i++)
        {
            char a = list.value(i).toInt(&ok,16);
            if(ok)
            {
                data.append(a);
            }else
            {
                QMessageBox::warning(this,"提示","非法的16进制数，请重新输入！");
                return ;
            }
        }
        m_Com->write(data);
        if(m_Com->isOpen()&&ui->checkBox_4->isChecked())
        {
            ui->textBrowser->insertHtml(toBlackText(str));
            m_number_send += data.size();
        }
    }else
    {
        m_Com->write(str.toLocal8Bit());
        if(m_Com->isOpen()&&ui->checkBox_4->isChecked())
        {
            ui->textBrowser->insertHtml(toBlackText(str));
            m_number_send += str.toLocal8Bit().size();
        }
    }
    if(!ui->checkBox_3->isChecked())
        //ui->textEdit->clear();
    showCountNumber();
}

//清空接收区
void mainform::on_pushButton_clicked()
{
    ui->textBrowser->setText("");
}

//清空发送区
void mainform::on_pushButton_2_clicked()
{
    ui->textEdit->setText("");
}

/******************************************
  *类型：槽函数
  *对应信号：向上切换命令按钮点击
  *功能：将发送区命令切换到上一条
        判断是否使用十六进制
  ******************************************/
void mainform::on_toolButton_2_clicked()
{
    currentCommand -=1;
    if(m_Command.value(currentCommand).left(7)=="#######")
    {
        ui->textEdit->setText(m_Command.value(currentCommand).right(m_Command.value(currentCommand).size()-7));
        ui->checkBox_2->setChecked(true);
    }else
    {
        ui->textEdit->setText(m_Command.value(currentCommand));
        ui->checkBox_2->setChecked(false);
    }
    checkCommand();
}

/******************************************
  *类型：槽函数
  *对应信号：向下切换命令按钮点击
  *功能：将发送区命令切换到下一条
        判断是否使用十六进制
  ******************************************/
void mainform::on_toolButton_clicked()
{
    currentCommand +=1;
    if(m_Command.value(currentCommand).left(7)=="#######")
    {
        ui->textEdit->setText(m_Command.value(currentCommand).right(m_Command.value(currentCommand).size()-7));
        ui->checkBox_2->setChecked(true);
    }else
    {
        ui->textEdit->setText(m_Command.value(currentCommand));
        ui->checkBox_2->setChecked(false);
    }
    checkCommand();
}

/***********************************************
  *类型：私有成员函数
  *功能：根据当前命令值以及，命令总数判断是否可以切换命令
        根据判断结果控制button是否可用
  **********************************************/
void mainform::checkCommand()
{
    ui->toolButton->setEnabled(true);
    ui->toolButton_2->setEnabled(true);
    if(currentCommand<=0)
        ui->toolButton_2->setEnabled(false);
    if(currentCommand>=m_Command.size())
        ui->toolButton->setEnabled(false);
    if(m_Command.size()==0)
    {
        ui->toolButton->setEnabled(false);
        ui->toolButton_2->setEnabled(false);
    }
}

/***********************************************
  *类型：槽
  *对应信号：自动发送选择按钮被点击
  *功能：开始或停止自动发送
        发送过程中进制选择时间
  **********************************************/
void mainform::on_checkBox_3_clicked(bool checked)
{
    if(checked)
    {
        autoTimer->start(ui->spinBox->value());
        ui->spinBox->setEnabled(false);
    }else
    {
        autoTimer->stop();
        ui->spinBox->setEnabled(true);
    }
}

/**********************************************************
  *类型：槽
  *对应信号：自动发送定时器时间到
  *功能：   判断编辑框是否有数据,然后发送
  **********************************************************/
void mainform::autoWrite()
{
    if(!ui->textEdit->toPlainText().isEmpty())
    {
        QString str;
        if(ui->checkBox_2->isChecked())
            str = QString("#######")+ui->textEdit->toPlainText();
        else
            str =ui->textEdit->toPlainText();
        writeCommand(str);
    }
}

/***********************************************************
  *类型：私有函数
  *功能：收到数据后判断是否有其对应的自动回复
        如果有自动就自动回复
  *参数：收到的数据
  ***********************************************************/
void mainform::AutoReply(QByteArray data)
{
    for(int i=0;i<10;i++)
    {
        if((!ui->tableWidget->item(i,1)->text().isEmpty())&&isuseCheck.value(i)->isChecked())
        {
            if(readHexCheck.value(i)->isChecked()==true)
            {
                QString str;
                for(int j=0;j<data.size();j++)
                {
                    if(QString::number(quint8(data.at(j)),16).toUpper().length()==1)
                        str.append(QString("0")+QString::number(quint8(data.at(j)),16).toUpper()+QString(" "));
                    else
                        str.append(QString::number(quint8(data.at(j)),16).toUpper()+QString(" "));
                }
                str = str.left(str.size()-1);
                if(str==ui->tableWidget->item(i,1)->text())
                {
                    QString sendStr;
                    if(sendHexCheck.value(i)->isChecked()==true)
                    {
                        sendStr = (QString("#######")+ui->tableWidget->item(i,3)->text());
                    }
                    writeCommand(sendStr);
                }
            }else
            {
                QString str = QString::fromLocal8Bit(data);
                if(str==ui->tableWidget->item(i,1)->text())
                {
                    QString sendStr = ui->tableWidget->item(i,3)->text();
                    if(sendHexCheck.value(i)->isChecked()==true)
                    {
                        sendStr = QString("#######")+ui->tableWidget->item(i,3)->text();
                    }
                    writeCommand(sendStr);
                }
            }

        }
    }
}

/*************************************************
  *类型：公有函数
  *功能:检查串口是否关闭，如果未关闭关闭串口
        释放timer、com等的内存
  *注：由于在tabWidget中remove不能释放内存，故写此函数
      关闭串口，释放内存
  ***********************************************/
void mainform::CloseCom()
{
    m_Com->close();
    delete m_Com;
//    delete readTimer;
    delete autoTimer;
    close();
}

/*******************************************************
  *类型：私有函数
  *功能：将一个文本字符串转换成蓝色的html文本颜色
        并换行
  ******************************************************/
QString mainform::toBlueText(QString str)
{
    QColor crl(0,0,255);
    stringToHtmlFilter(str);
    stringToHtml(str,crl);
    return str;
}

/*******************************************************
  *类型：私有函数
  *功能：将一个文本字符串转换成黑色的html文本颜色
        并换行
  ******************************************************/
QString mainform::toBlackText(QString str)
{
    QColor crl(0,0,0);
    stringToHtmlFilter(str);
    stringToHtml(str,crl);
    return str;
}

/************************************************
  *类型：槽函数
  *对应信号：有新的串口设备被发现
  *功能：在串口下拉列表中加入改串口设备
  *************************************************/
void mainform::hasComDiscovered(const QextPortInfo &info)
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
void mainform::hasComRemoved(const QextPortInfo &info)
{
    if(info.portName==ui->portBox->currentText())
    {
//        readTimer->stop();
        m_led->turnOff();
    }
    for(int i=0;i<ui->portBox->count();i++)
    {
        if(ui->portBox->itemText(i)==info.portName)
            ui->portBox->removeItem(i);
    }
}

/*****************************************************
  *类型：私有函数
  *功能：显示计数
  ****************************************************/
void mainform::showCountNumber()
{
    ui->label_4->setText(QString("已接受%1字节").arg(m_number_recive));
    ui->label_5->setText(QString("已发送%1字节").arg(m_number_send));
}

/********************************************************
  *类型：私有函数
  *功能：清空计数
  ****************************************************/
void mainform::on_pushButton_4_clicked()
{
    m_number_send = 0;
    m_number_recive = 0;
    showCountNumber();
}

/********************************************************
  *类型：私有函数
  *功能：替换特殊字符，帮助html显示
  ****************************************************/
void mainform::stringToHtmlFilter(QString &str)
{
    //注意这几行代码的顺序不能乱，否则会造成多次替换
    str.replace("&","&amp;");
    str.replace(">","&gt;");
    str.replace("<","&lt;");
    str.replace("\"","&quot;");
    str.replace("\'","&#39;");
    str.replace(" ","&nbsp;");
    str.replace("\n","<br>");
    str.replace("\r","<br>");
}

/********************************************************
  *类型：私有函数
  *功能：将替换特殊字符后的QString转换成对应颜色显示的html
  ****************************************************/
void mainform::stringToHtml(QString &str,QColor crl)
{
    QByteArray array;
    array.append(crl.red());
    array.append(crl.green());
    array.append(crl.blue());
    QString strC(array.toHex());
    str = QString("<span style=\" color:#%1;\">%2</span>").arg(strC).arg(str);
}
bool openCloseFlag = false;
void mainform::on_openMotorButton_clicked()
{
    ui->closeMotorButton->setEnabled(true);
    ui->openMotorButton->setEnabled(false);
    Leather_Data_Send(true,false,ui->verticalSlider->value(),0x01);
    openCloseFlag = true;
}

void mainform::on_closeMotorButton_clicked()
{
    ui->closeMotorButton->setEnabled(false);
    ui->openMotorButton->setEnabled(true);
    Leather_Data_Send(false,false,0,0x01);
    openCloseFlag = false;
}

void mainform::on_verticalSlider_sliderReleased()
{
    if(openCloseFlag == true)
    Leather_Data_Send(true,false,ui->verticalSlider->value(),0x01);
}

#define BYTE0(dwTemp)       ( *( (quint8 *)(&dwTemp)		) )
#define BYTE1(dwTemp)       ( *( (quint8 *)(&dwTemp) + 1) )//beautiful--Leather
void mainform::Leather_Data_Send(bool my_switch_1, bool my_switch_2, quint16 speed, quint8 flag)
{
    QByteArray array;
    unsigned char _cnt=0;
    unsigned char i=0;
    unsigned char sum = 0;

    array[_cnt++]=0xAA;
    array[_cnt++]=0xAA;
    array[_cnt++]=flag;
    array[_cnt++]=0;

    array[_cnt++]=my_switch_1;
    array[_cnt++]=my_switch_2;
    array[_cnt++]=BYTE1(speed);
    array[_cnt++]=BYTE0(speed);

    array[3] = _cnt-4;

     for(i=0;i<_cnt;i++)
        sum += array[i];

    array[_cnt++]=sum;

    m_Com->write(array);
}


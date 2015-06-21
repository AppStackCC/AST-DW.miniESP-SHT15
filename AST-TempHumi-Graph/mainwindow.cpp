#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QUdpSocket>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupRealtimeDataDemo(ui->graph);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initSocket(int port)
 {
     udpSocket = new QUdpSocket(this);
     udpSocket->bind(QHostAddress::Any, port);

     connect(udpSocket, SIGNAL(readyRead()),
             this, SLOT(readPendingDatagrams()));
 }

 void MainWindow::readPendingDatagrams()
 {
     QString ret_txt = "";
     while (udpSocket->hasPendingDatagrams())
     {
         QByteArray datagram;
         datagram.resize(udpSocket->pendingDatagramSize());
         QHostAddress sender;
         quint16 senderPort;

         udpSocket->readDatagram(datagram.data(), datagram.size(),
                                 &sender, &senderPort);

         ret_txt = process_data(datagram);
     }
 }


QString MainWindow::process_data(QByteArray data)
{
    st_float temp;
    st_float humi;
    QString humi_txt = "";
    QString temp_txt = "";

    uint8_t i = 0;

    for(i = 0; i < 4; i++)
    {
        temp.fdata[i] = data.at(i);
    }


    for(i = 0; i < 4; i++)
    {
        humi.fdata[i] = data.at(4 + i);
    }

    graph_data_update(ui->graph,temp.f1,humi.f1);
    temp_txt.setNum(temp.f1);
    humi_txt.setNum(humi.f1);


    ui->txt_output->append(temp_txt + " | " + humi_txt);

    return "";
}

void MainWindow::on_bt_start_server_clicked()
{

    if(SOCKET_STATE == 0)   //connect
    {
        int port = ui->txt_port->text().toInt();
        if(port > 0 && port < 65535)
        {
            initSocket(port);
            ui->bt_start_server->setText(_TXT_STOP_SERVER_);
            SOCKET_STATE = 1;
        }
        else
        {

        }
    }
    else
    {
        this->udpSocket->close();
        ui->bt_start_server->setText(_TXT_START_SERVER_);
        SOCKET_STATE = 0;
    }


}



void MainWindow::graph_data_update(QCustomPlot *customPlot,float temp,float humi)
{

    static double lastPointKey = 0;
    static  double key = 0;
    key++;

    customPlot->graph(_LINE_TEMP_)->addData(key, humi);
    customPlot->graph(_LINE_HUMI_)->addData(key, temp);

    // set data of dots:
    customPlot->graph(_DOT_TEMP_)->clearData();
    customPlot->graph(_DOT_TEMP_)->addData(key, humi);
    customPlot->graph(_DOT_HUMI_)->clearData();
    customPlot->graph(_DOT_HUMI_)->addData(key, temp);

    // remove data of lines that's outside visible range:
    customPlot->graph(_LINE_TEMP_)->removeDataBefore(key-100);
    customPlot->graph(_LINE_HUMI_)->removeDataBefore(key-100);

    // rescale value (vertical) axis to fit the current data:
    customPlot->graph(_LINE_TEMP_)->rescaleValueAxis(true);
    customPlot->graph(_LINE_HUMI_)->rescaleValueAxis(true);
    lastPointKey = key;

    customPlot->xAxis->setRange(key+0.25,80, Qt::AlignRight);
    customPlot->replot();

    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {

      lastFpsKey = key;
      frameCount = 0;
    }

}

void MainWindow::setupRealtimeDataDemo(QCustomPlot *customPlot)
{

    customPlot->addGraph(); // blue line
    customPlot->graph(_LINE_TEMP_)->setPen(QPen(Qt::blue));
    customPlot->graph(_LINE_TEMP_)->setAntialiasedFill(false);
    customPlot->addGraph(); // red line
    customPlot->graph(_LINE_HUMI_)->setPen(QPen(Qt::red));


    customPlot->addGraph(); // blue dot
    customPlot->graph(_DOT_TEMP_)->setPen(QPen(Qt::blue));
    customPlot->graph(_DOT_TEMP_)->setLineStyle(QCPGraph::lsNone);
    customPlot->graph(_DOT_TEMP_)->setScatterStyle(QCPScatterStyle::ssDisc);
    customPlot->addGraph(); // red dot
    customPlot->graph(_DOT_HUMI_)->setPen(QPen(Qt::red));
    customPlot->graph(_DOT_HUMI_)->setLineStyle(QCPGraph::lsNone);
    customPlot->graph(_DOT_HUMI_)->setScatterStyle(QCPScatterStyle::ssDisc);

    customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
    customPlot->xAxis->setDateTimeSpec(Qt::UTC);


    customPlot->xAxis->setAutoTickStep(true);
    customPlot->xAxis->setTickStep(1);


    customPlot->axisRect()->setupFullAxesBox();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

}

/*

  Copyright (c) 2015, 2016 Hubert Denkmair <hubert@denkmair.de>

  This file is part of cangaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "GraphWindow.h"
#include "ui_GraphWindow.h"

#include <QDomDocument>

#include <core/Backend.h>
#include <QtCharts/QChartView>
#include <core/CanTrace.h>
#include <core/CanDbMessage.h>
#include <QDebug>
#include <iostream>
#include <algorithm>
#include <QDateTime>
#define NUM_GRAPH_POINTS 20
void MyThread:: run()  {
    // 在这里执行耗时任务
    qDebug() << "线程任务执行中...";
    ((GraphWindow*)obj)->prepare_data();
    // 假设我们在这里进行复杂计算或耗时操作...

    // 完成后自动退出线程
}
GraphWindow::GraphWindow(QWidget *parent, Backend &backend) :
    ConfigurableWidget(parent),
    ui(new Ui::GraphWindow),
    _backend(backend)
{
    ui->setupUi(this);


    data_series = new my_lines[12];
    timer=new QTimer();
    timer->start(1000/ref_rate);
    timer->setSingleShot(true);
    // for(uint32_t i=0; i<NUM_GRAPH_POINTS; i++)
    // {
    //     data_series->append(i, i);
    // }

    data_chart = new QChart();
    data_chart->legend()->show();
    for(int i=0;i<12;i++)
    {
        //        data_series[i].setBorderColor(QColor(21, 100, 255));
        //        data_series[i].setMarkerSize(1);
        data_series[i].setName(QString::number(i+1));
        data_chart->addSeries(&data_series[i]);
        data_series[i].checkbox=new QCheckBox(QString::number(i+1));
    }
    data_chart->createDefaultAxes();
    data_chart->setTitle("Simple line chart example");
    for (int i=0;i<4;i++) {
        ui->check_layout->addWidget(data_series[i*3].checkbox,i,0);
        ui->check_layout->addWidget(data_series[i*3+1].checkbox,i,1);
        ui->check_layout->addWidget(data_series[i*3+2].checkbox,i,2);

    }


    // Have a box pop up that allows the user to select a signal from the loaded DBC to graph
    // On OK, add that CanDbMessage to a list.
    // Either sample the values regularly with a timer or somehow emit a signal when the message
    // is received that we catch here...

    //    backend.findDbMessage()

    //   CanDbMessage *result = 0;

    //   foreach (MeasurementNetwork *network, _networks) {
    //       foreach (pCanDb db, network->_canDbs) {
    //           result = db->getMessageById(msg.getRawId());
    //           if (result != 0) {
    //               return result;
    //           }
    //       }
    //   }
    //   return result;



    ui->chartView->setChart(data_chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);

    connect((QObject*)backend.getTrace(), SIGNAL(beforeAppend(int)), this, SLOT(beforeAppend(int)));
    connect((QObject*)backend.getTrace(), SIGNAL(beforeClear()), this, SLOT(beforeClear()));
    connect(ui->buttonTest, SIGNAL(released()), this, SLOT(testAddData()));
    connect(timer, SIGNAL(timeout()), this, SLOT(ref_plot()));
//    myThread = new MyThread();
//    myThread->obj=this;
//    myThread->start(); // 启动线程，将自动调用run方法

}
Backend* GraphWindow::backend() const
{
    return &_backend;
}
void GraphWindow::add_data(my_lines * line,qreal time,const CanMessage *msg)
{

    qint64 time_now=time;
    size_t size;

    const uint8_t* buf=msg->get_data_pin();
    float *data=(float*)buf;
    for (int i=0;i<2;i++) {
        if(!line[i].checkbox->isChecked())
        {
            if(line[i].count())
            {
                line[i].clear();
                line[i].data.clear();
                line[i].time_series.clear();
                line[i].maxs.clear();
                line[i].mins.clear();
                line[i].point_rest=0;
                line[i].current_sec=0;
            }
            continue;
        }
        int insertPos = line[i].data.size(); // 默认插入位置为列表末尾

        // 从后向前遍历列表
        for (int j = line[i].data.size() - 1; j >= 0; --j) {
            if (line[i].time_series[j] < time) { // 找到第一个小于新时间点的位置
                insertPos = j + 1; // 在这个位置之后插入
                break;
            }
        }
        line[i].data.insert(insertPos,data[i]);
        line[i].time_series.insert(insertPos,time);
        line[i].point_rest++;

    }
}
void GraphWindow::beforeAppend(int num_messages)
{
    //    static int count=0;
    //    if(++count<10)
    //        qDebug()<<"接到"<<num_messages<<"条数据";
    if(paused)
        return;
    CanTrace *trace = backend()->getTrace();
    int start_id = trace->size();

    static qint8 typenum=0;
    for (int i=start_id; i<start_id + num_messages; i++) {
        const CanMessage *msg = trace->getMessage(i);
        if(!starttime)
            starttime=msg->getFloatTimestamp();
        delta_time=((qreal)(msg->getFloatTimestamp()-starttime));
        //qDebug()<<delta_time;
        switch (msg->getId()) {
        case 0x20:
            add_data(&(data_series[0]),delta_time,msg);
            typenum++;
            break;
        case 0x21:add_data(&(data_series[2]),delta_time,msg);
            typenum++;
            break;
        case 0x22:add_data(&(data_series[4]),delta_time,msg);
            typenum++;
            break;
        case 0x23:add_data(&(data_series[6]),delta_time,msg);
            typenum++;
            break;
        case 0x24:add_data(&(data_series[8]),delta_time,msg);
            typenum++;
            break;
        case 0x25:add_data(&(data_series[10]),delta_time,msg);
            typenum++;
            break;
        }
    }


}

void GraphWindow::beforeClear()
{
    starttime=0;
    for (int i=0;i<12;i++) {
        data_series[i].clear();
        data_series[i].maxs.clear();
        data_series[i].mins.clear();
        data_series[i].current_sec=0;
        data_series[i].data.clear();
        data_series[i].time_series.clear();
        data_series[i].point_rest=0;

    }
    max_points=duration*1000/down_sample;
    qDebug()<<"duration:"<<duration<<"\tmax_points:"<<max_points;
}
void GraphWindow::testAddData()
{
    for (int i=0;i<12;i++) {
        if(data_series[i].points().size())
            qDebug()<<"point"<<i<<":"<<data_series[i].points().last();
    }
    //    QLineSeries* serbuf = new QLineSeries();

    //    // Start autorange at first point
    //    qreal ymin = data_series->at(1).y();
    //    qreal ymax = ymin;

    //    // Copy all points but first one
    //    for(uint32_t i=1; i < data_series->count(); i++)
    //    {
    //        serbuf->append(data_series->at(i).x()-1, data_series->at(i).y());

    //        // Autoranging
    //        if(data_series->at(i).y() < ymin)
    //            ymin = data_series->at(i).y();
    //        if(data_series->at(i).y() > ymax)
    //            ymax = data_series->at(i).y();
    //    }

    //    // Apply Y margin and set range
    //    ymin -= 1;
    //    ymax += 1;
    //    data_chart->axisY()->setRange(ymin, ymax);

    //    // Add new point in
    //    serbuf->append(serbuf->points().at(serbuf->count()-1).x() + 1, new_yval);
    //    testcount++;

    //    // Replace data
    //    data_series->replace(serbuf->points());

    //    delete serbuf;
}

GraphWindow::~GraphWindow()
{
    delete ui;
    delete data_chart;
    delete data_series;
}

bool GraphWindow::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    if (!ConfigurableWidget::saveXML(backend, xml, root)) { return false; }
    root.setAttribute("type", "GraphWindow");
    return true;
}

bool GraphWindow::loadXML(Backend &backend, QDomElement &el)
{
    if (!ConfigurableWidget::loadXML(backend, el)) { return false; }
    return true;
}

qreal temp_duration=10;
void GraphWindow::on_lineEdit_textEdited(const QString &arg1)
{
    qDebug()<<"duration:"<<arg1;
    if(arg1.toDouble()&&arg1.toDouble()>0)
        temp_duration=arg1.toDouble();
}

void GraphWindow::on_lineEdit_returnPressed()
{

    if(temp_duration>0)
        qDebug()<<"duration updateed!!!!";
    duration=temp_duration;
    if(duration>down_sample)
    {
        down_sample=duration+1;
    }
    beforeClear();
}
qreal temp_downs=0;
void GraphWindow::on_lineEdit_2_textChanged(const QString &arg1)
{
    qDebug()<<"downsample:"<<arg1;
    if(arg1.toDouble()&&arg1.toDouble()>0.5)
        temp_downs=arg1.toDouble();
}

void GraphWindow::on_lineEdit_2_returnPressed()
{

    if(temp_downs)
        down_sample=std::max((int)temp_downs,1);
    if(duration>down_sample)
    {
        duration=down_sample;
    }
    qDebug()<<"downsample updateed!!!!";
    beforeClear();
}



void GraphWindow::on_savedata_clicked()
{
    int temp_pause=paused;
    paused=1;
    // 创建一个文件对象
    QFile file("data.csv");

    // 打开文件，如果文件无法打开，输出错误信息并返回
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Could not open file for writing!";
        return;
    }
    int min_size=10000000;
    // 创建一个文本流，并将其和文件关联
    QTextStream stream(&file);

    // 写入数据到文件
    for (int i=1;i<12;i++) {
        if(data_series[i].checkbox->isChecked())
        {
            min_size=std::min(min_size,data_series[i].data.length());
            stream<<"时间序列"<<i<<','<<"数据"<<i<<',';
        }
    }
    stream<<'\n';

    for (int i = 0; i < min_size; ++i) {

        for (int j=0;j<12;j++) {
            if(data_series[j].checkbox->isChecked())
                stream<<data_series[j].time_series[i]<<','<<data_series[j].data[i]<<',';
        }
        stream<<'\n';
    }

    // 关闭文件
    file.close();
    paused=temp_pause;
}

void GraphWindow::on_hang_on_clicked()
{
    if(paused)
    {
        paused=0;
        ui->hang_on->setText("暂停");
    }
    else
    {
        paused=1;
        ui->hang_on->setText("继续");
    }
}
qreal temp_refrate=60;
void GraphWindow::on_lineEdit_3_textEdited(const QString &arg1)
{
    qDebug()<<"downsample:"<<arg1;
    if(arg1.toDouble()&&arg1.toDouble()>10)
        temp_refrate=arg1.toDouble();
}
void GraphWindow::on_lineEdit_3_returnPressed()
{
    qDebug()<<"downsample updated!!!!";
    if(temp_refrate)
        ref_rate=temp_refrate;
    timer->setInterval(ref_rate);

}
void GraphWindow::plot_resize()
{
    int first=1;
    for (int i=0;i<12;i++) {
        if(data_series[i].checkbox->isChecked())
        {
            if(first&&data_series[i].count())
            {
                data_series[i].min=*std::min_element(data_series[i].mins.begin(),data_series[i].mins.end());
                data_series[i].max=*std::max_element(data_series[i].maxs.begin(),data_series[i].maxs.end());
                first=0;
                ymin=data_series[i].min;
                ymax=data_series[i].max;
            }
            else if(data_series[i].count())
            {
                data_series[i].min=*std::min_element(data_series[i].mins.begin(),data_series[i].mins.end());
                data_series[i].max=*std::max_element(data_series[i].maxs.begin(),data_series[i].maxs.end());
                ymin=(ymin>data_series[i].min)?data_series[i].min:ymin;
                ymax=(ymax<data_series[i].max)?data_series[i].max:ymax;
            }
        }
    }
    if(!first)
    {
        ymargin=0.05*(ymax-ymin);
        ymargin=std::max(ymargin,0.001);

        //        ui->chartView->setChart(data_chart);
        //        ui->chartView->repaint();
    }
}
uint8_t data_need_ref=0;
void GraphWindow::prepare_data()
{
//    while(1)
//    {
//        QThread::msleep(10);
        for (int i=0;i<12;i++) {

            while (data_series[i].point_rest>0) {
                data_need_ref=1;
                int data_size=data_series[i].data.size();
                int data_p=data_size-data_series[i].point_rest;//当前处理的待显示点
                if(!data_series[i].checkbox->isChecked())
                {
                    break;
                }
                qreal time_now=data_series[i].time_series[data_p];
                qint64 sec_now=time_now*10/duration;
                qreal data_now=data_series[i].data[data_p];
                data_series[i].append(time_now,data_now);
                int size=data_series[i].maxs.size();
                if(sec_now==data_series[i].current_sec&&data_series[i].maxs.size())
                {
                    if(data_series[i].mins.last()>data_now)
                    {
                        data_series[i].mins.replace(data_series[i].mins.size()-1,data_now);
                    }
                    if(data_series[i].maxs.last()<data_now)
                    {
                        data_series[i].maxs.replace(data_series[i].maxs.size()-1,data_now);
                    }
                }else
                {
                    data_series[i].mins.append(data_now);
                    data_series[i].maxs.append(data_now);
                    size=data_series[i].maxs.size();
                    while(size>10)
                    {
                        data_series[i].maxs.takeFirst();
                        data_series[i].mins.takeFirst();
                        size=data_series[i].maxs.size();
                    }
                }
                int temp_count=data_series[i].count();
                if(temp_count>max_points)
                {
                    //            qDebug()<<*data_series[i].points().begin();
                    data_series[i].remove(0);
                    int to_erase=data_series[i].data.size()-duration*1000;
                    data_series[i].data.erase(data_series[i].data.begin(),std::next(data_series[i].data.begin(), to_erase-1));
                    data_series[i].time_series.erase(data_series[i].time_series.begin(),std::next(data_series[i].time_series.begin(), to_erase-1));
                }
                data_series[i].current_sec=sec_now;
                data_series[i].point_rest-=down_sample;
            }

        }
        if(data_need_ref)
        {
            plot_resize();
            static qint64 count;
            data_chart->axisY()->setRange(ymin-ymargin,ymax+ymargin);
            data_chart->axisX()->setRange(std::max(delta_time-duration,0.0),std::max(delta_time,1.0));
            //        qDebug()<<"刷新次数："<<count++<<"当前时间:"<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        }
//    }
}
void GraphWindow::ref_plot()
{

    prepare_data();
    timer->start(1);
}

qreal ymargin_temp=1;
void GraphWindow::on_lineEdit_4_textEdited(const QString &arg1)
{
    qDebug()<<"downsample:"<<arg1;
    if(arg1.toDouble()&&arg1.toDouble()>-0.1)
        ymargin_temp=arg1.toDouble();
}

void GraphWindow::on_lineEdit_4_returnPressed()
{
    qDebug()<<"downsample updateed!!!!";
    if(ymargin)
        ymargin=ymargin_temp;
}

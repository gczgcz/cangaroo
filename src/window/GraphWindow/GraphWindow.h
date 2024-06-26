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

#pragma once

#include <core/Backend.h>
#include <core/ConfigurableWidget.h>
#include <core/MeasurementSetup.h>
#include <QtCharts/QChartView>
#include <QtCharts/QtCharts>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QCheckBox>
#include <QTimer>
#include <QThread>
#include <QDebug>

class MyThread : public QThread {
    Q_OBJECT
public:
    // 构造函数，可根据需要添加参数
    MyThread(QObject *parent = nullptr) : QThread(parent) {}
    ConfigurableWidget *obj;
protected:
    // 重写run方法，执行线程任务
    void run() override;
};
class my_lines: public QLineSeries
{
public:
    qreal min=0,max=0;
    QCheckBox *checkbox;
    QList<qreal> mins,maxs;
    qint64 current_sec=0;
    qint32 down_count=0;
    QList<qreal>data;
    QList<qreal>time_series;
    qint32 point_rest=0;

};
namespace Ui {


class GraphWindow;
}

class QDomDocument;
class QDomElement;

class GraphWindow : public ConfigurableWidget
{
    Q_OBJECT

public:
    explicit GraphWindow(QWidget *parent, Backend &backend);
    ~GraphWindow();
    virtual bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);
    virtual bool loadXML(Backend &backend, QDomElement &el);
    Backend * backend() const;
    void add_data(my_lines * line,qreal time,const CanMessage *msg);
    void plot_resize();
    void prepare_data();
private slots:
    void beforeAppend(int num_messages);
    void beforeClear();
    void testAddData();


    void on_lineEdit_textEdited(const QString &arg1);

    void on_lineEdit_returnPressed();

    void on_lineEdit_2_textChanged(const QString &arg1);

    void on_lineEdit_2_returnPressed();


    void on_savedata_clicked();

    void on_hang_on_clicked();

    void on_lineEdit_3_textEdited(const QString &arg1);

    void on_lineEdit_3_returnPressed();
    void ref_plot();


    void on_lineEdit_4_textEdited(const QString &arg1);

    void on_lineEdit_4_returnPressed();

private:
    my_lines *data_series;
    QTimer *timer;
    QChart *data_chart;
    qreal ref_rate=60;
    qreal ymargin=1;
    uint32_t testcount;
    qreal starttime=0;
    qreal duration=10;
    qint32 down_sample=10;
    qint64 max_points=1000;
    bool paused=0;
    Ui::GraphWindow *ui;
    Backend &_backend;
    qreal ymin,ymax;
    qreal delta_time;
    MyThread *myThread;
};

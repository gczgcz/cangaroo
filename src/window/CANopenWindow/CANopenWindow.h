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

namespace Ui {
class CANopenWindow;
}

class QDomDocument;
class QDomElement;

class CANopenWindow : public ConfigurableWidget
{
    Q_OBJECT

public:
    explicit CANopenWindow(QWidget *parent, Backend &backend);
    ~CANopenWindow();

    virtual bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);
    virtual bool loadXML(Backend &backend, QDomElement &el);
    Backend * backend() const;

private slots:
    void changeDLC();
    void updateCapabilities();
    void changeRepeatRate(int ms);
    void sendRepeatMessage(bool enable);
    void disableTxWindow(int disable);
    void refreshInterfaces();
    void sendRawMessage();
    void repeatMessage();
    void beforeAppend(int num_messages);



    void on_PDOSendButton1_clicked();

    void on_PDOSendButton2_clicked();

    void on_PDOSendButton3_clicked();

    void on_PDOSendButton4_clicked();

    void on_SDOReadButton_clicked();

    void on_SDOWriteButton_clicked();

    void on_repeatWrite_clicked(bool checked);

    void on_repeatRead_clicked(bool checked);

private:
    Ui::CANopenWindow *ui;
    Backend &_backend;
    QTimer *repeatmsg_timer;
    uint8_t reading_flag=0,writtingflag=0,STD_SDO=0,repeat_status=0;
    QList<uint8_t> SDO_data;
    void hideFDFields();
    void showFDFields();
    void readCallback(const CanMessage *msg);
    void WriteCallback(const CanMessage * msg);
    void sendPDOMessage(int index);

};

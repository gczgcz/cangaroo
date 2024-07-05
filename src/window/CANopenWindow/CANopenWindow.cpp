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

#include "CANopenWindow.h"
#include "ui_CANopenWindow.h"

#include <QDomDocument>
#include <QTimer>
#include <core/Backend.h>
#include <driver/CanInterface.h>
#include "core/CanTrace.h"
#include <QDebug>


CANopenWindow::CANopenWindow(QWidget *parent, Backend &backend) :
    ConfigurableWidget(parent),
    ui(new Ui::CANopenWindow),
    _backend(backend)
{
    ui->setupUi(this);

    connect(ui->PDOSendAllButton, SIGNAL(released()), this, SLOT(sendRawMessage()));
    connect(ui->repeatSendButton, SIGNAL(toggled(bool)), this, SLOT(sendRepeatMessage(bool)));

    connect(ui->spinBox_RepeatRate, SIGNAL(valueChanged(int)), this, SLOT(changeRepeatRate(int)));

    connect(ui->comboBoxInterface, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCapabilities()));
    connect(ui->checkbox_FD, SIGNAL(stateChanged(int)), this, SLOT(updateCapabilities()));

    connect(&backend, SIGNAL(beginMeasurement()),  this, SLOT(refreshInterfaces()));

    // Timer for repeating messages
    repeatmsg_timer = new QTimer(this);
    connect(repeatmsg_timer, SIGNAL(timeout()), this, SLOT(repeatMessage()));
    connect((QObject*)backend.getTrace(), SIGNAL(beforeAppend(int)), this, SLOT(beforeAppend(int)));


    // TODO: Grey out checkboxes that are invalid depending on DLC spinbox state
    //connect(ui->fieldDLC, SIGNAL(valueChanged(int)), this, SLOT(changeDLC(int)));

    // Disable TX until interfaces are present
    this->setDisabled(1);

}


CANopenWindow::~CANopenWindow()
{
    delete ui;
}

Backend* CANopenWindow::backend() const
{
    return &_backend;
}

void CANopenWindow::changeDLC()
{

    ui->fieldByte0_0->setEnabled(true);
    ui->fieldByte1_0->setEnabled(true);
    ui->fieldByte2_0->setEnabled(true);
    ui->fieldByte3_0->setEnabled(true);
    ui->fieldByte4_0->setEnabled(true);
    ui->fieldByte5_0->setEnabled(true);
    ui->fieldByte6_0->setEnabled(true);
    ui->fieldByte7_0->setEnabled(true);


    ui->fieldByte0_2->setEnabled(true);
    ui->fieldByte1_2->setEnabled(true);
    ui->fieldByte2_2->setEnabled(true);
    ui->fieldByte3_2->setEnabled(true);
    ui->fieldByte4_2->setEnabled(true);
    ui->fieldByte5_2->setEnabled(true);
    ui->fieldByte6_2->setEnabled(true);
    ui->fieldByte7_2->setEnabled(true);


    ui->fieldByte0_4->setEnabled(true);
    ui->fieldByte1_4->setEnabled(true);
    ui->fieldByte2_4->setEnabled(true);
    ui->fieldByte3_4->setEnabled(true);
    ui->fieldByte4_4->setEnabled(true);
    ui->fieldByte5_4->setEnabled(true);
    ui->fieldByte6_4->setEnabled(true);
    ui->fieldByte7_4->setEnabled(true);


    ui->fieldByte0_6->setEnabled(true);
    ui->fieldByte1_6->setEnabled(true);
    ui->fieldByte2_6->setEnabled(true);
    ui->fieldByte3_6->setEnabled(true);
    ui->fieldByte4_6->setEnabled(true);
    ui->fieldByte5_6->setEnabled(true);
    ui->fieldByte6_6->setEnabled(true);
    ui->fieldByte7_6->setEnabled(true);


    //    repeatmsg_timer->setInterval(ms);
}

void CANopenWindow::updateCapabilities()
{

    // check if intf suports fd, if, enable, else dis
    //CanInterface *intf = _backend.getInterfaceById(idx);
    if(ui->comboBoxInterface->count() > 0)
    {
        // By default BRS should be available

        CanInterface *intf = _backend.getInterfaceById((CanInterfaceId)ui->comboBoxInterface->currentData().toUInt());

        if(intf == NULL)
        {
            return;
        }


        // If CANFD is available
        if(intf->getCapabilities() & intf->capability_canfd)
        {


            // Restore previous selected DLC if available
            //            if(idx_restore > 1 && idx_restore < ui->comboBoxDLC->count())
            //                ui->comboBoxDLC->setCurrentIndex(idx_restore);

            ui->checkbox_FD->setDisabled(0);

            // Enable BRS if this is an FD frame
            if(ui->checkbox_FD->isChecked())
            {
                // Enable BRS if FD enabled
                ui->checkbox_BRS->setDisabled(0);

                // Disable RTR if FD enabled
                ui->checkBox_IsRTR->setDisabled(1);
                ui->checkBox_IsRTR->setChecked(false);
            }
            else
            {
                // Disable BRS if FD disabled
                ui->checkbox_BRS->setDisabled(1);
                ui->checkbox_BRS->setChecked(false);

                // Enable RTR if FD disabled
                ui->checkBox_IsRTR->setDisabled(0);

            }
            showFDFields();
        }
        else
        {


            // Unset/disable FD / BRS checkboxes
            ui->checkbox_FD->setDisabled(1);
            ui->checkbox_BRS->setDisabled(1);
            ui->checkbox_FD->setChecked(false);
            ui->checkbox_BRS->setChecked(false);

            // Enable RTR (could be disabled by FD checkbox being set)
            ui->checkBox_IsRTR->setDisabled(0);

            hideFDFields();

        }
    }
}

void CANopenWindow::changeRepeatRate(int ms)
{
    repeatmsg_timer->setInterval(ms);
}


void CANopenWindow::sendRepeatMessage(bool enable)
{
    if(enable)
    {
        repeatmsg_timer->start(ui->spinBox_RepeatRate->value());
        repeat_status|=1;
    }
    else
    {
        repeat_status&=0xfe;
    }
}
void CANopenWindow::on_repeatWrite_clicked(bool checked)
{
    if(checked)
    {
        repeatmsg_timer->start(ui->spinBox_RepeatRate->value());
        repeat_status|=2;
    }
    else
    {
        repeat_status&=0xfd;
    }
}
void CANopenWindow::on_repeatRead_clicked(bool checked)
{
    if(checked)
    {
        repeatmsg_timer->start(ui->spinBox_RepeatRate->value());
        repeat_status|=4;
    }
    else
    {
        repeat_status&=0xfb;
    }
}
void CANopenWindow:: repeatMessage()
{
    if(repeat_status&0x01)
        sendRawMessage();
    if(repeat_status&0x02)
        on_SDOWriteButton_clicked();
    if(repeat_status&0x04)
        on_SDOReadButton_clicked();
}

void CANopenWindow::disableTxWindow(int disable)
{
    if(disable)
    {
        this->setDisabled(1);
    }
    else
    {
        // Only enable if an interface is present
        if(_backend.getInterfaceList().count() > 0)
            this->setDisabled(0);
        else
            this->setDisabled(1);
    }
}

void CANopenWindow::refreshInterfaces()
{
    ui->comboBoxInterface->clear();

    int cb_idx = 0;

    // TODO: Only show interfaces that are included in active MeasurementInterfaces!
    foreach (CanInterfaceId ifid, _backend.getInterfaceList()) {
        CanInterface *intf = _backend.getInterfaceById(ifid);

        if(intf->isOpen())
        {
            ui->comboBoxInterface->addItem(intf->getName() + " " + intf->getDriver()->getName());
            ui->comboBoxInterface->setItemData(cb_idx, QVariant(ifid));
            cb_idx++;
        }
    }

    if(cb_idx == 0)
        disableTxWindow(1);
    else
        disableTxWindow(0);

    updateCapabilities();
}
void CANopenWindow::sendPDOMessage(int i)
{
    CanMessage msg;

    bool en_extended = ui->checkBox_IsExtended->isChecked();
    bool en_rtr = ui->checkBox_IsRTR->isChecked();

    uint8_t data_int[64];
    int data_ctr = 0;

    data_int[data_ctr++] = ui->fieldByte0_0->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte1_0->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte2_0->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte3_0->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte4_0->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte5_0->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte6_0->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte7_0->text().toUpper().toInt(NULL, 16);


    data_int[data_ctr++] = ui->fieldByte0_2->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte1_2->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte2_2->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte3_2->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte4_2->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte5_2->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte6_2->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte7_2->text().toUpper().toInt(NULL, 16);


    data_int[data_ctr++] = ui->fieldByte0_4->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte1_4->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte2_4->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte3_4->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte4_4->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte5_4->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte6_4->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte7_4->text().toUpper().toInt(NULL, 16);


    data_int[data_ctr++] = ui->fieldByte0_6->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte1_6->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte2_6->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte3_6->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte4_6->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte5_6->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte6_6->text().toUpper().toInt(NULL, 16);
    data_int[data_ctr++] = ui->fieldByte7_6->text().toUpper().toInt(NULL, 16);



    uint32_t address = ui->NodeID_input->text().toUpper().toUInt(NULL, 16);


    msg.setLength(8);

    msg.setExtended(en_extended);
    msg.setRTR(en_rtr);
    msg.setErrorFrame(false);
    if(ui->checkbox_BRS->isChecked())
        msg.setBRS(true);
    if(ui->checkbox_FD->isChecked())
        msg.setFD(true);
    CanInterface *intf = _backend.getInterfaceById((CanInterfaceId)ui->comboBoxInterface->currentData().toUInt());
    msg.setId(0x200+0x100*i+address);
    for (int j=0;j<8;j++) {
        msg.setDataAt(j, data_int[i*8+j]);

    }

    intf->sendMessage(msg);


    char outmsg[256];
    snprintf(outmsg, 256, "Send [%s] to %d on port %s [ext=%u rtr=%u err=%u fd=%u brs=%u]",
             msg.getDataHexString().toLocal8Bit().constData(), msg.getId(), intf->getName().toLocal8Bit().constData(),
             msg.isExtended(), msg.isRTR(), msg.isErrorFrame(), msg.isFD(), msg.isBRS());
    log_info(outmsg);
}

void CANopenWindow::sendRawMessage()
{
    for (int i=0;i<4;i++) {
        sendPDOMessage(i);

    }

}
void CANopenWindow::on_PDOSendButton1_clicked()
{
    sendPDOMessage(0);
}
void CANopenWindow::on_PDOSendButton2_clicked()
{

    sendPDOMessage(1);
}

void CANopenWindow::on_PDOSendButton3_clicked()
{
    sendPDOMessage(2);
}

void CANopenWindow::on_PDOSendButton4_clicked()
{
    sendPDOMessage(3);
}
bool CANopenWindow::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    if (!ConfigurableWidget::saveXML(backend, xml, root)) { return false; }
    root.setAttribute("type", "CANopenWindow");
    return true;
}

bool CANopenWindow::loadXML(Backend &backend, QDomElement &el)
{
    if (!ConfigurableWidget::loadXML(backend, el)) { return false; }
    return true;
}

void CANopenWindow::hideFDFields()
{


}


void CANopenWindow::showFDFields()
{


}
void CANopenWindow::WriteCallback(const CanMessage *msg)
{
    if(msg->getByte(0)==0x60)
        writtingflag=0;
}
void CANopenWindow::readCallback(const CanMessage *msg)
{
    qDebug()<<"读取成功！";
    if((msg->getByte(0)&0xf0)==0x40)
    {
        int datalen=4-(msg->getByte(0)&0xf)/4;
        for (int i=0;i<datalen;i++)
        {
            ui->SDO_read_box->text().append( msg->getByte(4+i));
        }
    }
}

void CANopenWindow:: beforeAppend(int num_messages)
{

    CanTrace *trace = _backend.getTrace();
    int start_id = trace->size();
    uint32_t address = ui->NodeID_input->text().toUpper().toUInt(NULL, 16);
    uint32_t SDO_id=0x600+address;
    static qint8 typenum=0;
    for (int i=start_id; i<start_id + num_messages; i++) {
        const CanMessage *msg = trace->getMessage(i);
        if(msg->getId()==SDO_id)
        {
            switch (msg->getByte(0)) {
            case 0x60:
            case 0x20:
            case 0x30:
                qDebug()<<"写入成功！";
                WriteCallback(msg);
                break;
            case 0x00:
            case 0x10:
                readCallback(msg);
            default:
                if((msg->getByte(0)&0xf0)==0x40)
                {
                    if((msg->getByte(0)&3)==3)
                        readCallback(msg);
                }
                else if ((msg->getByte(0)&0xe0)==0x00)
                {
                    if((msg->getByte(0)&1)==1)
                        readCallback(msg);
                }
            }
        }
    }
}



void CANopenWindow::on_SDOReadButton_clicked()
{
    CanMessage msg;
    uint32_t address = ui->NodeID_input->text().toUpper().toUInt(NULL, 16);
    bool en_extended = ui->checkBox_IsExtended->isChecked();
    bool en_rtr = ui->checkBox_IsRTR->isChecked();


    msg.setLength(8);

    msg.setExtended(en_extended);
    msg.setRTR(en_rtr);
    msg.setErrorFrame(false);
    if(ui->checkbox_BRS->isChecked())
        msg.setBRS(true);
    if(ui->checkbox_FD->isChecked())
        msg.setFD(true);
    CanInterface *intf = _backend.getInterfaceById((CanInterfaceId)ui->comboBoxInterface->currentData().toUInt());
    msg.setId(0x580+address);
    uint16_t od_index=ui->index_input->text().toUpper().toUInt(NULL,16);
    uint8_t sub_index=ui->subIndex->text().toUpper().toUInt(NULL,16);
    msg.setDataAt(0,0x40);
    msg.setDataAt(1,od_index);
    msg.setDataAt(2,od_index>>8);
    msg.setDataAt(3,sub_index);

    for (int j=4;j<8;j++) {
        msg.setDataAt(j, 0);

    }

    intf->sendMessage(msg);


    char outmsg[256];
    snprintf(outmsg, 256, "Send [%s] to %d on port %s [ext=%u rtr=%u err=%u fd=%u brs=%u]",
             msg.getDataHexString().toLocal8Bit().constData(), msg.getId(), intf->getName().toLocal8Bit().constData(),
             msg.isExtended(), msg.isRTR(), msg.isErrorFrame(), msg.isFD(), msg.isBRS());
    log_info(outmsg);
}

void CANopenWindow::on_SDOWriteButton_clicked()
{
    CanMessage msg;
    uint32_t address = ui->NodeID_input->text().toUpper().toUInt(NULL, 16);
    bool en_extended = ui->checkBox_IsExtended->isChecked();
    bool en_rtr = ui->checkBox_IsRTR->isChecked();


    msg.setLength(8);

    msg.setExtended(en_extended);
    msg.setRTR(en_rtr);
    msg.setErrorFrame(false);
    if(ui->checkbox_BRS->isChecked())
        msg.setBRS(true);
    if(ui->checkbox_FD->isChecked())
        msg.setFD(true);
    CanInterface *intf = _backend.getInterfaceById((CanInterfaceId)ui->comboBoxInterface->currentData().toUInt());
    msg.setId(0x580+address);
    uint16_t od_index=ui->index_input->text().toUpper().toUInt(NULL,16);
    uint8_t sub_index=ui->subIndex->text().toUpper().toUInt(NULL,16);
    int datalen=ui->SDO_Length->text().toUpper().toUInt(NULL,16);
    msg.setDataAt(0,0x33-datalen*4);
    msg.setDataAt(1,od_index);
    msg.setDataAt(2,od_index>>8);
    msg.setDataAt(3,sub_index);
    QStringList inputlist=ui->SDO_write_box->text().split(' ');
    if(datalen<=4)
    {
    for (int j=4;j<4+datalen;j++) {
        msg.setDataAt(j, inputlist[j-4].toUInt(NULL,16));

    }
    for(int j=4+datalen;j<8;j++)
        msg.setDataAt(j, 0);
    }

    intf->sendMessage(msg);


    char outmsg[256];
    snprintf(outmsg, 256, "Send [%s] to %d on port %s [ext=%u rtr=%u err=%u fd=%u brs=%u]",
             msg.getDataHexString().toLocal8Bit().constData(), msg.getId(), intf->getName().toLocal8Bit().constData(),
             msg.isExtended(), msg.isRTR(), msg.isErrorFrame(), msg.isFD(), msg.isBRS());
    log_info(outmsg);
}









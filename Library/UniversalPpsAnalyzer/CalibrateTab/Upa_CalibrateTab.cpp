//*****************************************************************************************
// Project:     UniversalConfigurationManager
//
// Author:      Sven Meier, NetTimeLogic GmbH
//
// License:     Copyright (c) 2022, NetTimeLogic GmbH, Switzerland, Sven Meier <contact@nettimelogic.com>
//              All rights reserved.
//
//              THIS PROGRAM IS FREE SOFTWARE: YOU CAN REDISTRIBUTE IT AND/OR MODIFY
//              IT UNDER THE TERMS OF THE GNU LESSER GENERAL PUBLIC LICENSE AS
//              PUBLISHED BY THE FREE SOFTWARE FOUNDATION, VERSION 3.
//
//              THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT
//              WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//              MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. SEE THE GNU
//              LESSER GENERAL LESSER PUBLIC LICENSE FOR MORE DETAILS.
//
//              YOU SHOULD HAVE RECEIVED A COPY OF THE GNU LESSER GENERAL PUBLIC LICENSE
//              ALONG WITH THIS PROGRAM. IF NOT, SEE <http://www.gnu.org/licenses/>.
//
//*****************************************************************************************

#include <QStandardPaths>
#include <QProgressDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QThread>
#include <QElapsedTimer>

#include <Upa_CalibrateTab.h>
#include <ui_Upa_CalibrateTab.h>

Upa_CalibrateTab::Upa_CalibrateTab(Upa_UniversalPpsAnalyzer *parent) : QWidget()
{
    upa = parent;

    ui = new Ui::Upa_CalibrateTab();
    ui->setupUi(this);

    calibrate_timer = new QTimer(this);
    calibrate_timer->stop();

    for (int i = 0; i < upa->ts_core_config.size(); i++)
    {
        if (0 > ui->PpsAnalyzerComboBox->findText(upa->ts_core_config.at(i).com_port))
        {
            ui->PpsAnalyzerComboBox->addItem(upa->ts_core_config.at(i).com_port);
        }
    }

    connect(ui->CalibrateReadValuesButton, SIGNAL(clicked()), this, SLOT(calibrate_read_values_button_clicked()));
    connect(ui->CalibrateWriteValuesButton, SIGNAL(clicked()), this, SLOT(calibrate_write_values_button_clicked()));
    connect(ui->CalibrateReadMacIpValuesButton, SIGNAL(clicked()), this, SLOT(calibrate_read_mac_ip_values_button_clicked()));
    connect(ui->CalibrateWriteMacIpValuesButton, SIGNAL(clicked()), this, SLOT(calibrate_write_mac_ip_values_button_clicked()));
    connect(ui->CalibrateCalibrateButton, SIGNAL(clicked()), this, SLOT(calibrate_calibrate_button_clicked()));
    connect(calibrate_timer, SIGNAL(timeout()), this, SLOT(calibrate_read_values_timer()));
}

Upa_CalibrateTab::~Upa_CalibrateTab()
{
    calibrate_timer->stop();

    delete ui;
    delete calibrate_timer;
}

int Upa_CalibrateTab::calibrate_resize(int height, int width)
{
    int height_delta = 0;
    int width_delta = 0;

    height_delta = (height-Upa_MainHeight);
    width_delta = (width-Upa_MainWidth);

    updateGeometry();

    return 0;
}

int Upa_CalibrateTab::calibrate_enable(void)
{
    calibrate_timer->start(1000);
    return 0;
}

int Upa_CalibrateTab::calibrate_disable(void)
{
    calibrate_timer->stop();
    return 0;
}

int Upa_CalibrateTab::calibrate_eeprom_read_reg(const Upa_CoreConfig* eeprom, const unsigned int addr, unsigned int& data)
{
    unsigned int temp_data = 0;
    unsigned int temp_addr = 0;

    temp_addr = eeprom->address_range_low;

    // i2c addr
    temp_data = Upa_I2cConf_EepromAddr; // this is the address
    if (0 == eeprom->com_lib->write_reg(temp_addr + Upa_I2cConf_ChipAddrReg, temp_data))
    {
        // reg addr
        temp_data = addr;
        if (0 == eeprom->com_lib->write_reg(temp_addr + Upa_I2cConf_RegAddrReg, temp_data))
        {
            // control read and wait
            temp_data = 0x00000002; // read
            if (0 == eeprom->com_lib->write_reg(temp_addr + Upa_I2cConf_ControlReg, temp_data))
            {
                for (int i = 0; i < 10000; i++) // give it enough time
                {
                    if (0 == eeprom->com_lib->read_reg(temp_addr + Upa_I2cConf_ControlReg, temp_data))
                    {
                        if ((temp_data & 0x20000002) == 0x20000000)
                        {
                            // read done
                            if ((temp_data & 0x40000000) == 0x40000000)
                            {
                                cout << "ERROR: " << "read error" << endl;
                                temp_data = 0x00000000; // read clear
                                eeprom->com_lib->write_reg(temp_addr + Upa_I2cConf_ControlReg, temp_data);
                                return -1;
                            }
                            else if (0 == eeprom->com_lib->read_reg(temp_addr + Upa_I2cConf_DataReg, temp_data)) // read data
                            {
                                data = temp_data;
                            }
                            else
                            {
                                return -1;
                            }
                            // read done
                            break;
                        }
                        else if (i == 9999)// timed out
                        {
                            cout << "ERROR: " << "read did not complete" << endl;
                            temp_data = 0x00000000; // read clear
                            eeprom->com_lib->write_reg(temp_addr + Upa_I2cConf_ControlReg, temp_data);
                            return -1;
                        }
                    }
                    else
                    {
                        return -1;
                    }
                }
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    return 0;
}

int Upa_CalibrateTab::calibrate_eeprom_write_reg(const Upa_CoreConfig* eeprom, const unsigned int addr, unsigned int& data)
{
    unsigned int temp_data = 0;
    unsigned int temp_addr = 0;

    temp_addr = eeprom->address_range_low;

    // i2c addr
    temp_data = Upa_I2cConf_EepromAddr; // this is the address
    if (0 == eeprom->com_lib->write_reg(temp_addr + Upa_I2cConf_ChipAddrReg, temp_data))
    {
        // reg addr
        temp_data = addr;
        if (0 == eeprom->com_lib->write_reg(temp_addr + Upa_I2cConf_RegAddrReg, temp_data))
        {
            // write data
            temp_data = data;
            if (0 == eeprom->com_lib->write_reg(temp_addr + Upa_I2cConf_DataReg, temp_data))
            {
                // control write and wait
                temp_data = 0x00000001; // write
                if (0 == eeprom->com_lib->write_reg(temp_addr + Upa_I2cConf_ControlReg, temp_data))
                {
                    for (int i = 0; i < 10000; i++) // give it enough time
                    {
                        if(0 == eeprom->com_lib->read_reg(temp_addr + Upa_I2cConf_ControlReg, temp_data))
                        {
                            if ((temp_data & 0x10000001) == 0x10000000)
                            {
                                // write done
                                if ((temp_data & 0x40000000) == 0x40000000)
                                {
                                    cout << "ERROR: " << "write error" << endl;
                                    temp_data = 0x00000000; // write clear
                                    eeprom->com_lib->write_reg(temp_addr + Upa_I2cConf_ControlReg, temp_data);
                                    return -1;
                                }
                                break;
                            }
                            else if (i == 9999) // timed out
                            {
                                cout << "ERROR: " << "write did not complete" << endl;
                                temp_data = 0x00000000; // write clear
                                eeprom->com_lib->write_reg(temp_addr + Upa_I2cConf_ControlReg, temp_data);
                                return -1;
                            }
                        }
                        else
                        {
                            return -1;
                        }
                    }
                }
                else
                {
                    return -1;
                }
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    return 0;
}

void Upa_CalibrateTab::calibrate_read_ts_values(void)
{
    unsigned int temp_nanoseconds = 0;
    long long temp_signed_offset;
    long long temp_ref_offset = 0;
    unsigned int temp_data = 0;
    unsigned int temp_addr = 0;

    for (int k = 0; k < upa->ts_core_config.size(); k++)
    {
        if (ui->PpsAnalyzerComboBox->currentText() == upa->ts_core_config.at(k).com_port)
        {
            temp_addr =  upa->ts_core_config.at(k).address_range_low;

            //********************************
            // get timestamp
            //********************************
            //if (0 == upa->ts_core_config.at(k).com_lib->read_reg(temp_addr + Upa_ClkTs_IrqReg, temp_data))
            //{
                //if ((temp_data & 0x00000001) != 0)
                //{
            if (0 == upa->ts_core_config.at(k).com_lib->read_reg(temp_addr + Upa_ClkTs_StatusReg, temp_data)) // check if the PPS has activity at all (within the last 1.5s)
            {
                if ((temp_data & 0x00000002) != 0)
                {
                    // nanoseconds
                    if (0 == upa->ts_core_config.at(k).com_lib->read_reg(temp_addr + Upa_ClkTs_TimeValueLReg, temp_data))
                    {
                        temp_nanoseconds = temp_data;
                    }

                    // clear irq
                    //temp_data = 0x00000001;
                    //if (0 == upa->ts_core_config.at(k).com_lib->write_reg(temp_addr + Upa_ClkTs_IrqReg, temp_data))
                    //{
                    //    // nothing irq cleared
                    //}
                    //else
                    //{
                    //    cout << "ERROR: " << "irq cleaning didn't work" << endl;
                    //}

                    if(temp_nanoseconds > 500000000)
                    {
                        temp_signed_offset = (long long)temp_nanoseconds - 1000000000;
                    }
                    else
                    {
                        temp_signed_offset = (long long)temp_nanoseconds;
                    }
                    // compensate
                    temp_signed_offset -= temp_ref_offset;

                }
                else
                {
                    temp_signed_offset = 0;
                }

                switch (upa->ts_core_config.at(k).core_instance_nr)
                {
                    case 1:
                        temp_ref_offset = temp_signed_offset;
                        ui->PpsOffsetPpsRefInValue->setText(QString::number(temp_signed_offset));
                        break;
                    case 2:
                        ui->PpsOffsetPps1Value->setText(QString::number(temp_signed_offset));
                        break;
                    case 3:
                        ui->PpsOffsetPps2Value->setText(QString::number(temp_signed_offset));
                        break;
                    case 4:
                        ui->PpsOffsetPps3Value->setText(QString::number(temp_signed_offset));
                        break;
                    case 5:
                        ui->PpsOffsetPps4Value->setText(QString::number(temp_signed_offset));
                        break;
                    case 6:
                        ui->PpsOffsetPps5Value->setText(QString::number(temp_signed_offset));
                        break;
                    case 7:
                        ui->PpsOffsetPps6Value->setText(QString::number(temp_signed_offset));
                        break;
                    case 8:
                        ui->PpsOffsetPps7Value->setText(QString::number(temp_signed_offset));
                        break;
                    case 9:
                        ui->PpsOffsetPps8Value->setText(QString::number(temp_signed_offset));
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void Upa_CalibrateTab::calibrate_read_values(void)
{
    unsigned int temp_date = 0;
    unsigned int temp_serial_number = 0;
    unsigned int temp_nanoseconds = 0;
    unsigned int temp_data = 0;
    unsigned int temp_addr = 0;

    for (int k = 0; k < upa->i2c_core_config.size(); k++)
    {
        if (ui->PpsAnalyzerComboBox->currentText() == upa->i2c_core_config.at(k).com_port)
        {

            // Calibrated
            temp_addr = 0;
            if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
            {
                if (temp_data == 0x50)
                {
                    temp_addr = 1;
                    if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        if (temp_data == 0x50)
                        {
                            temp_addr = 2;
                            if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                            {
                                if (temp_data == 0x53)
                                {
                                    temp_addr = 3;
                                    if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                                    {
                                        if (temp_data == 0x41)
                                        {
                                            temp_addr = 12;
                                            if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                                            {
                                                if ((temp_data & 0xFF) == 0x00)
                                                {
                                                    ui->CalibratedCheckBox->setChecked(true);
                                                }
                                                else if ((temp_data & 0xFF) == 0x01)
                                                {
                                                    ui->CalibratedCheckBox->setChecked(false);
                                                }
                                                else
                                                {
                                                    ui->CalibratedCheckBox->setChecked(false);
                                                    calibrate_write_values(true);
                                                    calibrate_read_values();
                                                    return;
                                                }
                                            }
                                            else
                                            {
                                                // eeprom acces not possible?
                                                ui->CalibratedCheckBox->setChecked(false);
                                                return;
                                            }
                                        }
                                        else
                                        {
                                            ui->CalibratedCheckBox->setChecked(false);
                                            calibrate_write_values(true);
                                            calibrate_read_values();
                                            return;
                                        }
                                    }
                                    else
                                    {
                                        // eeprom acces not possible?
                                        ui->CalibratedCheckBox->setChecked(false);
                                        return;
                                    }
                                }
                                else
                                {
                                    ui->CalibratedCheckBox->setChecked(false);
                                    calibrate_write_values(true);
                                    calibrate_read_values();
                                    return;
                                }
                            }
                            else
                            {
                                // eeprom acces not possible?
                                ui->CalibratedCheckBox->setChecked(false);
                                return;
                            }
                        }
                        else
                        {
                            ui->CalibratedCheckBox->setChecked(false);
                            calibrate_write_values(true);
                            calibrate_read_values();
                            return;
                        }
                    }
                    else
                    {
                        // eeprom acces not possible?
                        ui->CalibratedCheckBox->setChecked(false);
                        return;
                    }
                }
                else
                {
                    ui->CalibratedCheckBox->setChecked(false);
                    calibrate_write_values(true);
                    calibrate_read_values();
                    return;
                }
            }
            else
            {
                // eeprom acces not possible?
                ui->CalibratedCheckBox->setChecked(false);
                return;
            }

            //Serial Number
            temp_addr = 4;
            if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
            {
                temp_serial_number = temp_data;
                temp_addr = 5;
                if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_serial_number = temp_serial_number << 8;
                    temp_serial_number += temp_data;
                    temp_addr = 6;
                    if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        temp_serial_number = temp_serial_number << 8;
                        temp_serial_number += temp_data;
                        temp_addr = 7;
                        if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                        {
                            temp_serial_number = temp_serial_number << 8;
                            temp_serial_number += temp_data;
                            ui->SerialNumberValue->setText(QString::number(temp_serial_number));
                        }
                        else
                        {
                            ui->SerialNumberValue->setText("NA");
                        }
                    }
                    else
                    {
                        ui->SerialNumberValue->setText("NA");
                    }
                }
                else
                {
                    ui->SerialNumberValue->setText("NA");
                }
            }
            else
            {
                ui->SerialNumberValue->setText("NA");
            }

            //Calibration Date
            if (true == ui->CalibratedCheckBox->isChecked())
            {
                temp_addr = 8;
                if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_date = temp_data;
                    temp_addr = 9;
                    if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        temp_date = temp_date << 8;
                        temp_date += temp_data;
                        temp_addr = 10;
                        if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                        {
                            temp_date = temp_date << 8;
                            temp_date += temp_data;
                            temp_addr = 11;
                            if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                            {
                                temp_date = temp_date << 8;
                                temp_date += temp_data;

                                // date
                                QDateTime temp_date_formated;
                                temp_date_formated.setSecsSinceEpoch(temp_date);
                                ui->CalibrationDateValue->setText(temp_date_formated.toUTC().toString("dd.MM.yyyy hh:mm:ss"));
                            }
                            else
                            {
                                ui->CalibrationDateValue->setText("NA");
                            }
                        }
                        else
                        {
                            ui->CalibrationDateValue->setText("NA");
                        }
                    }
                    else
                    {
                        ui->CalibrationDateValue->setText("NA");
                    }
                }
                else
                {
                    ui->CalibrationDateValue->setText("NA");
                }
            }
            else
            {
                ui->CalibrationDateValue->setText("NA");
            }

            //PPS Ref Out
            temp_addr = 64;
            if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
            {
                temp_nanoseconds = temp_data;
                temp_addr = 65;
                if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_nanoseconds = temp_nanoseconds << 8;
                    temp_nanoseconds += temp_data;
                    ui->PpsCalibrationPpsRefOutValue->setText(QString::number(temp_nanoseconds));
                }
                else
                {
                    ui->PpsCalibrationPpsRefOutValue->setText("NA");
                }
            }
            else
            {
                ui->PpsCalibrationPpsRefOutValue->setText("NA");
            }

            //PPS Ref In
            temp_addr = 66;
            if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
            {
                temp_nanoseconds = temp_data;
                temp_addr = 67;
                if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_nanoseconds = temp_nanoseconds << 8;
                    temp_nanoseconds += temp_data;
                    ui->PpsCalibrationPpsRefInValue->setText(QString::number(temp_nanoseconds));
                }
                else
                {
                    ui->PpsCalibrationPpsRefInValue->setText("NA");
                }
            }
            else
            {
                ui->PpsCalibrationPpsRefInValue->setText("NA");
            }

            //PPS 1
            temp_addr = 68;
            if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
            {
                temp_nanoseconds = temp_data;
                temp_addr = 69;
                if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_nanoseconds = temp_nanoseconds << 8;
                    temp_nanoseconds += temp_data;
                    ui->PpsCalibrationPps1Value->setText(QString::number(temp_nanoseconds));
                }
                else
                {
                    ui->PpsCalibrationPps1Value->setText("NA");
                }
            }
            else
            {
                ui->PpsCalibrationPps1Value->setText("NA");
            }

            //PPS 2
            temp_addr = 70;
            if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
            {
                temp_nanoseconds = temp_data;
                temp_addr = 71;
                if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_nanoseconds = temp_nanoseconds << 8;
                    temp_nanoseconds += temp_data;
                    ui->PpsCalibrationPps2Value->setText(QString::number(temp_nanoseconds));
                }
                else
                {
                    ui->PpsCalibrationPps2Value->setText("NA");
                }
            }
            else
            {
                ui->PpsCalibrationPps2Value->setText("NA");
            }

            //PPS 3
            temp_addr = 72;
            if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
            {
                temp_nanoseconds = temp_data;
                temp_addr = 73;
                if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_nanoseconds = temp_nanoseconds << 8;
                    temp_nanoseconds += temp_data;
                    ui->PpsCalibrationPps3Value->setText(QString::number(temp_nanoseconds));
                }
                else
                {
                    ui->PpsCalibrationPps3Value->setText("NA");
                }
            }
            else
            {
                ui->PpsCalibrationPps3Value->setText("NA");
            }

            //PPS 4
            temp_addr = 74;
            if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
            {
                temp_nanoseconds = temp_data;
                temp_addr = 75;
                if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_nanoseconds = temp_nanoseconds << 8;
                    temp_nanoseconds += temp_data;
                    ui->PpsCalibrationPps4Value->setText(QString::number(temp_nanoseconds));
                }
                else
                {
                    ui->PpsCalibrationPps4Value->setText("NA");
                }
            }
            else
            {
                ui->PpsCalibrationPps4Value->setText("NA");
            }

            //PPS 5
            temp_addr = 76;
            if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
            {
                temp_nanoseconds = temp_data;
                temp_addr = 77;
                if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_nanoseconds = temp_nanoseconds << 8;
                    temp_nanoseconds += temp_data;
                    ui->PpsCalibrationPps5Value->setText(QString::number(temp_nanoseconds));
                }
                else
                {
                    ui->PpsCalibrationPps5Value->setText("NA");
                }
            }
            else
            {
                ui->PpsCalibrationPps5Value->setText("NA");
            }

            //PPS 6
            temp_addr = 78;
            if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
            {
                temp_nanoseconds = temp_data;
                temp_addr = 79;
                if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_nanoseconds = temp_nanoseconds << 8;
                    temp_nanoseconds += temp_data;
                    ui->PpsCalibrationPps6Value->setText(QString::number(temp_nanoseconds));
                }
                else
                {
                    ui->PpsCalibrationPps6Value->setText("NA");
                }
            }
            else
            {
                ui->PpsCalibrationPps6Value->setText("NA");
            }

            //PPS 7
            temp_addr = 80;
            if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
            {
                temp_nanoseconds = temp_data;
                temp_addr = 81;
                if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_nanoseconds = temp_nanoseconds << 8;
                    temp_nanoseconds += temp_data;
                    ui->PpsCalibrationPps7Value->setText(QString::number(temp_nanoseconds));
                }
                else
                {
                    ui->PpsCalibrationPps7Value->setText("NA");
                }
            }
            else
            {
                ui->PpsCalibrationPps7Value->setText("NA");
            }

            //PPS 8
            temp_addr = 82;
            if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
            {
                temp_nanoseconds = temp_data;
                temp_addr = 83;
                if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_nanoseconds = temp_nanoseconds << 8;
                    temp_nanoseconds += temp_data;
                    ui->PpsCalibrationPps8Value->setText(QString::number(temp_nanoseconds));
                }
                else
                {
                    ui->PpsCalibrationPps8Value->setText("NA");
                }
            }
            else
            {
                ui->PpsCalibrationPps8Value->setText("NA");
            }

        }

    }
}
void Upa_CalibrateTab::calibrate_write_values(bool clear)
{
    unsigned int temp_date = 0;
    unsigned int temp_serial_number = 0;
    unsigned int temp_nanoseconds = 0;
    unsigned int temp_data = 0;
    unsigned int temp_addr = 0;
    QString temp_string;

    for (int k = 0; k < upa->i2c_core_config.size(); k++)
    {
        if (ui->PpsAnalyzerComboBox->currentText() == upa->i2c_core_config.at(k).com_port)
        {
            //  clear values
            if (clear == true)
            {
                for (unsigned int i = 4; i < 12; i++)
                {
                    temp_addr = i;
                    temp_data = 0x00;
                    calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
                }

                // say cleared
                temp_addr = 12;
                temp_data = 0x01;
                calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);

                for (unsigned int i = 64; i < 64+(10*2); i++)
                {
                    temp_addr = i;
                    temp_data = 0x00;
                    calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
                }

                // write PPSA
                temp_addr = 0;
                temp_data = 0x50;
                calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);

                temp_addr = 1;
                temp_data = 0x50;
                calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);

                temp_addr = 2;
                temp_data = 0x53;
                calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);

                temp_addr = 3;
                temp_data = 0x41;
                calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);

                return;
            }
            else
            {
                temp_addr = 12;
                temp_data = 0x00;
                calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            }

            //Detector String PPSA
            temp_addr = 0;
            temp_data = 0x50; //P
            if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
            {
                temp_addr = 1;
                temp_data = 0x50; //P
                if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_addr = 2;
                    temp_data = 0x53; //S
                    if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        temp_addr = 3;
                        temp_data = 0x41; //A
                        if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                        {
                            //nothing
                        }
                        else
                        {
                            cout << "ERROR: " << "could not write PPSA identifier 3" << endl;
                        }
                    }
                    else
                    {
                        cout << "ERROR: " << "could not write PPSA identifier 2" << endl;
                    }
                }
                else
                {
                    cout << "ERROR: " << "could not write PPSA identifier 1" << endl;
                }
            }
            else
            {
                cout << "ERROR: " << "could not write PPSA identifier 0" << endl;
            }

            //Serial Number
            temp_string = ui->SerialNumberValue->text();
            temp_serial_number = temp_string.toUInt(nullptr, 10);
            if (temp_string == "NA")
            {
                //nothing
            }
            else
            {
                temp_addr = 4;
                temp_data = (temp_serial_number >> 24) & 0x000000FF;
                if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_addr = 5;
                    temp_data = (temp_serial_number >> 16) & 0x000000FF;
                    if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        temp_addr = 6;
                        temp_data = (temp_serial_number >> 8) & 0x000000FF;
                        if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                        {
                            temp_addr = 7;
                            temp_data = (temp_serial_number >> 0) & 0x000000FF;
                            if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                            {
                                // nothing
                            }
                            else
                            {
                                ui->SerialNumberValue->setText("NA");
                            }
                        }
                        else
                        {
                            ui->SerialNumberValue->setText("NA");
                        }
                    }
                    else
                    {
                        ui->SerialNumberValue->setText("NA");
                    }
                }
                else
                {
                    ui->SerialNumberValue->setText("NA");
                }
            }

            //Calibration Date
            QDateTime temp_date_formated = QDateTime::currentDateTime();
            temp_date_formated.toLocalTime();
            temp_date = (unsigned int)temp_date_formated.toSecsSinceEpoch() + temp_date_formated.offsetFromUtc();
            temp_addr = 8;
            temp_data = (temp_date >> 24) & 0x000000FF;
            if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
            {
                temp_addr = 9;
                temp_data = (temp_date >> 16) & 0x000000FF;
                if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_addr = 10;
                    temp_data = (temp_date >> 8) & 0x000000FF;
                    if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        temp_addr = 11;
                        temp_data = (temp_date >> 0) & 0x000000FF;
                        if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                        {
                            // nothing
                        }
                        else
                        {
                            ui->SerialNumberValue->setText("NA");
                        }
                    }
                    else
                    {
                        ui->SerialNumberValue->setText("NA");
                    }
                }
                else
                {
                    ui->SerialNumberValue->setText("NA");
                }
            }
            else
            {
                ui->SerialNumberValue->setText("NA");
            }

            //PPS Ref Out
            temp_string = ui->PpsCalibrationPpsRefOutValue->text();
            temp_nanoseconds = temp_string.toUInt(nullptr, 10);
            if (temp_string == "NA")
            {
                //nothing
            }
            else
            {
                temp_addr = 64;
                temp_data = (temp_nanoseconds >> 8) & 0x000000FF;
                if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_addr = 65;
                    temp_data = (temp_nanoseconds >> 0) & 0x000000FF;
                    if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        // nothing
                    }
                    else
                    {
                        ui->PpsCalibrationPpsRefOutValue->setText("NA");
                    }
                }
                else
                {
                    ui->PpsCalibrationPpsRefOutValue->setText("NA");
                }
            }

            //PPS Ref In
            temp_string = ui->PpsCalibrationPpsRefInValue->text();
            temp_nanoseconds = temp_string.toUInt(nullptr, 10);
            if (temp_string == "NA")
            {
                //nothing
            }
            else
            {
                temp_addr = 66;
                temp_data = (temp_nanoseconds >> 8) & 0x000000FF;
                if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_addr = 67;
                    temp_data = (temp_nanoseconds >> 0) & 0x000000FF;
                    if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        // nothing
                    }
                    else
                    {
                        ui->PpsCalibrationPpsRefInValue->setText("NA");
                    }
                }
                else
                {
                    ui->PpsCalibrationPpsRefInValue->setText("NA");
                }
            }

            //PPS 1
            temp_string = ui->PpsCalibrationPps1Value->text();
            temp_nanoseconds = temp_string.toUInt(nullptr, 10);
            if (temp_string == "NA")
            {
                //nothing
            }
            else
            {
                temp_addr = 68;
                temp_data = (temp_nanoseconds >> 8) & 0x000000FF;
                if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_addr = 69;
                    temp_data = (temp_nanoseconds >> 0) & 0x000000FF;
                    if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        // nothing
                    }
                    else
                    {
                        ui->PpsCalibrationPps1Value->setText("NA");
                    }
                }
                else
                {
                    ui->PpsCalibrationPps1Value->setText("NA");
                }
            }

            //PPS 2
            temp_string = ui->PpsCalibrationPps2Value->text();
            temp_nanoseconds = temp_string.toUInt(nullptr, 10);
            if (temp_string == "NA")
            {
                //nothing
            }
            else
            {
                temp_addr = 70;
                temp_data = (temp_nanoseconds >> 8) & 0x000000FF;
                if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_addr = 71;
                    temp_data = (temp_nanoseconds >> 0) & 0x000000FF;
                    if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        // nothing
                    }
                    else
                    {
                        ui->PpsCalibrationPps2Value->setText("NA");
                    }
                }
                else
                {
                    ui->PpsCalibrationPps2Value->setText("NA");
                }
            }

            //PPS 3
            temp_string = ui->PpsCalibrationPps3Value->text();
            temp_nanoseconds = temp_string.toUInt(nullptr, 10);
            if (temp_string == "NA")
            {
                //nothing
            }
            else
            {
                temp_addr = 72;
                temp_data = (temp_nanoseconds >> 8) & 0x000000FF;
                if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_addr = 73;
                    temp_data = (temp_nanoseconds >> 0) & 0x000000FF;
                    if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        // nothing
                    }
                    else
                    {
                        ui->PpsCalibrationPps3Value->setText("NA");
                    }
                }
                else
                {
                    ui->PpsCalibrationPps3Value->setText("NA");
                }
            }

            //PPS 4
            temp_string = ui->PpsCalibrationPps4Value->text();
            temp_nanoseconds = temp_string.toUInt(nullptr, 10);
            if (temp_string == "NA")
            {
                //nothing
            }
            else
            {
                temp_addr = 74;
                temp_data = (temp_nanoseconds >> 8) & 0x000000FF;
                if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_addr = 75;
                    temp_data = (temp_nanoseconds >> 0) & 0x000000FF;
                    if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        // nothing
                    }
                    else
                    {
                        ui->PpsCalibrationPps4Value->setText("NA");
                    }
                }
                else
                {
                    ui->PpsCalibrationPps4Value->setText("NA");
                }
            }

            //PPS 5
            temp_string = ui->PpsCalibrationPps5Value->text();
            temp_nanoseconds = temp_string.toUInt(nullptr, 10);
            if (temp_string == "NA")
            {
                //nothing
            }
            else
            {
                temp_addr = 76;
                temp_data = (temp_nanoseconds >> 8) & 0x000000FF;
                if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_addr = 77;
                    temp_data = (temp_nanoseconds >> 0) & 0x000000FF;
                    if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        // nothing
                    }
                    else
                    {
                        ui->PpsCalibrationPps5Value->setText("NA");
                    }
                }
                else
                {
                    ui->PpsCalibrationPps5Value->setText("NA");
                }
            }

            //PPS 6
            temp_string = ui->PpsCalibrationPps6Value->text();
            temp_nanoseconds = temp_string.toUInt(nullptr, 10);
            if (temp_string == "NA")
            {
                //nothing
            }
            else
            {
                temp_addr = 78;
                temp_data = (temp_nanoseconds >> 8) & 0x000000FF;
                if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_addr = 79;
                    temp_data = (temp_nanoseconds >> 0) & 0x000000FF;
                    if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        // nothing
                    }
                    else
                    {
                        ui->PpsCalibrationPps6Value->setText("NA");
                    }
                }
                else
                {
                    ui->PpsCalibrationPps6Value->setText("NA");
                }
            }

            //PPS 7
            temp_string = ui->PpsCalibrationPps7Value->text();
            temp_nanoseconds = temp_string.toUInt(nullptr, 10);
            if (temp_string == "NA")
            {
                //nothing
            }
            else
            {
                temp_addr = 80;
                temp_data = (temp_nanoseconds >> 8) & 0x000000FF;
                if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_addr = 81;
                    temp_data = (temp_nanoseconds >> 0) & 0x000000FF;
                    if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        // nothing
                    }
                    else
                    {
                        ui->PpsCalibrationPps7Value->setText("NA");
                    }
                }
                else
                {
                    ui->PpsCalibrationPps7Value->setText("NA");
                }
            }

            //PPS 8
            temp_string = ui->PpsCalibrationPps8Value->text();
            temp_nanoseconds = temp_string.toUInt(nullptr, 10);
            if (temp_string == "NA")
            {
                //nothing
            }
            else
            {
                temp_addr = 82;
                temp_data = (temp_nanoseconds >> 8) & 0x000000FF;
                if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_addr = 83;
                    temp_data = (temp_nanoseconds >> 0) & 0x000000FF;
                    if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        // nothing
                    }
                    else
                    {
                        ui->PpsCalibrationPps8Value->setText("NA");
                    }
                }
                else
                {
                    ui->PpsCalibrationPps8Value->setText("NA");
                }
            }

        }
    }
}

void Upa_CalibrateTab::calibrate_read_values_timer(void)
{
    calibrate_read_ts_values();
}

void Upa_CalibrateTab::calibrate_read_values_button_clicked(void)
{
    calibrate_read_values();
}

void Upa_CalibrateTab::calibrate_write_values_button_clicked(void)
{
    calibrate_write_values(false);
    calibrate_read_values();
}

void Upa_CalibrateTab::calibrate_read_mac_ip_values_button_clicked(void)
{
    unsigned int temp_data = 0;
    unsigned int temp_addr = 0;
    QString temp_string;

    for (int k = 0; k < upa->i2c_core_config.size(); k++)
    {
        if (ui->PpsAnalyzerComboBox->currentText() == upa->i2c_core_config.at(k).com_port)
        {
            // mac
            temp_string.clear();
            for (temp_addr = 32; temp_addr <= 37; temp_addr++)
            {
                if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_string.append(QString("%1").arg((temp_data & 0x000000FF), 2, 16, QLatin1Char('0')));
                    if (temp_addr == 37)
                    {
                        ui->PpsMacAddressValue->setText(temp_string);
                    }
                    else
                    {
                        temp_string.append(":");
                    }
                }
                else
                {
                    ui->PpsMacAddressValue->setText("NA");
                    break;
                }
            }

            // ip
            temp_string.clear();
            for (temp_addr = 40; temp_addr <= 43; temp_addr++)
            {
                if (0 == calibrate_eeprom_read_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                {
                    temp_string.append(QString::number(temp_data));
                    if (temp_addr == 43)
                    {
                        ui->PpsIpAddressValue->setText(temp_string);
                    }
                    else
                    {
                        temp_string.append(".");
                    }
                }
                else
                {
                    ui->PpsIpAddressValue->setText("NA");
                    break;
                }
            }
        }
    }
}

void Upa_CalibrateTab::calibrate_write_mac_ip_values_button_clicked(void)
{
    unsigned long long temp_mac;
    unsigned long temp_ip;
    unsigned int temp_data = 0;
    unsigned int temp_addr = 0;
    QString temp_string;

    for (int k = 0; k < upa->i2c_core_config.size(); k++)
    {
        if (ui->PpsAnalyzerComboBox->currentText() == upa->i2c_core_config.at(k).com_port)
        {
            // mac
            temp_string = ui->PpsMacAddressValue->text();
            temp_string.remove(QChar(':'), Qt::CaseInsensitive);
            temp_mac = temp_string.toULongLong(nullptr, 16);
            if (ui->PpsMacAddressValue->text() != "NA")
            {
                for (temp_addr = 32; temp_addr <= 37; temp_addr++)
                {
                    temp_data = (temp_mac >> ((37-temp_addr)*8)) & 0xFF;
                    if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        // nothing
                    }
                    else
                    {
                        ui->PpsMacAddressValue->setText("NA");
                        break;
                    }
                }
            }

            // ip
            temp_string = ui->PpsIpAddressValue->text();
            temp_ip = QHostAddress(temp_string).toIPv4Address();
            if (ui->PpsIpAddressValue->text() != "NA")
            {
                for (temp_addr = 40; temp_addr <= 43; temp_addr++)
                {
                    temp_data = (temp_ip >> ((43-temp_addr)*8)) & 0xFF;
                    if (0 == calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data))
                    {
                        // nothing
                    }
                    else
                    {
                        ui->PpsIpAddressValue->setText("NA");
                        break;
                    }
                }
            }
        }
    }
    calibrate_read_mac_ip_values_button_clicked();
}

void Upa_CalibrateTab::calibrate_calibrate_button_clicked(void)
{
    QElapsedTimer temp_timer;
    int sync_count = 0;
    unsigned int temp_value = 0;
    unsigned int temp_data = 0;
    unsigned int temp_addr = 0;

    QAbstractButton *next_button;
    QAbstractButton *cancel_button;


    for (int k = 0; k < upa->i2c_core_config.size(); k++)
    {
        if (ui->PpsAnalyzerComboBox->currentText() == upa->i2c_core_config.at(k).com_port)
        {
            // set switches
            QMessageBox message_shield;
            message_shield.setWindowTitle(tr("Connect Calibration Shield"));
            message_shield.setInformativeText("Please connect the Calibration shield \n");
            next_button = message_shield.addButton(tr("Done - Next Step"), QMessageBox::YesRole);
            cancel_button = message_shield.addButton(tr("Abort Calibration"), QMessageBox::NoRole);
            message_shield.setIcon(QMessageBox::Question);
            message_shield.exec();

            if(message_shield.clickedButton() == cancel_button)
            {
                return;
            }

            // clear memory
            QProgressDialog progress_clear("Clearing Memory", "Abort Calibration", 0, 128, this);
            progress_clear.setWindowModality(Qt::WindowModal);
            for (unsigned int i = 0; i < 128; i++)
            {
                if (((i >= 32) && (i <= 37)) ||
                    ((i >= 40) && (i <= 43)))
                {
                    // don't clear the mac and ip
                }
                else
                {
                    temp_addr = i;
                    temp_data = 0x00;
                    calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
                }

                progress_clear.setValue(i+1);
                if (progress_clear.wasCanceled())
                {
                    return;
                }
            }

            // say cleared
            temp_addr = 12;
            temp_data = 0x01;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);


            // set switches
            QMessageBox message_switches;
            message_switches.setWindowTitle(tr("Set Switches"));
            message_switches.setInformativeText("Please set the Switches on the Arty to positions: \n\n SW0 = OFF \n SW1 = OFF \n SW2 = ON \n SW3 = OFF \n");
            next_button = message_switches.addButton(tr("Done - Next Step"), QMessageBox::YesRole);
            cancel_button = message_switches.addButton(tr("Abort Calibration"), QMessageBox::NoRole);
            message_switches.setIcon(QMessageBox::Question);
            message_switches.exec();

            if(message_switches.clickedButton() == cancel_button)
            {
                return;
            }

            // set jumpers
            QMessageBox message_jumper1;
            message_jumper1.setWindowTitle(tr("Set Jumpers"));
            message_jumper1.setInformativeText("Please set the Jumpers on the Calibration Shield to postions: \n\n JP-CAL1 = 1-2 \n JP-CAL2 = 2-4 \n");
            next_button = message_jumper1.addButton(tr("Done - Next Step"), QMessageBox::YesRole);
            cancel_button = message_jumper1.addButton(tr("Abort Calibration"), QMessageBox::NoRole);
            message_jumper1.setIcon(QMessageBox::Question);
            message_jumper1.exec();

            if(message_jumper1.clickedButton() == cancel_button)
            {
                return;
            }

            // wait to sync
            QProgressDialog progress_sync1("Sync to Calib PPS", "Abort Calibration", 0, 10, this);
            progress_sync1.setWindowModality(Qt::WindowModal);
            sync_count = 0;
            progress_sync1.setValue(sync_count);
            while (sync_count < 10)
            {
                temp_timer.start();
                while(!temp_timer.hasExpired(1000))
                {
                    QThread::msleep(100);
                }
                if (progress_sync1.wasCanceled())
                {
                    return;
                }
                if (abs(ui->PpsOffsetPpsRefInValue->text().toInt(nullptr, 10)) < 20)
                {
                    sync_count++;
                    progress_sync1.setValue(sync_count);
                }
                //else
                //{
                //    sync_count = 0;
                //}
            }

            // get and write PPS Ref Out value
            QProgressDialog progress_write_pps_ref_out("Writing PPS Out Ref Calibration", "Abort Calibration", 0, 6, this);
            progress_write_pps_ref_out.setWindowModality(Qt::WindowModal);
            progress_write_pps_ref_out.setValue(0);

            temp_value = abs(ui->PpsOffsetPps1Value->text().toInt(nullptr, 10));
            ui->PpsCalibrationPpsRefOutValue->setText(QString::number(temp_value));

            temp_addr = 64;
            temp_data = (temp_value >> 8) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps_ref_out.setValue(1);

            temp_addr = 65;
            temp_data = (temp_value >> 0) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps_ref_out.setValue(2);

            // write PPSA
            temp_addr = 0;
            temp_data = 0x50;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps_ref_out.setValue(3);

            temp_addr = 1;
            temp_data = 0x50;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps_ref_out.setValue(4);

            temp_addr = 2;
            temp_data = 0x53;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps_ref_out.setValue(5);

            temp_addr = 3;
            temp_data = 0x41;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps_ref_out.setValue(6);

            if (progress_write_pps_ref_out.wasCanceled())
            {
                return;
            }

            // set jumpers
            QMessageBox message_jumper2;
            message_jumper2.setWindowTitle(tr("Set Jumpers"));
            message_jumper2.setInformativeText("Please set the Jumpers on the Calibration Shield to postions: \n\n JP-CAL1 = 1-2 \n JP-CAL2 = 1-2 \n");
            next_button = message_jumper2.addButton(tr("Done - Next Step"), QMessageBox::YesRole);
            cancel_button = message_jumper2.addButton(tr("Abort Calibration"), QMessageBox::NoRole);
            message_jumper2.setIcon(QMessageBox::Question);
            message_jumper2.exec();

            if(message_jumper2.clickedButton() == cancel_button)
            {
                return;
            }

            // wait to sync
            QProgressDialog progress_sync2("Sync to Calib PPS", "Abort Calibration", 0, 10, this);
            progress_sync2.setWindowModality(Qt::WindowModal);
            sync_count = 0;
            progress_sync2.setValue(sync_count);
            while (sync_count < 10)
            {
                temp_timer.start();
                while(!temp_timer.hasExpired(1000))
                {
                    QThread::msleep(100);
                }
                if (progress_sync2.wasCanceled())
                {
                    return;
                }
                if (abs(ui->PpsOffsetPpsRefInValue->text().toInt(nullptr, 10)) < 20)
                {
                    sync_count++;
                    progress_sync2.setValue(sync_count);
                }
                //else
                //{
                //    sync_count = 0;
                //}
            }

            // get and write PPS Ref In value
            QProgressDialog progress_write_pps_ref_in("Writing PPS In Ref Calibration", "Abort Calibration", 0, 2, this);
            progress_write_pps_ref_in.setWindowModality(Qt::WindowModal);
            progress_write_pps_ref_in.setValue(0);

            temp_value = abs(ui->PpsOffsetPps1Value->text().toInt(nullptr, 10));
            ui->PpsCalibrationPpsRefInValue->setText(QString::number(temp_value));

            temp_addr = 66;
            temp_data = (temp_value >> 8) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps_ref_in.setValue(1);

            temp_addr = 67;
            temp_data = (temp_value >> 0) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps_ref_in.setValue(2);

            if (progress_write_pps_ref_in.wasCanceled())
            {
                return;
            }

            // set jumpers
            QMessageBox message_jumper3;
            message_jumper3.setWindowTitle(tr("Set Jumpers"));
            message_jumper3.setInformativeText("Please set the Jumpers on the Calibration Shield to postions: \n\n JP-CAL1 = 1-2 \n JP-CAL2 = 2-3 \n");
            next_button = message_jumper3.addButton(tr("Done - Next Step"), QMessageBox::YesRole);
            cancel_button = message_jumper3.addButton(tr("Abort Calibration"), QMessageBox::NoRole);
            message_jumper3.setIcon(QMessageBox::Question);
            message_jumper3.exec();

            if(message_jumper3.clickedButton() == cancel_button)
            {
                return;
            }

            // wait to sync
            QProgressDialog progress_sync3("Sync to Calib PPS", "Abort Calibration", 0, 10, this);
            progress_sync3.setWindowModality(Qt::WindowModal);
            sync_count = 0;
            progress_sync3.setValue(sync_count);
            while (sync_count < 10)
            {
                temp_timer.start();
                while(!temp_timer.hasExpired(1000))
                {
                    QThread::msleep(100);
                }
                if (progress_sync3.wasCanceled())
                {
                    return;
                }
                if (abs(ui->PpsOffsetPpsRefInValue->text().toInt(nullptr, 10)) < 20)
                {
                    sync_count++;
                    progress_sync3.setValue(sync_count);
                }
                //else
                //{
                //    sync_count = 0;
                //}
            }

            // get and write PPS values
            QProgressDialog progress_write_pps("Writing PPS Calibrations", "Abort Calibration", 0, 16, this);
            progress_write_pps.setWindowModality(Qt::WindowModal);
            progress_write_pps.setValue(0);

            temp_value = abs(ui->PpsOffsetPps1Value->text().toInt(nullptr, 10));
            ui->PpsCalibrationPps1Value->setText(QString::number(temp_value));

            temp_addr = 68;
            temp_data = (temp_value >> 8) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(1);

            temp_addr = 69;
            temp_data = (temp_value >> 0) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(2);

            if (progress_write_pps.wasCanceled())
            {
                return;
            }

            temp_value = abs(ui->PpsOffsetPps2Value->text().toInt(nullptr, 10));
            ui->PpsCalibrationPps2Value->setText(QString::number(temp_value));

            temp_addr = 70;
            temp_data = (temp_value >> 8) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(3);

            temp_addr = 71;
            temp_data = (temp_value >> 0) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(4);

            if (progress_write_pps.wasCanceled())
            {
                return;
            }

            temp_value = abs(ui->PpsOffsetPps3Value->text().toInt(nullptr, 10));
            ui->PpsCalibrationPps3Value->setText(QString::number(temp_value));

            temp_addr = 72;
            temp_data = (temp_value >> 8) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(5);

            temp_addr = 73;
            temp_data = (temp_value >> 0) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(6);

            if (progress_write_pps.wasCanceled())
            {
                return;
            }

            temp_value = abs(ui->PpsOffsetPps4Value->text().toInt(nullptr, 10));
            ui->PpsCalibrationPps4Value->setText(QString::number(temp_value));

            temp_addr = 74;
            temp_data = (temp_value >> 8) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(7);

            temp_addr = 75;
            temp_data = (temp_value >> 0) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(8);

            if (progress_write_pps.wasCanceled())
            {
                return;
            }

            temp_value = abs(ui->PpsOffsetPps5Value->text().toInt(nullptr, 10));
            ui->PpsCalibrationPps5Value->setText(QString::number(temp_value));

            temp_addr = 76;
            temp_data = (temp_value >> 8) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(9);

            temp_addr = 77;
            temp_data = (temp_value >> 0) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(10);

            if (progress_write_pps.wasCanceled())
            {
                return;
            }

            temp_value = abs(ui->PpsOffsetPps6Value->text().toInt(nullptr, 10));
            ui->PpsCalibrationPps6Value->setText(QString::number(temp_value));

            temp_addr = 78;
            temp_data = (temp_value >> 8) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(11);

            temp_addr = 79;
            temp_data = (temp_value >> 0) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(12);

            if (progress_write_pps.wasCanceled())
            {
                return;
            }

            temp_value = abs(ui->PpsOffsetPps7Value->text().toInt(nullptr, 10));
            ui->PpsCalibrationPps7Value->setText(QString::number(temp_value));

            temp_addr = 80;
            temp_data = (temp_value >> 8) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(13);

            temp_addr = 81;
            temp_data = (temp_value >> 0) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(14);

            if (progress_write_pps.wasCanceled())
            {
                return;
            }

            temp_value = abs(ui->PpsOffsetPps8Value->text().toInt(nullptr, 10));
            ui->PpsCalibrationPps8Value->setText(QString::number(temp_value));

            temp_addr = 82;
            temp_data = (temp_value >> 8) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(15);

            temp_addr = 83;
            temp_data = (temp_value >> 0) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_pps.setValue(16);

            if (progress_write_pps.wasCanceled())
            {
                return;
            }

            // get and write serial number and date
            bool ok;
            temp_value = (unsigned int)QInputDialog::getInt(this, tr("Serial Number"), tr("Please enter a Serial Number:"), 0, 0, 1000000, 1, &ok);
            if (ok == false)
            {
                temp_value = 0; // we just say 0
            }
            QProgressDialog progress_write_meta("Writing Meta Information", "Abort Calibration", 0, 8, this);
            progress_write_meta.setWindowModality(Qt::WindowModal);
            progress_write_meta.setValue(0);

            ui->SerialNumberValue->setText(QString::number(temp_value));

            temp_addr = 4;
            temp_data = (temp_value >> 24) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_meta.setValue(1);

            temp_addr = 5;
            temp_data = (temp_value >> 16) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_meta.setValue(2);

            temp_addr = 6;
            temp_data = (temp_value >> 8) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_meta.setValue(3);

            temp_addr = 7;
            temp_data = (temp_value >> 0) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_meta.setValue(4);

            if (progress_write_meta.wasCanceled())
            {
                return;
            }

            //Calibration date
            QDateTime temp_date_formated = QDateTime::currentDateTime();
            temp_date_formated.toLocalTime();
            temp_value = (unsigned int)temp_date_formated.toSecsSinceEpoch() + temp_date_formated.offsetFromUtc();
            temp_date_formated.setSecsSinceEpoch(temp_value);

            ui->CalibrationDateValue->setText(temp_date_formated.toUTC().toString("dd.MM.yyyy hh:mm:ss"));

            temp_addr = 8;
            temp_data = (temp_value >> 24) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_meta.setValue(5);

            temp_addr = 9;
            temp_data = (temp_value >> 16) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_meta.setValue(6);

            temp_addr = 10;
            temp_data = (temp_value >> 8) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_meta.setValue(7);

            temp_addr = 11;
            temp_data = (temp_value >> 0) & 0xFF;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);
            progress_write_meta.setValue(8);

            if (progress_write_meta.wasCanceled())
            {
                return;
            }

            // say done
            temp_addr = 12;
            temp_data = 0x00;
            calibrate_eeprom_write_reg(&(upa->i2c_core_config.at(k)), temp_addr, temp_data);

            ui->CalibratedCheckBox->setChecked(true);

            QMessageBox::information(this, tr("Done"), tr("Calibration finished"));
        }
    }
}



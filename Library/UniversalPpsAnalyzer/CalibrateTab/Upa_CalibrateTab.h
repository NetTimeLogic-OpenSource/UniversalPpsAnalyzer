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

#ifndef UPA_CALIBRATE_H
#define UPA_CALIBRATE_H

#include <QWidget>
#include <QTimer>
#include <QtCore/QDateTime>
#include <Upa_UniversalPpsAnalyzer.h>
#include <Upa_PpsTab.h>
#include <Upa_CoreConfig.h>

#define Upa_I2cConf_EepromAddr                            0x50

#define Upa_I2cConf_ControlReg                            0x00000000
#define Upa_I2cConf_ChipAddrReg                           0x00000004
#define Upa_I2cConf_RegAddrReg                            0x00000008
#define Upa_I2cConf_DataReg                               0x0000000C

class Upa_UniversalPpsAnalyzer;

namespace Ui {
class Upa_CalibrateTab;
}

class Upa_CalibrateTab : public QWidget
{
    Q_OBJECT

public:
    Upa_CalibrateTab(Upa_UniversalPpsAnalyzer* parent);
    ~Upa_CalibrateTab();

    int calibrate_resize(int height, int width);
    int calibrate_disable(void);
    int calibrate_enable(void);

public:
    Upa_UniversalPpsAnalyzer* upa;

private:
    Ui::Upa_CalibrateTab *ui;

    int calibrate_eeprom_write_reg(const Upa_CoreConfig* eeprom, const unsigned int addr, unsigned int& data);
    int calibrate_eeprom_read_reg(const Upa_CoreConfig* eeprom, const unsigned int addr, unsigned int& data);

    // Calibrate tab
    QTimer* calibrate_timer;

    void calibrate_read_ts_values(void);

    void calibrate_read_values(void);
    void calibrate_write_values(bool clear);

private slots:
    // Calibrate tab
    void calibrate_read_values_timer(void);
    void calibrate_read_values_button_clicked(void);
    void calibrate_write_values_button_clicked(void);
    void calibrate_read_mac_ip_values_button_clicked(void);
    void calibrate_write_mac_ip_values_button_clicked(void);
    void calibrate_calibrate_button_clicked(void);
};

#endif // UPA_CALIBRATE_H

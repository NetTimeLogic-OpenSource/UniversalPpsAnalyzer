//*****************************************************************************************
// Project:     UniversalPpsAnalyzer
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

#ifndef UPA_CONFIGTAB_H
#define UPA_CONFIGTAB_H

#include <QWidget>
#include <QMessageBox>
#include <Upa_UniversalPpsAnalyzer.h>

using namespace std;

class Upa_UniversalPpsAnalyzer;

#define Upa_Config_BlockSize                            16

#define Upa_Config_TypeInstanceReg                      0x00000000
#define Upa_Config_BaseAddrLReg                         0x00000004
#define Upa_Config_BaseAddrHReg                         0x00000008
#define Upa_Config_IrqMaskReg                           0x0000000C


namespace Ui {
class Upa_ConfigTab;
}

class Upa_ConfigTab : public QWidget
{
    Q_OBJECT

public:
    Upa_UniversalPpsAnalyzer* upa;
    Upa_ConfigTab(Upa_UniversalPpsAnalyzer* parent);
    ~Upa_ConfigTab();
    int config_resize(int height, int width);

private:
    Ui::Upa_ConfigTab *ui;

    // Config tab
    QTimer* config_timer;
    void config_init_interfaces(void);


private slots:
    void config_connect_button_clicked(void);
    void config_full_screen_button_clicked(void);
    void config_com_port_check_timer(void);
};

#endif // UPA_CONFIGTAB_H

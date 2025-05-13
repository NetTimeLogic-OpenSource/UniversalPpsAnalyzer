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

#ifndef UPA_ADVANCEDTAB_H
#define UPA_ADVANCEDTAB_H

#include <QWidget>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <Upa_UniversalPpsAnalyzer.h>
#include <Upa_CalibrateTab.h>

class Upa_UniversalPpsAnalyzer;
class Upa_CalibrateTab;

namespace Ui {
class Upa_AdvancedTab;
class Upa_CalibrateTab;
}

class Upa_AdvancedTab : public QWidget
{
    Q_OBJECT

public:
    Upa_AdvancedTab(Upa_UniversalPpsAnalyzer* parent);
    ~Upa_AdvancedTab();

    int advanced_resize(int height, int width);
    int advanced_disable(void);
    int advanced_enable(void);
    int advanced_disable_calibration(void);
    int advanced_enable_calibration(void);


private:
    Upa_UniversalPpsAnalyzer* upa;
    Ui::Upa_AdvancedTab *ui;

    // Advanced tab
    Upa_DebugStream* debug_stream;

    // Config tab
    Upa_CalibrateTab* calibrate_tab;

private slots:
    void advanced_clear_log_button_clicked(void);
    void advanced_save_log_button_clicked(void);
    void advanced_save_log_file_browse_button_clicked(void);
    void advanced_calibrate_button_clicked(void);
};

#endif // UPA_ADVANCEDTAB_H

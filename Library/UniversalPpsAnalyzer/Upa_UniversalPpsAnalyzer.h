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

#ifndef UPA_UNIVERSALCONFIGURATIONMANAGER_H
#define UPA_UNIVERSALCONFIGURATIONMANAGER_H

#include <QScreen>
#include <QCloseEvent>
#include <QWidget>
#include <QTimer>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCore/QDateTime>
#include <QtCharts/QValueAxis>
#include <ui_Upa_UniversalPpsAnalyzer.h>
#include <Upa_CommunicationLib.h>
#include <Upa_CoreConfig.h>
#include <Upa_DebugStream.h>

#include <Upa_ConfigTab.h>
#include <Upa_AdvancedTab.h>
#include <Upa_PpsTab.h>
#include <Upa_HelpMenuAbout.h>

#if QT_MAJOR_VERSION < 6
    using namespace QtCharts;
#endif

class Upa_ConfigTab;
class Upa_AdvancedTab;
class Upa_PpsTab;
class Upa_HelpMenuAbout;


#define Upa_Version_Nr               "4.3.00"

#define Upa_MainHeight               820
#define Upa_MainWidth                1380

class Upa_UniversalPpsAnalyzer : public QMainWindow, public Ui::Upa_UniversalPpsAnalyzer
{
    Q_OBJECT

    public:
        Upa_UniversalPpsAnalyzer(QMainWindow *parent = 0, int open_gl = 1);
        ~Upa_UniversalPpsAnalyzer();

        int upa_resize(int height, int width);

        QList<Upa_CommunicationLib*> com_lib;
        QList<Upa_CoreConfig> ts_core_config;
        QList<Upa_CoreConfig> io_core_config;
        QList<Upa_CoreConfig> i2c_core_config;

        // If OpenGl shall be used
        int use_open_gl;

        // Config tab
        Upa_ConfigTab* config_tab;

        // Advcanced tab
        Upa_AdvancedTab* advanced_tab;

        // Pps tab
        Upa_PpsTab* pps_tab;

private:
        void resizeEvent(QResizeEvent* event);
        void closeEvent(QCloseEvent *event);

        Upa_HelpMenuAbout about_screen;

private slots:
        // File menu
        void upa_file_exit_clicked(void);
        // Help menu
        void upa_help_about_clicked(void);
};

#endif // UPA_UNIVERSALCONFIGURATIONMANAGER_H

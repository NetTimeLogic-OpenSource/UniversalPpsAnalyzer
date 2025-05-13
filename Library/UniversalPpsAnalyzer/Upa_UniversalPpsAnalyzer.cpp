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

#include <Upa_UniversalPpsAnalyzer.h>

using namespace std;
#include <iostream>
#include <streambuf>
#include <string>

Upa_UniversalPpsAnalyzer::Upa_UniversalPpsAnalyzer(QMainWindow *parent, int open_gl) : QMainWindow(parent)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    int height = screenGeometry.height();
    int width = screenGeometry.width();

    // set this first
    use_open_gl = open_gl;

    cout << "INFO: " << "Screen size: " << width << "x" << height << endl;

    setupUi(this);

    // File menu
    connect(Upa_Exit, SIGNAL(triggered()), this, SLOT(upa_file_exit_clicked()));

    // Help menu
    connect(Upa_About, SIGNAL(triggered()), this, SLOT(upa_help_about_clicked()));

    // Config Tab
    config_tab = new Upa_ConfigTab(this);
    Upa_MainTab->addTab(config_tab, "Config"); // show this at startup

    // Advanced Tab
    advanced_tab = new Upa_AdvancedTab(this);
    Upa_MainTab->addTab(advanced_tab, "Advanced"); // show this at startup

    pps_tab = NULL;

    if (height > (Upa_MainHeight+2))
    {
        height = (Upa_MainHeight+2);
    }
    if (width > Upa_MainWidth)
    {
        width = Upa_MainWidth;
    }
    this->resize(width, height);
}


Upa_UniversalPpsAnalyzer::~Upa_UniversalPpsAnalyzer()
{
    delete config_tab;
    delete advanced_tab;
    ts_core_config.clear();
    io_core_config.clear();
    i2c_core_config.clear();
}

int Upa_UniversalPpsAnalyzer::upa_resize(int height, int width)
{
    int height_delta = 0;
    int width_delta = 0;

    cout << "INFO: " << "Resize to: " << width << "x" << height << endl;

    height_delta = (height-Upa_MainHeight);
    width_delta = (width-Upa_MainWidth);

    Upa_MainTab->setFixedHeight(Upa_MainHeight-20+height_delta);
    Upa_MainTab->setFixedWidth(Upa_MainWidth+width_delta);

    Upa_MainMenu->setFixedWidth(Upa_MainWidth+width_delta);

    // Config Tab
    config_tab->config_resize(height, width);

    // Advanced Tab
    advanced_tab->advanced_resize(height, width);

    // Pps Tab
    if (pps_tab != NULL)
    {
        pps_tab->pps_resize(height, width);
    }

    updateGeometry();

    return 0;
}

/******************************************************************/
// Resize
/******************************************************************/
void Upa_UniversalPpsAnalyzer::resizeEvent(QResizeEvent* event)
{
   QSize new_size = this->size();

   upa_resize(new_size.height(), new_size.width());

}

/******************************************************************/
// X Close
/******************************************************************/
void Upa_UniversalPpsAnalyzer::closeEvent(QCloseEvent *event)
{
    ts_core_config.clear();
    io_core_config.clear();
    i2c_core_config.clear();
    com_lib.clear();
    exit(0);
}

/******************************************************************/
// File menu
/******************************************************************/
void Upa_UniversalPpsAnalyzer::upa_file_exit_clicked(void)
{
    ts_core_config.clear();
    io_core_config.clear();
    i2c_core_config.clear();
    com_lib.clear();
    exit(0);
}

/******************************************************************/
// Help menu
/******************************************************************/
void Upa_UniversalPpsAnalyzer::upa_help_about_clicked(void)
{
    about_screen.show();
}


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

#include <Upa_AdvancedTab.h>
#include <ui_Upa_AdvancedTab.h>

Upa_AdvancedTab::Upa_AdvancedTab(Upa_UniversalPpsAnalyzer *parent) : QWidget()
{
    upa = parent;

    ui = new Ui::Upa_AdvancedTab();
    ui->setupUi(this);

    debug_stream = new Upa_DebugStream(cout, ui->AdvancedDebugConsole);

    connect(ui->AdvancedClearLogButton, SIGNAL(clicked()), this, SLOT(advanced_clear_log_button_clicked()));
    connect(ui->AdvancedSaveLogButton, SIGNAL(clicked()), this, SLOT(advanced_save_log_button_clicked()));
    connect(ui->AdvancedSaveLogFileBrowseButton, SIGNAL(clicked()), this, SLOT(advanced_save_log_file_browse_button_clicked()));
    connect(ui->AdvancedCalibrateButton, SIGNAL(clicked()), this, SLOT(advanced_calibrate_button_clicked()));

}

Upa_AdvancedTab::~Upa_AdvancedTab()
{
    delete ui;
}

int Upa_AdvancedTab::advanced_resize(int height, int width)
{
    int height_delta = 0;
    int width_delta = 0;

    height_delta = (height-Upa_MainHeight);
    width_delta = (width-Upa_MainWidth);

    ui->AdvancedDebugConsole->setFixedHeight(580+height_delta);
    ui->AdvancedDebugConsole->setFixedWidth(1340+width_delta);

    updateGeometry();

    return 0;
}

int Upa_AdvancedTab::advanced_enable(void)
{
    return 0;
}

int Upa_AdvancedTab::advanced_disable(void)
{
    advanced_disable_calibration();
    return 0;
}

int Upa_AdvancedTab::advanced_enable_calibration(void)
{
    ui->AdvancedCalibrateButton->setEnabled(true);
    return 0;
}

int Upa_AdvancedTab::advanced_disable_calibration(void)
{
    ui->AdvancedCalibrateButton->setText("Calibrate");
    ui->AdvancedCalibrateButton->setEnabled(false);
    return 0;
}

void Upa_AdvancedTab::advanced_clear_log_button_clicked(void)
{
    ui->AdvancedDebugConsole->clear();
    debug_stream->line_nr = 0;
}

void Upa_AdvancedTab::advanced_save_log_button_clicked(void)
{
    if (ui->AdvancedSaveLogFileValue->text() == "NA")
    {
        cout << "ERROR: " << "no save log file specified" << endl;
        QMessageBox::information(this, tr("Save Log"), tr("No save log file specified"));
        return;
    }

    QFile temp_file(ui->AdvancedSaveLogFileValue->text());

    if (false == temp_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        cout << "ERROR: " << "opening save log file:" << ui->AdvancedSaveLogFileValue->text().toLatin1().constData() << " failed" << endl;
        QMessageBox::information(this, tr("Save Log"), tr("Opening save log file failed"));
        return;
    }

    QTextStream temp_stream(&temp_file);
    QString temp_string = ui->AdvancedDebugConsole->toPlainText();

    temp_stream << temp_string << "\n"; //endl;

    temp_file.close();

    cout << "INFO: " << "log file saved" << endl;

    QMessageBox::information(this, tr("Save Log"), tr("Log saved to file"));
}

void Upa_AdvancedTab::advanced_save_log_file_browse_button_clicked(void)
{
    QString temp_directory;

    if (ui->AdvancedSaveLogFileValue->text() == "NA")
    {
        temp_directory = QDir::currentPath();
    }
    else
    {
        temp_directory = ui->AdvancedSaveLogFileValue->text();
    }

    QString temp_file = QFileDialog::getSaveFileName(this, tr("save log"), temp_directory, tr("config files (*.log)"));
    if(false == temp_file.isEmpty())
    {
        ui->AdvancedSaveLogFileValue->setText(temp_file);
    }
}

void Upa_AdvancedTab::advanced_calibrate_button_clicked(void)
{
    if (ui->AdvancedCalibrateButton->text() == "Calibrate")
    {
        // PPS Tab
        calibrate_tab = new Upa_CalibrateTab(upa);
        upa->Upa_MainTab->addTab(calibrate_tab, "Calibrate");
        QSize new_size = upa->Upa_MainWindow->size();
        calibrate_tab->calibrate_resize((new_size.height()+21), new_size.width());
        calibrate_tab->calibrate_enable();

        //PPS Tab
        upa->pps_tab->pps_disable();
        delete(upa->pps_tab);
        upa->pps_tab = NULL;

        ui->AdvancedCalibrateButton->setText("Stop Calibration");
    }
    else
    {
        //PPS Tab
        calibrate_tab->calibrate_disable();
        delete(calibrate_tab);
        calibrate_tab = NULL;

        // PPS Tab
        upa->pps_tab = new Upa_PpsTab(upa);
        upa->Upa_MainTab->addTab(upa->pps_tab, "Pps");
        QSize new_size = upa->Upa_MainWindow->size();
        upa->pps_tab->pps_resize((new_size.height()+21), new_size.width());
        upa->pps_tab->pps_enable();

        ui->AdvancedCalibrateButton->setText("Calibrate");
    }

}


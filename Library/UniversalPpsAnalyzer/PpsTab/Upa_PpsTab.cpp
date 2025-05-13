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

#include <QElapsedTimer>
#include <QFuture>
#include <QStandardPaths>
#include <cmath>

#include <Upa_PpsTab.h>
#include <ui_Upa_PpsTab.h>
#include <ui_Upa_PpsConfigScreen.h>
#include <ui_Upa_PpsThresholdScreen.h>

static void pps_calc_variance(Upa_PpsTab* upa_pps);
static void pps_gui_chart_sample(Upa_PpsTab* upa_pps);
static void pps_gui_histogram_sample(Upa_PpsTab* upa_pps);
static void pps_gui_variance_sample(Upa_PpsTab* upa_pps);

Upa_PpsTs::Upa_PpsTs()
{
    pps_board = NULL;
    ts_core_config = NULL;
    pps_sample.pps_active = 0;
    pps_sample.pps_offset = 0;
    pps_sample.pps_fetch_time = QDateTime::currentDateTime();
}

Upa_PpsTs::~Upa_PpsTs()
{
    pps_board = NULL;
    ts_core_config = NULL;
}

int Upa_PpsTs::pps_disable(void)
{
    //unsigned int temp_data = 0;
    //unsigned int temp_addr = 0;
    //QString temp_string;

    pps_sample.pps_active = 0;
    pps_sample.pps_offset = 0;
    pps_sample.pps_fetch_time = QDateTime::currentDateTime();

    // since we support multiuser we can not disable
    //temp_addr =  ts_core_config->address_range_low;
    //temp_string = pps_board->pps_gui[pps_index].pps_name;
    //temp_data = 0x00000000;
    //if (0 == ts_core_config->com_lib->write_reg(temp_addr + Upa_ClkTs_ControlReg, temp_data))
    //{
    //    cout << "VERBOSE: " << "enabled " << temp_string.toLatin1().constData() << endl;
    //}
    //else
    //{
    //    cout << "ERROR: " << "could not enable" << temp_string.toLatin1().constData() << endl;
    //    return -1;
    //}
    //temp_data = 0x00000000;
    //if (0 == ts_core_config->com_lib->write_reg(temp_addr + Upa_ClkTs_IrqMaskReg, temp_data))
    //{
    //    cout << "VERBOSE: " << "enabled irq on " << temp_string.toLatin1().constData() << endl;
    //}
    //else
    //{
    //    cout << "ERROR: " << "could not enable irq on" << temp_string.toLatin1().constData() << endl;
    //    return -1;
    //}

    return 0;
}

int Upa_PpsTs::pps_enable(void)
{
    unsigned int temp_data = 0;
    unsigned int temp_addr = 0;
    QString temp_string;

    pps_sample.pps_active = 0;
    pps_sample.pps_offset = 0;
    pps_sample.pps_fetch_time = QDateTime::currentDateTime();

    temp_addr =  ts_core_config->address_range_low;
    temp_string = pps_board->pps_gui[pps_index].pps_name;
    temp_data = 0x00000001;
    if (0 == ts_core_config->com_lib->write_reg(temp_addr + Upa_ClkTs_ControlReg, temp_data))
    {
        cout << "VERBOSE: " << "enabled " << temp_string.toLatin1().constData() << endl;
    }
    else
    {
        cout << "ERROR: " << "could not enable" << temp_string.toLatin1().constData() << endl;
        return -1;
    }
    temp_data = 0x00000001;
    if (0 == ts_core_config->com_lib->write_reg(temp_addr + Upa_ClkTs_IrqMaskReg, temp_data))
    {
        cout << "VERBOSE: " << "enabled irq on " << temp_string.toLatin1().constData() << endl;
    }
    else
    {
        cout << "ERROR: " << "could not enable irq on" << temp_string.toLatin1().constData() << endl;
        return -1;
    }

    return 0;
}


Upa_PpsCtrl::Upa_PpsCtrl()
{
    pps_board = NULL;
    pps_index = 0;
    pps_delay = 0;
    pps_threshold_enabled = 0;
    pps_threshold_high = 0;
    pps_threshold_low = 0;
    pps_threshold_high_exceeded = 0;
    pps_threshold_low_exceeded = 0;
    pps_samples.clear();
}

Upa_PpsCtrl::~Upa_PpsCtrl()
{
    pps_board = NULL;
    pps_index = 0;
    pps_samples.clear();
}

int Upa_PpsCtrl::pps_disable(void)
{
    pps_samples.clear();
    pps_threshold_high_exceeded = 0;
    pps_threshold_low_exceeded = 0;
    return 0;
}

int Upa_PpsCtrl::pps_enable(void)
{
    pps_samples.clear();
    pps_threshold_high_exceeded = 0;
    pps_threshold_low_exceeded = 0;
    return 0;
}


Upa_PpsLog::Upa_PpsLog()
{
    pps_board = NULL;
    pps_index = 0;
    pps_samples.clear();
}

Upa_PpsLog::~Upa_PpsLog()
{
    pps_board = NULL;
    pps_index = 0;
    pps_samples.clear();
}

int Upa_PpsLog::pps_disable(void)
{
    pps_samples.clear();
    return 0;
}

int Upa_PpsLog::pps_enable(void)
{
    pps_samples.clear();
    return 0;
}


Upa_PpsGui::Upa_PpsGui()
{
    pps_board = NULL;
    pps_index = 0;
    pps_name.clear();
    pps_show = 0;
    pps_max_diff = 0;
    pps_min_diff = 0;
    pps_avg_diff = 0;
    pps_samples.clear();
    pps_samples_chart.clear();
    pps_samples_histogram.clear();
    pps_samples_variance.clear();
    pps_samples_calc_variance.clear();
    pps_offset_chart_series = NULL;
}

Upa_PpsGui::~Upa_PpsGui()
{
    pps_board = NULL;
    pps_index = 0;
    pps_name.clear();
    pps_max_diff = 0;
    pps_min_diff = 0;
    pps_avg_diff = 0.0;
    pps_std_deviation = 0.0;
    pps_samples.clear();
    pps_samples_chart.clear();
    pps_samples_histogram.clear();
    pps_samples_variance.clear();
    pps_samples_calc_variance.clear();
    pps_offset_chart_series = NULL;
    pps_offset_histogram_series = NULL;
    pps_offset_variance_series = NULL;
}

int Upa_PpsGui::pps_disable(void)
{
    pps_max_diff = 0;
    pps_min_diff = 0;
    pps_avg_diff = 0.0;
    pps_std_deviation = 0.0;
    pps_samples.clear();
    pps_samples_chart.clear();
    pps_samples_histogram.clear();
    pps_samples_variance.clear();
    pps_samples_calc_variance.clear();
    pps_offset_chart_series->clear();
    pps_offset_histogram_series->clear();
    pps_offset_variance_series->clear();
    return 0;
}

int Upa_PpsGui::pps_enable(void)
{
    pps_max_diff = 0;
    pps_min_diff = 0;
    pps_avg_diff = 0.0;
    pps_std_deviation = 0.0;
    pps_samples.clear();
    pps_samples_chart.clear();
    pps_samples_histogram.clear();
    pps_samples_variance.clear();
    pps_samples_calc_variance.clear();
    pps_offset_chart_series->clear();
    pps_offset_histogram_series->clear();
    pps_offset_variance_series->clear();
    return 0;
}


Upa_PpsBoard::Upa_PpsBoard()
{
    com_port.clear();
    pps_ref_channel = 0; // default we take the reference
    for (int i = 0; i < Upa_PpsPerBoard; i++)
    {
        pps_ts[i].pps_board = this;
        pps_ctrl[i].pps_board = this;
        pps_log[i].pps_board = this;
        pps_gui[i].pps_board = this;
        pps_ts[i].pps_index = i;
        pps_ctrl[i].pps_index = i;
        pps_log[i].pps_index = i;
        pps_gui[i].pps_index = i;
    }
}

Upa_PpsBoard::~Upa_PpsBoard()
{
    com_port.clear();
    for (int i = 0; i < Upa_PpsPerBoard; i++)
    {
        pps_ts[i].pps_board = NULL;
        pps_ctrl[i].pps_board = NULL;
        pps_log[i].pps_board = NULL;
        pps_gui[i].pps_board = NULL;
    }
}

int Upa_PpsBoard::pps_disable(void)
{
    mutex_ctrl.lock();
    mutex_ctrl_threshold.lock();
    mutex_ctrl_log.lock();
    mutex_gui_chart.lock();
    mutex_gui_histogram.lock();
    mutex_gui_variance.lock();
    mutex_calc_variance.lock();
    mutex_ctrl_gui.lock();
    for (int i = 0; i < Upa_PpsPerBoard; i++)
    {
        pps_ts[i].pps_disable();
        pps_ctrl[i].pps_disable();
        pps_log[i].pps_disable();
        pps_gui[i].pps_disable();
    }
    mutex_ctrl_gui.unlock();
    mutex_calc_variance.unlock();
    mutex_gui_variance.unlock();
    mutex_gui_histogram.unlock();
    mutex_gui_chart.unlock();
    mutex_ctrl_log.unlock();
    mutex_ctrl_threshold.unlock();
    mutex_ctrl.unlock();
    return 0;
}

int Upa_PpsBoard::pps_enable(void)
{
    mutex_ctrl.lock();
    mutex_ctrl_threshold.lock();
    mutex_ctrl_log.lock();
    mutex_gui_chart.lock();
    mutex_gui_histogram.lock();
    mutex_gui_variance.lock();
    mutex_calc_variance.lock();
    mutex_ctrl_gui.lock();
    for (int i = 0; i < Upa_PpsPerBoard; i++)
    {
        pps_ts[i].pps_enable();
        pps_ctrl[i].pps_enable();
        pps_log[i].pps_enable();
        pps_gui[i].pps_enable();
    }
    mutex_ctrl_gui.unlock();
    mutex_calc_variance.unlock();
    mutex_gui_variance.unlock();
    mutex_gui_histogram.unlock();
    mutex_gui_chart.unlock();
    mutex_ctrl_log.unlock();
    mutex_ctrl_threshold.unlock();
    mutex_ctrl.unlock();
    return 0;
}


Upa_PpsTab::Upa_PpsTab(Upa_UniversalPpsAnalyzer *parent) : QWidget()
{
    QList<QString> temp_com_ports;
    unsigned int temp_data = 0;
    unsigned int temp_addr = 0;

    upa = parent;

    ui = new Ui::Upa_PpsTab();
    ui->setupUi(this);

    // timer and slots
    pps_ctrl_timer = new QTimer(this);
    pps_ctrl_timer->stop();
    pps_log_timer = new QTimer(this);
    pps_log_timer->setSingleShot(true);
    pps_log_timer->stop();
    pps_gui_timer_chart = new QTimer(this);
    pps_gui_timer_chart->setSingleShot(true);
    pps_gui_timer_chart->stop();
    pps_gui_timer_histogram = new QTimer(this);
    pps_gui_timer_histogram->setSingleShot(true);
    pps_gui_timer_histogram->stop();
    pps_gui_timer_variance = new QTimer(this);
    pps_gui_timer_variance->setSingleShot(true);
    pps_gui_timer_variance->stop();
    pps_calc_timer_variance = new QTimer(this);
    pps_calc_timer_variance->setSingleShot(true);
    pps_calc_timer_variance->stop();
    pps_threshold_timer = new QTimer(this);
    pps_threshold_timer->setSingleShot(true);
    pps_threshold_timer->stop();

    connect(ui->PpsClearButton, SIGNAL(clicked()), this, SLOT(pps_clear_button_clicked()));
    connect(ui->PpsSaveViewsButton, SIGNAL(clicked()), this, SLOT(pps_save_views_button_clicked()));
    connect(ui->PpsSaveValuesButton, SIGNAL(clicked()), this, SLOT(pps_save_values_button_clicked()));
    connect(ui->PpsLogButton, SIGNAL(clicked()), this, SLOT(pps_log_button_clicked()));
    connect(ui->PpsCompensateValuesButton, SIGNAL(clicked()), this, SLOT(pps_compensate_values_button_clicked()));
    connect(ui->PpsChangeDelaysButton, SIGNAL(clicked()), this, SLOT(pps_delay_button_clicked()));
    connect(ui->PpsSetThresholdsButton, SIGNAL(clicked()), this, SLOT(pps_threshold_button_clicked()));
    connect(ui->PpsZoomInButton, SIGNAL(clicked()), this, SLOT(pps_zoom_in_button_clicked()));
    connect(ui->PpsZoomOutButton, SIGNAL(clicked()), this, SLOT(pps_zoom_out_button_clicked()));
    connect(ui->PpsOffsetVarianceCalculateButton, SIGNAL(clicked()), this, SLOT(pps_calc_variance_button_clicked()));
    connect(ui->PpsOffsetVarianceAutoCalculateCheckBox, SIGNAL(clicked(bool)), this, SLOT(pps_auto_calc_variance_changed()));
    connect(pps_ctrl_timer, SIGNAL(timeout()), this, SLOT(pps_ctrl_timer_run()));
    connect(pps_log_timer, SIGNAL(timeout()), this, SLOT(pps_log_timer_run()));
    connect(pps_gui_timer_chart, SIGNAL(timeout()), this, SLOT(pps_gui_timer_chart_run()));
    connect(pps_gui_timer_histogram, SIGNAL(timeout()), this, SLOT(pps_gui_timer_histogram_run()));
    connect(pps_gui_timer_variance, SIGNAL(timeout()), this, SLOT(pps_gui_timer_variance_run()));
    connect(pps_calc_timer_variance, SIGNAL(timeout()), this, SLOT(pps_calc_timer_variance_run()));
    connect(pps_threshold_timer, SIGNAL(timeout()), this, SLOT(pps_threshold_timer_run()));

    // search for all different com ports
    for (int i = 0; i < upa->ts_core_config.size(); i++)
    {
        int j = 0;
        for (j = 0; j < temp_com_ports.size(); j++)
        {
            if (0 == temp_com_ports.at(j).compare(upa->ts_core_config.at(i).com_port))
            {
                break;
            }
        }
        if (j == temp_com_ports.size())
        {
            temp_com_ports.append(upa->ts_core_config.at(i).com_port);
        }
    }

    // create the boards and assign the com port name
    for (int i = 0; i < temp_com_ports.size(); i++)
    {
        Upa_PpsBoard* temp_board = new Upa_PpsBoard();
        temp_board->com_port.clear();
        temp_board->com_port = temp_com_ports.at(i);

        pps_boards.append(temp_board);
    }

    // connect the right timestampers of that board to the modules
    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            for (int k = 0; k < upa->ts_core_config.size(); k++)
            {
                if ((0 == pps_boards.at(i)->com_port.compare(upa->ts_core_config.at(k).com_port)) &&
                    ((j+1) == upa->ts_core_config.at(k).core_instance_nr))
                {
                    pps_boards.at(i)->pps_ts[j].ts_core_config = (Upa_CoreConfig*)&(upa->ts_core_config.at(k));
                }
            }
        }
    }

    // set the control part to 0
    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            if (j == 0) // this is the reference
            {
                pps_boards.at(i)->pps_ctrl[j].pps_delay = 0;
                pps_boards.at(i)->pps_ctrl[j].pps_threshold_enabled = 1;
                pps_boards.at(i)->pps_ctrl[j].pps_threshold_high = 50;
                pps_boards.at(i)->pps_ctrl[j].pps_threshold_low = -50;
            }
            else
            {
                pps_boards.at(i)->pps_ctrl[j].pps_delay = 0;
                pps_boards.at(i)->pps_ctrl[j].pps_threshold_enabled = 0;
                pps_boards.at(i)->pps_ctrl[j].pps_threshold_high = 1000;
                pps_boards.at(i)->pps_ctrl[j].pps_threshold_low = -1000;
            }
        }
    }

    // get versions
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->version = 0;
        for (int j = 0; j < upa->io_core_config.size(); j++)
        {
            if (pps_boards.at(i)->com_port == upa->io_core_config.at(j).com_port)
            {
                temp_addr = upa->io_core_config.at(j).address_range_low;
                if (0 == upa->io_core_config.at(j).com_lib->read_reg(temp_addr + Upa_IoConf_InputDataReg, temp_data))
                {
                    pps_boards.at(i)->version = temp_data;
                }
                else
                {
                    cout << "ERROR: " << "could not read Version from PPS Analyzer" << endl;
                }
            }
        }
    }

    //general part
    pps_compensate_values = 1;

    //log part
    pps_log_values = 0;

    // gui parts
    pps_zoom_factor = Upa_MinZoom;
    ui->PpsOffsetScrollBar->setMaximum(0);
    ui->PpsOffsetScrollBar->setMinimum((int)pps_zoom_factor-Upa_MaxSamples);
    ui->PpsOffsetScrollBar->setPageStep(pps_zoom_factor);
    ui->PpsOffsetScrollBar->setSingleStep(1);
    ui->PpsOffsetScrollBar->setSliderPosition(0);

    ui->PpsZoomInButton->setEnabled(false);
    ui->PpsZoomOutButton->setEnabled(true); // now we can zoom out for sure

    // offset chart
    pps_offset_chart = new QChart();
    pps_offset_chart->setContentsMargins(0, 0, 0, 0);
    pps_offset_chart->setBackgroundRoundness(0);
    pps_offset_chart->setBackgroundBrush(Qt::white);

    pps_offset_chart_x_axis = new QValueAxis;
    pps_offset_chart_x_axis->setLabelFormat("%i");
    pps_offset_chart_x_axis->setTickCount(21);
    pps_offset_chart_x_axis->setMin(0);
    pps_offset_chart_x_axis->setMax(Upa_MinZoom);
    pps_offset_chart->addAxis(pps_offset_chart_x_axis, Qt::AlignBottom);

    pps_offset_chart_y_axis = new QValueAxis;
    pps_offset_chart_y_axis->setLabelFormat("%i");
    pps_offset_chart_y_axis->setTickCount(11);
    pps_offset_chart_y_axis->setMin(-100);
    pps_offset_chart_y_axis->setMax(100);
    pps_offset_chart->addAxis(pps_offset_chart_y_axis, Qt::AlignLeft);

    // create the line series and initialize them
    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            QString temp_name;
            QLineSeries* temp_line_series = new QLineSeries();
            if (upa->use_open_gl == 1)
            {
                temp_line_series->setUseOpenGL(true);
            }
            QPen temp_pen;
            temp_pen.setWidth(2);
            temp_line_series->setPen(temp_pen);

            temp_name.clear();
            temp_name.append(pps_boards.at(i)->com_port);
            if (j == 0) // this is the reference
            {
                temp_name.append("_REF (");
                temp_name.append(pps_boards.at(i)->com_port);
                temp_name.append("_PPS_REF)");
                QColor temp_color;
                temp_color.setRgb(230, 230, 230);
                temp_line_series->setColor(temp_color);
            }
            else
            {
                temp_name.append("_PPS_");
                temp_name.append(QString::number(j));
                QColor temp_color;
                switch (j) // only 8 Input PPS per analyzer
                {
                    case 1:
                        temp_color.setRgb(255, 64, 64);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 2:
                        temp_color.setRgb(64, 255, 64);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 3:
                        temp_color.setRgb(64, 64, 255);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 4:
                        temp_color.setRgb(255, 64, 255);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 5:
                        temp_color.setRgb(160, 64, 64);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 6:
                        temp_color.setRgb(64, 160, 64);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 7:
                        temp_color.setRgb(64, 64, 160);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 8:
                        temp_color.setRgb(160, 64, 160);
                        temp_line_series->setColor(temp_color);
                        break;
                    default:
                        temp_line_series->setColor(Qt::black);
                        break;
                }
            }

            temp_line_series->setName(temp_name);

            pps_offset_chart->addSeries(temp_line_series);
            temp_line_series->attachAxis(pps_offset_chart_x_axis);
            temp_line_series->attachAxis(pps_offset_chart_y_axis);

            pps_boards.at(i)->pps_gui[j].pps_show = 1; // default show all
            pps_boards.at(i)->pps_gui[j].pps_name = temp_name;
            pps_boards.at(i)->pps_gui[j].pps_offset_chart_series = temp_line_series;
        }
    }

    pps_offset_chart_view = new QChartView(pps_offset_chart);
    pps_offset_chart_view->setRenderHint(QPainter::Antialiasing);

    ui->PpsOffsetChartLayout->addWidget(pps_offset_chart_view, 0);

    // offset histogram
    pps_offset_histogram = new QChart();
    pps_offset_histogram->setContentsMargins(0, 0, 0, 0);
    pps_offset_histogram->setBackgroundRoundness(0);
    pps_offset_histogram->setBackgroundBrush(Qt::white);

    pps_offset_histogram_x_axis = new QValueAxis;
    pps_offset_histogram_x_axis->setLabelFormat("%i");
    pps_offset_histogram_x_axis->setTickCount(21);
    pps_offset_histogram_x_axis->setMin(-Upa_MinZoom);
    pps_offset_histogram_x_axis->setMax(Upa_MinZoom);
    pps_offset_histogram->addAxis(pps_offset_histogram_x_axis, Qt::AlignBottom);

    pps_offset_histogram_y_axis = new QValueAxis;
    pps_offset_histogram_y_axis->setLabelFormat("%i");
    pps_offset_histogram_y_axis->setTickCount(11);
    pps_offset_histogram_y_axis->setMin(0);
    pps_offset_histogram_y_axis->setMax(10);
    pps_offset_histogram->addAxis(pps_offset_histogram_y_axis, Qt::AlignLeft);

    // create the line series and initialize them
    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            QString temp_name;
            QLineSeries* temp_line_series = new QLineSeries();
            if (upa->use_open_gl == 1)
            {
                temp_line_series->setUseOpenGL(true);
            }
            QPen temp_pen;
            temp_pen.setWidth(2);
            temp_line_series->setPen(temp_pen);

            temp_name.clear();
            temp_name.append(pps_boards.at(i)->com_port);
            if (j == 0) // this is the reference
            {
                temp_name.append("_REF (");
                temp_name.append(pps_boards.at(i)->com_port);
                temp_name.append("_PPS_REF)");
                QColor temp_color;
                temp_color.setRgb(230, 230, 230);
                temp_line_series->setColor(temp_color);
            }
            else
            {
                temp_name.append("_PPS_");
                temp_name.append(QString::number(j));
                QColor temp_color;
                switch (j) // only 8 Input PPS per analyzer
                {
                    case 1:
                        temp_color.setRgb(255, 64, 64);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 2:
                        temp_color.setRgb(64, 255, 64);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 3:
                        temp_color.setRgb(64, 64, 255);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 4:
                        temp_color.setRgb(255, 64, 255);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 5:
                        temp_color.setRgb(160, 64, 64);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 6:
                        temp_color.setRgb(64, 160, 64);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 7:
                        temp_color.setRgb(64, 64, 160);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 8:
                        temp_color.setRgb(160, 64, 160);
                        temp_line_series->setColor(temp_color);
                        break;
                    default:
                        temp_line_series->setColor(Qt::black);
                        break;
                }
            }

            temp_line_series->setName(temp_name);

            pps_offset_histogram->addSeries(temp_line_series);
            temp_line_series->attachAxis(pps_offset_histogram_x_axis);
            temp_line_series->attachAxis(pps_offset_histogram_y_axis);

            pps_boards.at(i)->pps_gui[j].pps_show = 1; // default show all
            pps_boards.at(i)->pps_gui[j].pps_name = temp_name;
            pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series = temp_line_series;
        }
    }

    pps_offset_histogram_view = new QChartView(pps_offset_histogram);
    pps_offset_histogram_view->setRenderHint(QPainter::Antialiasing);

    ui->PpsOffsetHistogramLayout->addWidget(pps_offset_histogram_view, 0);

    // offset variance
    pps_offset_variance = new QChart();
    pps_offset_variance->setContentsMargins(0, 0, 0, 0);
    pps_offset_variance->setBackgroundRoundness(0);
    pps_offset_variance->setBackgroundBrush(Qt::white);

    pps_offset_variance_x_axis = new QLogValueAxis ;
    pps_offset_variance_x_axis->setLabelFormat("%E");
    pps_offset_variance_x_axis->setBase(10.0);
    pps_offset_variance_x_axis->setMinorTickCount(-1);
    pps_offset_variance_x_axis->setMin(1.0);
    pps_offset_variance_x_axis->setMax(50.0);
    pps_offset_variance->addAxis(pps_offset_variance_x_axis, Qt::AlignBottom);

    pps_offset_variance_y_axis = new QLogValueAxis;
    pps_offset_variance_y_axis->setLabelFormat("%E");
    pps_offset_variance_y_axis->setBase(10.0);
    pps_offset_variance_y_axis->setMinorTickCount(-1);
    pps_offset_variance_y_axis->setMin(0.1);
    pps_offset_variance_y_axis->setMax(1.0);
    pps_offset_variance->addAxis(pps_offset_variance_y_axis, Qt::AlignLeft);

    pps_offset_variance_calc_time = 0;

    // create the line series and initialize them
    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            QString temp_name;
            QLineSeries* temp_line_series = new QLineSeries();
            if (upa->use_open_gl == 1)
            {
                temp_line_series->setUseOpenGL(true);
            }
            QPen temp_pen;
            temp_pen.setWidth(2);
            temp_line_series->setPen(temp_pen);

            temp_name.clear();
            temp_name.append(pps_boards.at(i)->com_port);
            if (j == 0) // this is the reference
            {
                temp_name.append("_REF (");
                temp_name.append(pps_boards.at(i)->com_port);
                temp_name.append("_PPS_REF)");
                QColor temp_color;
                temp_color.setRgb(230, 230, 230);
                temp_line_series->setColor(temp_color);
            }
            else
            {
                temp_name.append("_PPS_");
                temp_name.append(QString::number(j));
                QColor temp_color;
                switch (j) // only 8 Input PPS per analyzer
                {
                    case 1:
                        temp_color.setRgb(255, 64, 64);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 2:
                        temp_color.setRgb(64, 255, 64);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 3:
                        temp_color.setRgb(64, 64, 255);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 4:
                        temp_color.setRgb(255, 64, 255);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 5:
                        temp_color.setRgb(160, 64, 64);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 6:
                        temp_color.setRgb(64, 160, 64);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 7:
                        temp_color.setRgb(64, 64, 160);
                        temp_line_series->setColor(temp_color);
                        break;
                    case 8:
                        temp_color.setRgb(160, 64, 160);
                        temp_line_series->setColor(temp_color);
                        break;
                    default:
                        temp_line_series->setColor(Qt::black);
                        break;
                }
            }

            temp_line_series->setName(temp_name);

            pps_offset_variance->addSeries(temp_line_series);
            temp_line_series->attachAxis(pps_offset_variance_x_axis);
            temp_line_series->attachAxis(pps_offset_variance_y_axis);

            pps_boards.at(i)->pps_gui[j].pps_show = 1; // default show all
            pps_boards.at(i)->pps_gui[j].pps_name = temp_name;
            pps_boards.at(i)->pps_gui[j].pps_offset_variance_series = temp_line_series;
        }
    }

    pps_offset_variance_view = new QChartView(pps_offset_variance);
    pps_offset_variance_view->setRenderHint(QPainter::Antialiasing);

    ui->PpsOffsetVarianceLayout->addWidget(pps_offset_variance_view, 0);

    // disable all of the parts (set to a defined value)
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->pps_disable();
    }

    // get the configs
    ui_config = new Upa_PpsConfigScreen(this);
    ui_config->pps_reload();

    ui_threshold = new Upa_PpsThresholdScreen(this);
    ui_threshold->pps_reload();

    // show PPS Chart
    ui->Upa_PpsGuiTab->setCurrentIndex(0);

    // auto calculate varoance off
    ui->PpsOffsetVarianceAutoCalculateCheckBox->setChecked(false);
    ui->PpsOffsetVarianceCalculateButton->setEnabled(true);
    ui->PpsOffsetVarianceLogStepsCheckBox->setChecked(false);

    pps_offset_chart->legend()->setVisible(true);
    pps_offset_histogram->legend()->setVisible(true);
    pps_offset_variance->legend()->setVisible(true);
}

Upa_PpsTab::~Upa_PpsTab()
{
    pps_ctrl_timer->stop();
    pps_log_timer->stop();
    pps_gui_timer_chart->stop();
    pps_gui_timer_histogram->stop();
    pps_gui_timer_variance->stop();
    pps_calc_timer_variance->stop();
    pps_threshold_timer->stop();

    // stop loging
    if (pps_log_values != 0)
    {
        pps_log_values = 0;
        pps_log_file.close();
        ui->PpsLogButton->setText("Start Log");
    }

    // disable
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->pps_disable();
    }

    // remove gui parts
    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            delete pps_boards.at(i)->pps_gui[j].pps_offset_chart_series;
            delete pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series;
            delete pps_boards.at(i)->pps_gui[j].pps_offset_variance_series;
        }
    }
    delete pps_offset_chart;
    delete pps_offset_chart_x_axis;
    delete pps_offset_chart_y_axis;
    delete pps_offset_histogram;
    delete pps_offset_histogram_x_axis;
    delete pps_offset_histogram_y_axis;
    delete pps_offset_variance;
    delete pps_offset_variance_x_axis;
    delete pps_offset_variance_y_axis;
    delete ui;
    ui_config->close();
    delete ui_config;
    ui_threshold->close();
    delete ui_threshold;

    // remove timers
    delete pps_ctrl_timer;
    delete pps_log_timer;
    delete pps_gui_timer_chart;
    delete pps_gui_timer_histogram;
    delete pps_gui_timer_variance;
    delete pps_calc_timer_variance;
    delete pps_threshold_timer;

    for (int i = 0; i < pps_boards.size(); i++)
    {
        delete pps_boards.at(i);
    }
    pps_boards.clear();

    delete pps_offset_chart_view;
    delete pps_offset_histogram_view;
    delete pps_offset_variance_view;
}

int Upa_PpsTab::pps_resize(int height, int width)
{
    int height_delta = 0;
    int width_delta = 0;

    height_delta = (height-Upa_MainHeight);
    width_delta = (width-Upa_MainWidth);

    ui->Upa_PpsGuiTab->setFixedHeight(700+height_delta);
    ui->Upa_PpsGuiTab->setFixedWidth(1120+width_delta);

    ui->PpsOffsetChartValue->setFixedHeight(650+height_delta);
    ui->PpsOffsetChartValue->setFixedWidth(1114+width_delta);
    ui->PpsOffsetChartLabel->setFixedWidth(1114+width_delta);

    ui->PpsOffsetHistogramValue->setFixedHeight(650+height_delta);
    ui->PpsOffsetHistogramValue->setFixedWidth(1114+width_delta);
    ui->PpsOffsetHistogramLabel->setFixedWidth(1114+width_delta);

    ui->PpsOffsetVarianceValue->setFixedHeight(650+height_delta);
    ui->PpsOffsetVarianceValue->setFixedWidth(1114+width_delta);
    ui->PpsOffsetVarianceLabel->setFixedWidth(1114+width_delta);

    ui->PpsOffsetScrollBar->move(0, 650+height_delta);
    ui->PpsOffsetScrollBar->setFixedWidth(1114+width_delta);

    ui->PpsStatisticsValue->setFixedHeight(680+height_delta);

    updateGeometry();

    return 0;
}

int Upa_PpsTab::pps_enable(void)
{
    // enable
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->pps_enable();
    }

    pps_ctrl_timer->start(1000);

    return 0;
}

int Upa_PpsTab::pps_disable(void)
{
    pps_ctrl_timer->stop();
    pps_log_timer->stop();
    pps_gui_timer_chart->stop();
    pps_gui_timer_histogram->stop();
    pps_gui_timer_variance->stop();
    pps_calc_timer_variance->stop();
    pps_threshold_timer->stop();

    if (pps_log_values != 0)
    {
        pps_log_values = 0;
        pps_log_file.close();
        ui->PpsLogButton->setText("Start Log");
    }

    // disable
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->pps_disable();
    }

    return 0;
}

int Upa_PpsTab::pps_fetch(Upa_PpsTs* pps_ts)
{
    unsigned int temp_data = 0;
    unsigned int temp_addr = 0;

    //********************************
    // get timestamp
    //********************************
    temp_addr = pps_ts->ts_core_config->address_range_low;

    pps_ts->pps_sample.pps_fetch_time = QDateTime::currentDateTime();

    if (0 == pps_ts->ts_core_config->com_lib->read_reg(temp_addr + Upa_ClkTs_StatusReg, temp_data)) // check if the PPS has activity at all (within the last 1.5s)
    {
        if ((temp_data & 0x00000002) != 0)
        {
            pps_ts->pps_sample.pps_active = 1;

            // nanoseconds
            if (0 == pps_ts->ts_core_config->com_lib->read_reg(temp_addr + Upa_ClkTs_TimeValueLReg, temp_data)) // check if the PPS has activity at all (within the last 1.5s)
            {
                if(temp_data > 500000000)
                {
                    pps_ts->pps_sample.pps_offset = (int)temp_data - 1000000000;
                }
                else
                {
                    pps_ts->pps_sample.pps_offset = (int)temp_data;
                }
                return 0;
            }
            else
            {
                pps_ts->pps_sample.pps_active = 0;
                pps_ts->pps_sample.pps_offset = 0;
                cout << "ERROR: " << "could not read sample" << endl;
                return -1;
            }
        }
        else
        {
            pps_ts->pps_sample.pps_active = 0;
            pps_ts->pps_sample.pps_offset = 0;
        }
    }
    else
    {
        pps_ts->pps_sample.pps_active = 0;
        pps_ts->pps_sample.pps_offset = 0;
        cout << "ERROR: " << "could not read sample" << endl;
        return -1;
    }
    return -1;
}


void Upa_PpsTab::pps_clear_button_clicked(void)
{
    // lock all passing of samples
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_gui_chart.lock();
        pps_boards.at(i)->mutex_gui_histogram.lock();
        pps_boards.at(i)->mutex_gui_variance.lock();
        pps_boards.at(i)->mutex_calc_variance.lock();
    }
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_ctrl_gui.lock();
    }

    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            pps_boards.at(i)->pps_gui[j].pps_samples.clear();
            pps_boards.at(i)->pps_gui[j].pps_samples_chart.clear();
            pps_boards.at(i)->pps_gui[j].pps_samples_histogram.clear();
            pps_boards.at(i)->pps_gui[j].pps_samples_variance.clear();
            pps_boards.at(i)->pps_gui[j].pps_samples_calc_variance.clear();
            pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->clear();
            pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->clear();
            pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->clear();
        }
    }

    pps_offset_chart->axisY()->setMin(-100);
    pps_offset_chart->axisY()->setMax(100);

    pps_offset_histogram->axisY()->setMin(0);
    pps_offset_histogram->axisY()->setMax(10);

    pps_offset_variance->axisX()->setMin(1);
    pps_offset_variance->axisX()->setMax(2);
    pps_offset_variance->axisY()->setMin(0.1);
    pps_offset_variance->axisY()->setMax(1);

    pps_offset_variance_calc_time = 0;

    ui->PpsOffsetScrollBar->setSliderPosition(0);

    // unlock all passing of samples
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_ctrl_gui.unlock();
    }
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_calc_variance.unlock();
        pps_boards.at(i)->mutex_gui_variance.unlock();
        pps_boards.at(i)->mutex_gui_histogram.unlock();
        pps_boards.at(i)->mutex_gui_chart.unlock();
    }

    pps_offset_chart->show();
    pps_offset_histogram->show();
    pps_offset_variance->show();
}

void Upa_PpsTab::pps_save_views_button_clicked(void)
{
    QDateTime temp_time = QDateTime::currentDateTime();
    QString temp_string = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0);
    temp_string.append("/pps_chart_");
    temp_string.append(temp_time.toString("dd-MM-yyyy_hh-mm-ss"));

    QString temp_filename = QFileDialog::getSaveFileName(this, "save chart, histogram and variance (if calculated)", temp_string, "PNG(*.png);; TIFF(*.tiff *.tif);; JPEG(*.jpg *.jpeg)");

     // lock all passing of samples
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_gui_chart.lock();
        pps_boards.at(i)->mutex_gui_histogram.lock();
        pps_boards.at(i)->mutex_gui_variance.lock();
    }

    pps_gui_timer_chart->stop();
    pps_gui_timer_histogram->stop();
    pps_gui_timer_variance->stop();
    QString temp_filename_chart = temp_filename;
    temp_filename_chart.insert(temp_filename_chart.lastIndexOf("."), "_chart");
    QString temp_filename_histogram = temp_filename;
    temp_filename_histogram.insert(temp_filename_histogram.lastIndexOf("."), "_histogram");
    QString temp_filename_variance = temp_filename;
    temp_filename_variance.insert(temp_filename_variance.lastIndexOf("."), "_variance");

    QPixmap temp_logo = QPixmap(":/HelpMenu/NetTimeLogic.png");
    temp_logo = temp_logo.scaledToHeight(30);
    QPixmap temp_chart = ui->PpsOffsetChartValue->grab();
    QPixmap temp_histogram = ui->PpsOffsetHistogramValue->grab();
    QPixmap temp_variance = ui->PpsOffsetVarianceValue->grab();

    QPainter temp_painter_chart(&temp_chart);
    temp_painter_chart.drawImage(8, 8, temp_logo.toImage());
    QPainter temp_painter_histogram(&temp_histogram);
    temp_painter_histogram.drawImage(8, 8, temp_logo.toImage());
    QPainter temp_painter_variance(&temp_variance);
    temp_painter_variance.drawImage(8, 8, temp_logo.toImage());

    temp_chart.save(temp_filename_chart);
    temp_histogram.save(temp_filename_histogram);
    if (ui->OffsetVariance->isVisible() == true)
    {
        temp_variance.save(temp_filename_variance);
    }

    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_gui_variance.unlock();
        pps_boards.at(i)->mutex_gui_histogram.unlock();
        pps_boards.at(i)->mutex_gui_chart.unlock();
    }
}

void Upa_PpsTab::pps_log_button_clicked(void)
{
    QDateTime temp_time = QDateTime::currentDateTime();
    QString temp_string = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0);
    temp_string.append("/pps_log_");
    temp_string.append(temp_time.toString("dd-MM-yyyy_hh-mm-ss"));

    if (ui->PpsLogButton->text() == "Start Log")
    {
        QString temp_filename = QFileDialog::getSaveFileName(this, "save log", temp_string, "CSV(*.csv)");

        if (temp_filename != "")
        {
            pps_log_file.setFileName(temp_filename);
            if (true == pps_log_file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream temp_stream(&pps_log_file);

                temp_stream << "sep=;\n";
                temp_stream << "Timestamp";
                for (int i = 0; i < pps_boards.size(); i++)
                {
                    for (int j = 0; j < Upa_PpsPerBoard; j++)
                    {
                        temp_stream << ";" << pps_boards.at(i)->pps_gui[j].pps_name;
                    }
                }
                for (int i = 0; i < pps_boards.size(); i++)
                {
                    for (int j = 0; j < Upa_PpsPerBoard; j++)
                    {
                        temp_stream << ";" << pps_boards.at(i)->pps_gui[j].pps_name << " threshold high ("  << QString::number(pps_boards.at(i)->pps_ctrl[j].pps_threshold_high) << ") exceeded";
                        temp_stream << ";" << pps_boards.at(i)->pps_gui[j].pps_name << " threshold low ("  << QString::number(pps_boards.at(i)->pps_ctrl[j].pps_threshold_low) << ") exceeded";
                    }
                }
                temp_stream << "\n";

                pps_log_values = 1;
                ui->PpsLogButton->setText("Stop Log");
            }
            else
            {
                pps_log_values = 0;
                pps_log_file.setFileName("");
                cout << "ERROR: " << "could not open log file" << endl;
            }
        }
        else
        {
            pps_log_values = 0;
            cout << "ERROR: " << "empty log file name" << endl;
        }
    }
    else
    {
        QMessageBox temp_msg_box;
        temp_msg_box.setText("Do you realy want to stop logging?");
        temp_msg_box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        temp_msg_box.setDefaultButton(QMessageBox::Cancel);

        int ret = temp_msg_box.exec();
        switch (ret)
        {
            case QMessageBox::Ok:
                pps_log_values = 0;
                pps_log_file.close();
                ui->PpsLogButton->setText("Start Log");
                break;

            default:
                break;
        }
    }
}

void Upa_PpsTab::pps_save_values_button_clicked(void)
{
    QFile temp_file;
    Upa_PpsLogSample temp_log_sample;
    QDateTime temp_time = QDateTime::currentDateTime();
    QString temp_string = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0);
    temp_string.append("/pps_values_");
    temp_string.append(temp_time.toString("dd-MM-yyyy_hh-mm-ss"));

    QString temp_filename = QFileDialog::getSaveFileName(this, "save values", temp_string, "CSV(*.csv)");

    if (temp_filename != "")
    {
        temp_file.setFileName(temp_filename);
        if (true == temp_file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream temp_stream(&temp_file);

            temp_stream << "sep=;\n";
            temp_stream << "Timestamp";
            for (int i = 0; i < pps_boards.size(); i++)
            {
                for (int j = 0; j < Upa_PpsPerBoard; j++)
                {
                    temp_stream << ";" << pps_boards.at(i)->pps_gui[j].pps_name;
                }
            }
            for (int i = 0; i < pps_boards.size(); i++)
            {
                for (int j = 0; j < Upa_PpsPerBoard; j++)
                {
                    temp_stream << ";" << pps_boards.at(i)->pps_gui[j].pps_name << " threshold high ("  << QString::number(pps_boards.at(i)->pps_ctrl[j].pps_threshold_high) << ") exceeded";
                    temp_stream << ";" << pps_boards.at(i)->pps_gui[j].pps_name << " threshold low ("  << QString::number(pps_boards.at(i)->pps_ctrl[j].pps_threshold_low) << ") exceeded";
                }
            }
            temp_stream << "\n";

            // lock log
            for (int i = 0; i < pps_boards.size(); i++)
            {
                pps_boards.at(i)->mutex_ctrl.lock();
            }

            temp_string.clear();
            for (int k = 0; k < pps_boards.at(0)->pps_ctrl[0].pps_samples.size(); k++)
            {
                for (int i = 0; i < pps_boards.size(); i++)
                {
                    for (int j = 0; j < Upa_PpsPerBoard; j++)
                    {
                        temp_log_sample = pps_boards.at(i)->pps_ctrl[j].pps_samples.at(k);
                        if ((i==0) && (j==0))
                        {
                            temp_string.append(temp_log_sample.pps_fetch_time.toString("dd-MM-yyyy hh:mm:ss"));
                        }
                        temp_string.append(";");
                        if (temp_log_sample.pps_active != 0)
                        {
                            temp_string.append(QString::number(temp_log_sample.pps_offset));
                        }
                        else
                        {
                            temp_string.append("-");
                        }
                    }
                }

                for (int i = 0; i < pps_boards.size(); i++)
                {
                    for (int j = 0; j < Upa_PpsPerBoard; j++)
                    {
                        temp_log_sample = pps_boards.at(i)->pps_ctrl[j].pps_samples.at(k);
                        if ((temp_log_sample.pps_active != 0) &&
                            (temp_log_sample.pps_threshold_enabled != 0))
                        {
                            temp_string.append(";");
                            if (temp_log_sample.pps_threshold_high_exceeded == 0)
                            {
                                temp_string.append("n");
                            }
                            else
                            {
                                temp_string.append("y");
                            }
                            temp_string.append(";");
                            if (temp_log_sample.pps_threshold_low_exceeded == 0)
                            {
                                temp_string.append("n");
                            }
                            else
                            {
                                temp_string.append("y");
                            }
                        }
                        else
                        {
                            temp_string.append(";");
                            temp_string.append("-");
                            temp_string.append(";");
                            temp_string.append("-");
                        }
                    }
                }

                temp_string.append("\n");
            }

            // lock log
            for (int i = 0; i < pps_boards.size(); i++)
            {
                pps_boards.at(i)->mutex_ctrl.unlock();
            }

            // now write out the file
            temp_stream << temp_string;


            temp_file.close();

        }
        else
        {
            temp_file.setFileName("");
            cout << "ERROR: " << "could not open values file" << endl;
        }
    }
    else
    {
        cout << "ERROR: " << "empty values file name" << endl;
    }
}

void Upa_PpsTab::pps_compensate_values_button_clicked(void)
{
    if (ui->PpsCompensateValuesButton->text() == "Raw Values")
    {
        pps_compensate_values = 0;
        ui->PpsCompensateValuesButton->setText("Compensate Values");
    }
    else
    {
        pps_compensate_values = 1;
        ui->PpsCompensateValuesButton->setText("Raw Values");
    }
}

void Upa_PpsTab::pps_delay_button_clicked(void)
{
    ui_config->pps_reload();
    ui_config->setModal(true);
    ui_config->exec();
    //ui_config->show();
    //ui_config->activateWindow();
    //ui_config->raise();
}

void Upa_PpsTab::pps_threshold_button_clicked(void)
{
    ui_threshold->pps_reload();
    ui_threshold->setModal(true);
    ui_threshold->exec();
    //ui_threshold->pps_reload();
    //ui_threshold->show();
    //ui_threshold->activateWindow();
    //ui_threshold->raise();
}


void Upa_PpsTab::pps_ctrl_timer_run(void)
{
    QDateTime temp_time;
    int temp_offset_ref;
    int temp_offset;
    int temp_active;
    int temp_threshold_high_exceeded;
    int temp_threshold_low_exceeded;
    Upa_PpsGuiSample temp_gui_sample;
    Upa_PpsLogSample temp_log_sample;

    // just take a snapshot
    temp_time = QDateTime::currentDateTime();

    //********************************
    // fetch all timestamps from all boards
    //********************************
    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            pps_fetch(&(pps_boards.at(i)->pps_ts[j]));
        }
    }

    //********************************
    // calculations and forwarding
    //********************************
    // lock all passing of samples
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_ctrl.lock();
    }
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_ctrl_threshold.lock();
    }
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_ctrl_gui.lock();
    }
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_ctrl_log.lock();
    }


    // calculate and push things
    for (int i = 0; i < pps_boards.size(); i++)
    {
        if (pps_boards.at(i)->pps_ts[pps_boards.at(i)->pps_ref_channel].pps_sample.pps_active != 0)
        {
            temp_offset_ref = pps_boards.at(i)->pps_ts[pps_boards.at(i)->pps_ref_channel].pps_sample.pps_offset;
            temp_offset_ref += pps_boards.at(i)->pps_ctrl[pps_boards.at(i)->pps_ref_channel].pps_delay;
        }
        else
        {
            temp_offset_ref = 0;
        }
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            if (pps_boards.at(i)->pps_ts[j].pps_sample.pps_active != 0)
            {
                temp_active = 1;
                if (j == 0)
                {
                    temp_offset = pps_boards.at(i)->pps_ts[0].pps_sample.pps_offset;
                    temp_offset += pps_boards.at(i)->pps_ctrl[pps_boards.at(i)->pps_ref_channel].pps_delay;
                }
                else if (j == pps_boards.at(i)->pps_ref_channel)
                {
                    temp_offset = 0;
                }
                else
                {
                    temp_offset = pps_boards.at(i)->pps_ts[j].pps_sample.pps_offset;
                    temp_offset -= pps_boards.at(i)->pps_ctrl[j].pps_delay;
                    if (pps_compensate_values != 0)
                    {
                        temp_offset -= temp_offset_ref;
                    }
                }

                if (pps_boards.at(i)->pps_ctrl[j].pps_threshold_enabled != 0)
                {
                    if (temp_offset > pps_boards.at(i)->pps_ctrl[j].pps_threshold_high)
                    {
                        temp_threshold_high_exceeded = 1;
                    }
                    else
                    {
                        temp_threshold_high_exceeded = 0;
                    }
                    if (temp_offset < pps_boards.at(i)->pps_ctrl[j].pps_threshold_low)
                    {
                        temp_threshold_low_exceeded = 1;
                    }
                    else
                    {
                        temp_threshold_low_exceeded = 0;
                    }
                }
                else
                {
                    temp_threshold_high_exceeded = 0;
                    temp_threshold_low_exceeded = 0;
                }

            }
            else
            {
                temp_active = 0;
                temp_offset = 0;
                temp_threshold_high_exceeded = 0;
                temp_threshold_low_exceeded = 0;
            }

            // fill
            temp_gui_sample.pps_active = temp_active;
            temp_gui_sample.pps_offset = temp_offset;


            // fill the array
            while (pps_boards.at(i)->pps_gui[j].pps_samples.size() >= Upa_MaxSamples) // only the last N samples
            {
                pps_boards.at(i)->pps_gui[j].pps_samples.removeFirst();
            }
            pps_boards.at(i)->pps_gui[j].pps_samples.append(temp_gui_sample);

            // fill
            temp_log_sample.pps_active = temp_active;
            temp_log_sample.pps_offset = temp_offset;
            temp_log_sample.pps_fetch_time = temp_time; // just take when we started
            temp_log_sample.pps_threshold_enabled = pps_boards.at(i)->pps_ctrl[j].pps_threshold_enabled;
            temp_log_sample.pps_threshold_high = pps_boards.at(i)->pps_ctrl[j].pps_threshold_high;
            temp_log_sample.pps_threshold_low = pps_boards.at(i)->pps_ctrl[j].pps_threshold_low;
            temp_log_sample.pps_threshold_high_exceeded = temp_threshold_high_exceeded;
            temp_log_sample.pps_threshold_low_exceeded  = temp_threshold_low_exceeded;

            if (pps_log_values != 0)
            {
                // push the offset to the Log
                pps_boards.at(i)->pps_log[j].pps_samples.append(temp_log_sample);
            }
            else
            {
                pps_boards.at(i)->pps_log[j].pps_samples.clear();
            }

            // fill the array
            while (pps_boards.at(i)->pps_ctrl[j].pps_samples.size() >= Upa_MaxSamples) // only the last N samples
            {
                pps_boards.at(i)->pps_ctrl[j].pps_samples.removeFirst();
            }
            // save
            pps_boards.at(i)->pps_ctrl[j].pps_samples.append(temp_log_sample);

            // push the threshold
            pps_boards.at(i)->pps_ctrl[j].pps_threshold_high_exceeded = temp_threshold_high_exceeded;
            pps_boards.at(i)->pps_ctrl[j].pps_threshold_low_exceeded = temp_threshold_low_exceeded;
        }
    }

    // unlock all passing of samples
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_ctrl_log.unlock();
    }

    if (ui->PpsOffsetScrollBar->sliderPosition() > ui->PpsOffsetScrollBar->minimum())
    {
        if (ui->PpsOffsetScrollBar->sliderPosition() != 0)
        {
            ui->PpsOffsetScrollBar->setSliderPosition(ui->PpsOffsetScrollBar->sliderPosition()-1);
        }
    }
    else
    {
        ui->PpsOffsetScrollBar->setSliderPosition(ui->PpsOffsetScrollBar->minimum());
    }

    QFuture<void> gui_chart = QtConcurrent::run(pps_gui_chart_sample, this);
    QFuture<void> gui_histogram = QtConcurrent::run(pps_gui_histogram_sample, this);
    QFuture<void> gui_variance = QtConcurrent::run(pps_gui_variance_sample, this);

    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_ctrl_gui.unlock();
    }
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_ctrl_threshold.unlock();
    }
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_ctrl.unlock();
    }

    //********************************
    // wake up others
    //********************************
    if (pps_log_values != 0)
    {
        if (false == pps_log_timer->isActive())
        {
            pps_log_timer->start(1);
        }
    }
    if (false == pps_gui_timer_chart->isActive())
    {
        pps_gui_timer_chart->start(1);
    }
    if (false == pps_gui_timer_histogram->isActive())
    {
        pps_gui_timer_histogram->start(1);
    }

    if ((ui->PpsOffsetVarianceAutoCalculateCheckBox->isChecked() == true) &&
        (ui->OffsetVariance->isVisible() == true))
    {
        if (false == pps_calc_timer_variance->isActive())
        {
            pps_calc_timer_variance->start(1); // only every second sample
        }
    }
    if (false == pps_threshold_timer->isActive())
    {
        pps_threshold_timer->start(1);
    }
}


void Upa_PpsTab::pps_log_timer_run(void)
{
    QString temp_string;
    Upa_PpsLogSample temp_log_sample;

    // lock log
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_ctrl_log.lock();
    }

    if (pps_log_values != 0)
    {
        temp_string.clear();
        while (pps_boards.at(0)->pps_log[0].pps_samples.size() > 0)
        {
            for (int i = 0; i < pps_boards.size(); i++)
            {
                for (int j = 0; j < Upa_PpsPerBoard; j++)
                {
                    temp_log_sample = pps_boards.at(i)->pps_log[j].pps_samples.first();
                    if ((i==0) && (j==0))
                    {
                        temp_string.append(temp_log_sample.pps_fetch_time.toString("dd-MM-yyyy hh:mm:ss"));
                    }
                    temp_string.append(";");
                    if (temp_log_sample.pps_active != 0)
                    {
                        temp_string.append(QString::number(temp_log_sample.pps_offset));
                    }
                    else
                    {
                        temp_string.append("-");
                    }
                }
            }

            for (int i = 0; i < pps_boards.size(); i++)
            {
                for (int j = 0; j < Upa_PpsPerBoard; j++)
                {
                    temp_log_sample = pps_boards.at(i)->pps_log[j].pps_samples.first();
                    if ((temp_log_sample.pps_active != 0) &&
                        (temp_log_sample.pps_threshold_enabled != 0))
                    {
                        temp_string.append(";");
                        if (temp_log_sample.pps_threshold_high_exceeded == 0)
                        {
                            temp_string.append("n");
                        }
                        else
                        {
                            temp_string.append("y");
                        }
                        temp_string.append(";");
                        if (temp_log_sample.pps_threshold_low_exceeded == 0)
                        {
                            temp_string.append("n");
                        }
                        else
                        {
                            temp_string.append("y");
                        }
                    }
                    else
                    {
                        temp_string.append(";");
                        temp_string.append("-");
                        temp_string.append(";");
                        temp_string.append("-");
                    }
                }
            }

            temp_string.append("\n");

            for (int i = 0; i < pps_boards.size(); i++)
            {
                for (int j = 0; j < Upa_PpsPerBoard; j++)
                {
                    pps_boards.at(i)->pps_log[j].pps_samples.removeFirst();
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < pps_boards.size(); i++)
        {
            for (int j = 0; j < Upa_PpsPerBoard; j++)
            {
                pps_boards.at(i)->pps_log[j].pps_samples.clear();
            }
        }
    }

    // lock log
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_ctrl_log.unlock();
    }

    if ((pps_log_values != 0) && (temp_string.size() != 0))
    {
        QTextStream temp_stream(&pps_log_file);
        temp_stream << temp_string;
    }

}

static void pps_gui_chart_sample(Upa_PpsTab* upa_pps)
{
    int temp_size;
    QColor temp_color;
    int temp_range_max;

    // lock gui
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_gui_chart.lock();
    }

    temp_range_max = upa_pps->ui->PpsOffsetScrollBar->sliderPosition();

    // lock all passing of samples
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_ctrl_gui.lock();
    }

    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples_chart.clear();

            if ((j == 0) || (upa_pps->pps_boards.at(i)->pps_gui[j].pps_show != 0))
            {
                temp_size = upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples.size();
                for (int k = upa_pps->pps_zoom_factor; k >= 0 ; k--)
                {
                    int temp_index = temp_size - 1 + temp_range_max - k;
                    if ((temp_index >= 0) && (temp_index < temp_size))
                    {
                        upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples_chart.append(upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples.at(temp_index));
                    }
                }
            }
        }
    }

    // unlock all passing of samples
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_ctrl_gui.unlock();
    }

    // unlock gui
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_gui_chart.unlock();
    }
}

void Upa_PpsTab::pps_gui_timer_chart_run(void)
{
    int temp_size;
    QColor temp_color;
    int temp_min;
    int temp_max;
    int temp_range_min;
    int temp_range_max;

    // lock gui
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_gui_chart.lock();
    }

    // disable updates
    pps_offset_chart_view->setUpdatesEnabled(false);

    //remove all Series
    foreach (QAbstractSeries *i, pps_offset_chart->series())
    {
        pps_offset_chart->removeSeries(i);
    }

    // disable updates
    pps_offset_histogram_view->setUpdatesEnabled(false);

    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->clear();
        }
    }

    temp_range_min = ui->PpsOffsetScrollBar->sliderPosition()-pps_zoom_factor;
    temp_range_max = ui->PpsOffsetScrollBar->sliderPosition();
    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->setName(pps_boards.at(i)->pps_gui[j].pps_name);

            if ((j == 0) || (pps_boards.at(i)->pps_gui[j].pps_show != 0))
            {
                temp_size = pps_boards.at(i)->pps_gui[j].pps_samples_chart.size();
                for (int k = 0; k < temp_size ; k++)
                {
                    if (pps_boards.at(i)->pps_gui[j].pps_samples_chart.at(k).pps_active != 0)
                    {
                        pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->append((temp_range_max-temp_size+1+k), pps_boards.at(i)->pps_gui[j].pps_samples_chart.at(k).pps_offset);
                    }
                }
                if (pps_boards.at(i)->pps_gui[j].pps_show != 0)
                {
                    pps_offset_chart->addSeries(pps_boards.at(i)->pps_gui[j].pps_offset_chart_series);
                }
            }

            if (j != 0)
            {
                if (pps_boards.at(i)->pps_gui[j].pps_samples_chart.isEmpty() ||
                    pps_boards.at(i)->pps_gui[j].pps_samples_chart.last().pps_active == 0)
                {
                    temp_color.setRgb(128, 128, 128);
                    pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->setColor(temp_color);
                }
                else
                {
                    switch (j) // only 8 Input PPS per analyzer
                    {
                        case 1:
                            temp_color.setRgb(255, 64, 64);
                            pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->setColor(temp_color);
                            break;
                        case 2:
                            temp_color.setRgb(64, 255, 64);
                            pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->setColor(temp_color);
                            break;
                        case 3:
                            temp_color.setRgb(64, 64, 255);
                            pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->setColor(temp_color);
                            break;
                        case 4:
                            temp_color.setRgb(255, 64, 255);
                            pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->setColor(temp_color);
                            break;
                        case 5:
                            temp_color.setRgb(160, 64, 64);
                            pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->setColor(temp_color);
                            break;
                        case 6:
                            temp_color.setRgb(64, 160, 64);
                            pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->setColor(temp_color);
                            break;
                        case 7:
                            temp_color.setRgb(64, 64, 160);
                            pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->setColor(temp_color);
                            break;
                        case 8:
                            temp_color.setRgb(160, 64, 160);
                            pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->setColor(temp_color);
                            break;
                        default:
                            pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->setColor(Qt::black);
                            break;
                    }
                }
            }
        }
    }

    // axis
    pps_offset_chart->removeAxis(pps_offset_chart_x_axis);
    pps_offset_chart_x_axis->setLabelFormat("%i");
    pps_offset_chart_x_axis->setTickCount(21);
    pps_offset_chart_x_axis->setMin(temp_range_min);
    pps_offset_chart_x_axis->setMax(temp_range_max);
    pps_offset_chart->addAxis(pps_offset_chart_x_axis, Qt::AlignBottom);
    foreach (QAbstractSeries *i, pps_offset_chart->series())
    {
        i->attachAxis(pps_offset_chart_x_axis);
    }

    // statistics
    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            if (pps_boards.at(i)->pps_gui[j].pps_show != 0)
            {
                pps_boards.at(i)->pps_gui[j].pps_min_diff = 1000000000;
                pps_boards.at(i)->pps_gui[j].pps_max_diff = -1000000000;
                pps_boards.at(i)->pps_gui[j].pps_avg_diff = 0.0;
                pps_boards.at(i)->pps_gui[j].pps_std_deviation = 0.0;

                for (int k = 0; k < pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->count(); k++)
                {
                    QPointF temp_point = pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->at(k);
                    if (pps_boards.at(i)->pps_gui[j].pps_min_diff > temp_point.y())
                    {
                        pps_boards.at(i)->pps_gui[j].pps_min_diff = temp_point.y();
                    }
                    if (pps_boards.at(i)->pps_gui[j].pps_max_diff < temp_point.y())
                    {
                        pps_boards.at(i)->pps_gui[j].pps_max_diff = temp_point.y();
                    }
                    pps_boards.at(i)->pps_gui[j].pps_avg_diff += temp_point.y();
                }

                pps_boards.at(i)->pps_gui[j].pps_avg_diff /= pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->count();

                for (int k = 0; k < pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->count(); k++)
                {
                    QPointF temp_point = pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->at(k);
                    pps_boards.at(i)->pps_gui[j].pps_std_deviation += pow((((double)temp_point.y()) - pps_boards.at(i)->pps_gui[j].pps_avg_diff), 2);
                }
                pps_boards.at(i)->pps_gui[j].pps_std_deviation /= (pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->count() - 1);
                pps_boards.at(i)->pps_gui[j].pps_std_deviation = sqrt(pps_boards.at(i)->pps_gui[j].pps_std_deviation);
            }
        }
    }

    // min and max for display
    if (ui->PpsFixedScaleCheckBox->isChecked() == false)
    {
        temp_min = 0;
        temp_max = 0;

        for (int i = 0; i < pps_boards.size(); i++)
        {
            for (int j = 0; j < Upa_PpsPerBoard; j++)
            {
                if (pps_boards.at(i)->pps_gui[j].pps_show != 0)
                {
                    if (temp_min > pps_boards.at(i)->pps_gui[j].pps_min_diff)
                    {
                        temp_min = pps_boards.at(i)->pps_gui[j].pps_min_diff;
                    }
                    if (temp_max < pps_boards.at(i)->pps_gui[j].pps_max_diff)
                    {
                        temp_max = pps_boards.at(i)->pps_gui[j].pps_max_diff;
                    }
                }
            }
        }
        temp_max = ((temp_max/100)+1)*100;
        temp_min = ((temp_min/100)-1)*100;
        if (temp_max > 1000000000)
        {
            temp_max = 1000000000;
        }
        if (temp_min < -1000000000)
        {
            temp_min = -1000000000;
        }
    }
    else
    {
        temp_max = ui->PpsMaxScaleValue->text().toInt(nullptr, 10);
        temp_min = ui->PpsMinScaleValue->text().toInt(nullptr, 10);
        if (temp_max > 1000000000)
        {
            temp_max = 1000000000;
            ui->PpsMaxScaleValue->setText(QString::number(temp_max));
        }
        if (temp_min < -1000000000)
        {
            temp_min = -1000000000;
            ui->PpsMinScaleValue->setText(QString::number(temp_min));
        }
    }

    pps_offset_chart->removeAxis(pps_offset_chart_y_axis);
    pps_offset_chart_y_axis->setLabelFormat("%i");
    pps_offset_chart_y_axis->setTickCount(11);
    pps_offset_chart_y_axis->setMin(temp_min);
    pps_offset_chart_y_axis->setMax(temp_max);
    pps_offset_chart->addAxis(pps_offset_chart_y_axis, Qt::AlignLeft);
    foreach (QAbstractSeries *i, pps_offset_chart->series())
    {
        i->attachAxis(pps_offset_chart_y_axis);
    }

    // statistic
    ui->PpsStatisticsValue->clear();
    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            if (pps_boards.at(i)->pps_gui[j].pps_show != 0)
            {
                QString temp_string;
                temp_string.append(pps_boards.at(i)->pps_gui[j].pps_name);
                temp_string.append(":\n");
                temp_string.append("cur: ");
                if (pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->count() > 0)
                {
                    QPointF temp_point = pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->at(pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->count()-1);
                    temp_string.append(QString::number(temp_point.y()));
                    temp_string.append("ns, avg: ");
                    temp_string.append(QString::number(pps_boards.at(i)->pps_gui[j].pps_avg_diff, 'f', 3));
                    temp_string.append("ns\n1σ: ");
                    temp_string.append(QString::number(pps_boards.at(i)->pps_gui[j].pps_std_deviation, 'f', 3));
                    temp_string.append("ns, min/max: ");
                    temp_string.append(QString::number(pps_boards.at(i)->pps_gui[j].pps_min_diff));
                    temp_string.append("/");
                    temp_string.append(QString::number(pps_boards.at(i)->pps_gui[j].pps_max_diff));
                    temp_string.append("ns\n");
                }
                else
                {
                    temp_string.append("NA, avg: ");
                    temp_string.append("NA\n1σ: ");
                    temp_string.append("NA, min/max: ");
                    temp_string.append("NA\n");
                }
                //cout << "VERBOSE: " << pps_boards.at(i)->pps_gui[j].pps_name.toLatin1().constData() << " accuracy is: " << pps_boards.at(i)->pps_gui[j].pps_avg_diff << "ns +/-" << pps_boards.at(i)->pps_gui[j].pps_std_deviation << "ns" << endl ;

                ui->PpsStatisticsValue->addItem(temp_string);
                pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->show();
                pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->show();
                pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->show();
            }
            else
            {
                pps_boards.at(i)->pps_gui[j].pps_offset_chart_series->hide();
                pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->hide();
                pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->hide();
            }
        }
    }

    // enable updates
    pps_offset_chart_view->setUpdatesEnabled(true);

    // unlock gui
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_gui_chart.unlock();
    }

    pps_offset_chart->show();
}

static void pps_gui_histogram_sample(Upa_PpsTab* upa_pps)
{
    int temp_size;
    QColor temp_color;
    int temp_range_max;

    // lock gui
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_gui_histogram.lock();
    }

    temp_range_max = upa_pps->ui->PpsOffsetScrollBar->sliderPosition();

    // lock all passing of samples
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_ctrl_gui.lock();
    }

    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples_histogram.clear();

            if ((j == 0) || (upa_pps->pps_boards.at(i)->pps_gui[j].pps_show != 0))
            {
                temp_size = upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples.size();
                for (int k = upa_pps->pps_zoom_factor; k >= 0 ; k--)
                {
                    int temp_index = temp_size - 1 + temp_range_max - k;
                    if ((temp_index >= 0) && (temp_index < temp_size))
                    {
                        upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples_histogram.append(upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples.at(temp_index));
                    }
                }
            }
        }
    }

    // unlock all passing of samples
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_ctrl_gui.unlock();
    }

    // unlock gui
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_gui_histogram.unlock();
    }
}

void Upa_PpsTab::pps_gui_timer_histogram_run(void)
{
    int temp_size;
    int temp_range;
    QColor temp_color;
    int temp_min;
    int temp_max;
    int temp_range_min;
    int temp_range_max;
    int temp_step;

    // lock gui
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_gui_histogram.lock();
    }

    // disable updates
    pps_offset_histogram_view->setUpdatesEnabled(false);

    //remove all Series
    foreach (QAbstractSeries *i, pps_offset_histogram->series())
    {
        pps_offset_histogram->removeSeries(i);
    }

    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->clear();
        }
    }

    if (ui->PpsFixedScaleCheckBox->isChecked() == false)
    {
        temp_min = 0;
        temp_max = 0;
        for (int i = 0; i < pps_boards.size(); i++)
        {
            for (int j = 0; j < Upa_PpsPerBoard; j++)
            {
                if ((j == 0) || (pps_boards.at(i)->pps_gui[j].pps_show != 0))
                {
                    temp_size = pps_boards.at(i)->pps_gui[j].pps_samples_histogram.size();
                    for (int k = 0; k < temp_size ; k++)
                    {
                        if ((pps_boards.at(i)->pps_gui[j].pps_samples_histogram.at(k).pps_offset > temp_max) &&
                            (pps_boards.at(i)->pps_gui[j].pps_samples_histogram.at(k).pps_active != 0))
                        {
                            temp_max = pps_boards.at(i)->pps_gui[j].pps_samples_histogram.at(k).pps_offset;
                        }
                        if ((pps_boards.at(i)->pps_gui[j].pps_samples_histogram.at(k).pps_offset < temp_min) &&
                            (pps_boards.at(i)->pps_gui[j].pps_samples_histogram.at(k).pps_active != 0))
                        {
                            temp_min = pps_boards.at(i)->pps_gui[j].pps_samples_histogram.at(k).pps_offset;
                        }

                    }
                }
            }
        }
        temp_max = ((temp_max/100)+1)*100;
        temp_min = ((temp_min/100)-1)*100;
        if (temp_max > 1000000000)
        {
            temp_max = 1000000000;
        }
        if (temp_min < -1000000000)
        {
            temp_min = -1000000000;
        }
    }
    else
    {
        temp_max = ui->PpsMaxScaleValue->text().toInt(nullptr, 10);
        temp_min = ui->PpsMinScaleValue->text().toInt(nullptr, 10);
        if (temp_max > 1000000000)
        {
            temp_max = 1000000000;
            ui->PpsMaxScaleValue->setText(QString::number(temp_max));
        }
        if (temp_min < -1000000000)
        {
            temp_min = -1000000000;
            ui->PpsMinScaleValue->setText(QString::number(temp_min));
        }
    }


    // the min/max is our range for the histogram
    if (abs(temp_min) > abs(temp_max))
    {
        temp_range_min = -abs(temp_min);
        temp_range_max = abs(temp_min);
    }
    else
    {
        temp_range_min = -abs(temp_max);
        temp_range_max = abs(temp_max);
    }

    temp_range = (temp_range_max-temp_range_min);

    // create ~2000 steps in between (less for smaller offsets)
    temp_step = temp_range / 2000;
    if (temp_step <= 0)
    {
        temp_step = 1; // we need a step
    }

    temp_max = 0;
    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->setName(pps_boards.at(i)->pps_gui[j].pps_name);

            if (pps_boards.at(i)->pps_gui[j].pps_show != 0)
            {
                // first create all 0s
                for (int k = temp_range_min; k <= temp_range_max; k += temp_step)
                {
                    pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->append(k, 0);
                }
                // now count

                temp_size = pps_boards.at(i)->pps_gui[j].pps_samples_histogram.size();
                for (int k = 0; k < temp_size ; k++)
                {
                    if (pps_boards.at(i)->pps_gui[j].pps_samples_histogram.at(k).pps_active != 0)
                    {
                        int temp_offset = pps_boards.at(i)->pps_gui[j].pps_samples_histogram.at(k).pps_offset;
                        temp_offset /= temp_step;
                        temp_offset += (pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->count()/2); // this is the index
                        if ((temp_offset >= 0) && (temp_offset < pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->count()))
                        {
                            QPointF temp_point = pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->at(temp_offset);
                            temp_point.setY(temp_point.y()+1); // increment count
                            pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->replace(temp_offset, temp_point);
                            if (temp_point.y() > temp_max)
                            {
                                temp_max = (int)temp_point.y();
                            }
                        }
                    }
                }
                pps_offset_histogram->addSeries(pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series);
            }

            if (j != 0)
            {
                if (pps_boards.at(i)->pps_gui[j].pps_samples_histogram.isEmpty() ||
                    pps_boards.at(i)->pps_gui[j].pps_samples_histogram.last().pps_active == 0)
                {
                    temp_color.setRgb(128, 128, 128);
                    pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->setColor(temp_color);
                }
                else
                {
                    switch (j) // only 8 Input PPS per analyzer
                    {
                        case 1:
                            temp_color.setRgb(255, 64, 64);
                            pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->setColor(temp_color);
                            break;
                        case 2:
                            temp_color.setRgb(64, 255, 64);
                            pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->setColor(temp_color);
                            break;
                        case 3:
                            temp_color.setRgb(64, 64, 255);
                            pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->setColor(temp_color);
                            break;
                        case 4:
                            temp_color.setRgb(255, 64, 255);
                            pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->setColor(temp_color);
                            break;
                        case 5:
                            temp_color.setRgb(160, 64, 64);
                            pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->setColor(temp_color);
                            break;
                        case 6:
                            temp_color.setRgb(64, 160, 64);
                            pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->setColor(temp_color);
                            break;
                        case 7:
                            temp_color.setRgb(64, 64, 160);
                            pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->setColor(temp_color);
                            break;
                        case 8:
                            temp_color.setRgb(160, 64, 160);
                            pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->setColor(temp_color);
                            break;
                        default:
                            pps_boards.at(i)->pps_gui[j].pps_offset_histogram_series->setColor(Qt::black);
                            break;
                    }
                }
            }
        }
    }

    temp_max = (int)((double)temp_max * 1.1);
    if (temp_max < 10)
    {
        temp_max = 10;
    }
    pps_offset_histogram->removeAxis(pps_offset_histogram_y_axis);
    pps_offset_histogram_y_axis->setLabelFormat("%i");
    pps_offset_histogram_y_axis->setTickCount(11);
    pps_offset_histogram_y_axis->setMin(0);
    pps_offset_histogram_y_axis->setMax(temp_max);
    pps_offset_histogram->addAxis(pps_offset_histogram_y_axis, Qt::AlignLeft);
    foreach (QAbstractSeries *i, pps_offset_histogram->series())
    {
        i->attachAxis(pps_offset_histogram_y_axis);
    }

    pps_offset_histogram->removeAxis(pps_offset_histogram_x_axis);
    pps_offset_histogram_x_axis->setLabelFormat("%i");
    pps_offset_histogram_x_axis->setTickCount(21);
    pps_offset_histogram_x_axis->setMin(temp_range_min);
    pps_offset_histogram_x_axis->setMax(temp_range_max);
    pps_offset_histogram->addAxis(pps_offset_histogram_x_axis, Qt::AlignBottom);
    foreach (QAbstractSeries *i, pps_offset_histogram->series())
    {
        i->attachAxis(pps_offset_histogram_x_axis);
    }

    // enable updates
    pps_offset_histogram_view->setUpdatesEnabled(true);

    // unlock gui
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_gui_histogram.unlock();
    }

    pps_offset_histogram->show();

}

static void pps_gui_variance_sample(Upa_PpsTab* upa_pps)
{
    int temp_size;
    QColor temp_color;
    int temp_range_max;

    // lock calculation
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_calc_variance.lock();
    }

    temp_range_max = upa_pps->ui->PpsOffsetScrollBar->sliderPosition();

    // lock all passing of samples
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_ctrl_gui.lock();
    }

    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples_variance.clear();

            if ((j == 0) || (upa_pps->pps_boards.at(i)->pps_gui[j].pps_show != 0))
            {
                temp_size = upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples.size();
                for (int k = upa_pps->pps_zoom_factor; k >= 0 ; k--)
                {
                    int temp_index = temp_size - 1 + temp_range_max - k;
                    if ((temp_index >= 0) && (temp_index < temp_size))
                    {
                        upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples_variance.append(upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples.at(temp_index));
                    }
                }
            }
        }
    }

    // unlock all passing of samples
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_ctrl_gui.unlock();
    }

    // unlock gui
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_calc_variance.unlock();
    }
}

static int pps_calc_variance_inc_sample_count(int sample_count, bool log_steps)
{
    if (log_steps == true)
    {
        return sample_count + (int)pow(10, (int)log10(sample_count));
    }
    else
    {
        return sample_count + 1;
    }
}    

static void pps_calc_variance(Upa_PpsTab* upa_pps)
{
    QElapsedTimer temp_timer;

    temp_timer.start();

    // lock gui
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_gui_variance.lock();
    }

    // lock calculation
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_calc_variance.lock();
    }

    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            if (upa_pps->pps_boards.at(i)->pps_gui[j].pps_show != 0)
            {
                upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples_calc_variance.clear();
                int temp_nr_of_samples = upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples_variance.size();
                bool temp_log_steps = upa_pps->ui->PpsOffsetVarianceLogStepsCheckBox->isChecked();
                for (int k=1; ((k <= temp_nr_of_samples/2) && (temp_nr_of_samples >= 2)); k=pps_calc_variance_inc_sample_count(k, temp_log_steps))
                {
                    QList<double> temp_subgroup_averages;
                    int temp_nr_of_subgroups = temp_nr_of_samples/k;
                    for (int s = 0; s < temp_nr_of_subgroups; s++)
                    {
                        double temp_average_subgroup = 0.00000001;
                        for (int l = (s*k); l < ((s*k)+k); l++)
                        {
                            if (l >= temp_nr_of_samples)
                            {
                                cerr << "NrS:" << temp_nr_of_samples << "S:" << s << " K:" << k << " L:" << l << endl;
                            }
                            else
                            {
                                temp_average_subgroup += (double)upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples_variance.at(l).pps_offset;
                            }
                        }
                        temp_average_subgroup = temp_average_subgroup / ((double)k);
                        temp_subgroup_averages.append(temp_average_subgroup);
                    }
                    double temp_variance = 0.0;
                    temp_nr_of_subgroups--;
                    for (int s=0; s<temp_nr_of_subgroups; s++)
                    {
                        double temp_diff = temp_subgroup_averages.at(s+1)-temp_subgroup_averages.at(s);
                        temp_variance += (temp_diff * temp_diff);
                    }
                    temp_variance = temp_variance / 2.0;
                    temp_variance = temp_variance / ((double)temp_nr_of_subgroups);
                    temp_variance = sqrt(temp_variance);
                    temp_subgroup_averages.clear();

                    if (temp_variance > 0.0)
                    {
                        Upa_PpsGuiVarianceSample temp_sample;
                        temp_sample.x = (double)k;
                        temp_sample.y = temp_variance;

                        upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples_calc_variance.append(temp_sample);
                    }
                }

                if ((upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples_variance.isEmpty() == false) &&
                    (upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples_calc_variance.isEmpty() == false))
                {
                    upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples_calc_variance.last().pps_active = upa_pps->pps_boards.at(i)->pps_gui[j].pps_samples_variance.last().pps_active;
                }
            }
        }
    }

    // unlock calculation
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_calc_variance.unlock();
    }

    // unlock calculation
    for (int i = 0; i < upa_pps->pps_boards.size(); i++)
    {
        upa_pps->pps_boards.at(i)->mutex_gui_variance.unlock();
    }

    upa_pps->pps_offset_variance_calc_time = temp_timer.elapsed();

}

void Upa_PpsTab::pps_calc_timer_variance_run(void)
{
    if (calc_variance_thread.isRunning() == false)
    {
        calc_variance_thread = QtConcurrent::run(pps_calc_variance, this);
    }

    if (false == pps_gui_timer_variance->isActive())
    {
        if (ui->PpsOffsetVarianceCalculateButton->isEnabled() == true)
        {
            ui->PpsOffsetVarianceCalculateButton->setText("Wait Calc");
            ui->PpsOffsetVarianceCalculateButton->setEnabled(false);
            ui->PpsOffsetVarianceAutoCalculateCheckBox->setEnabled(false);
        }
        //if (pps_offset_variance_calc_time == 0)
        //{
        //    pps_offset_variance_calc_time = 1;
        //}
        //pps_gui_timer_variance->start(pps_offset_variance_calc_time);
        pps_gui_timer_variance->start(10);
    }
}

void Upa_PpsTab::pps_gui_timer_variance_run(void)
{

    QColor temp_color;
    double temp_min;
    double temp_max;
    double temp_max_values;

    if (calc_variance_thread.isRunning() == true)
    {
        pps_gui_timer_variance->start(10);
        return;
    }

    // lock gui
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_gui_variance.lock();
    }

    // disable updates
    pps_offset_variance_view->setUpdatesEnabled(false);

    //remove all Series
    foreach (QAbstractSeries *i, pps_offset_variance->series())
    {
        pps_offset_variance->removeSeries(i);
    }

    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->clear();
        }
    }

    // variance
    temp_min = 1.0;
    temp_max = 0.0;
    temp_max_values = 0.0;
    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->setName(pps_boards.at(i)->pps_gui[j].pps_name);

            if (pps_boards.at(i)->pps_gui[j].pps_show != 0)
            {
                int temp_nr_of_samples = pps_boards.at(i)->pps_gui[j].pps_samples_calc_variance.size();

                for (int k=0; k<temp_nr_of_samples; k++)
                {
                    Upa_PpsGuiVarianceSample temp_sample = pps_boards.at(i)->pps_gui[j].pps_samples_calc_variance.at(k);

                    if (temp_sample.y > temp_max)
                    {
                        temp_max = temp_sample.y;
                    }
                    if (temp_sample.y < temp_min)
                    {
                        temp_min = temp_sample.y;
                    }

                    if (temp_sample.x > temp_max_values)
                    {
                        temp_max_values = temp_sample.x;
                    }

                    pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->append(temp_sample.x, temp_sample.y);
                }

                pps_offset_variance->addSeries(pps_boards.at(i)->pps_gui[j].pps_offset_variance_series);
            }

            if (j != 0)
            {
                if (pps_boards.at(i)->pps_gui[j].pps_samples_calc_variance.isEmpty() ||
                    pps_boards.at(i)->pps_gui[j].pps_samples_calc_variance.last().pps_active == 0)
                {
                    temp_color.setRgb(128, 128, 128);
                    pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->setColor(temp_color);
                }
                else
                {
                    switch (j) // only 8 Input PPS per analyzer
                    {
                        case 1:
                            temp_color.setRgb(255, 64, 64);
                            pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->setColor(temp_color);
                            break;
                        case 2:
                            temp_color.setRgb(64, 255, 64);
                            pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->setColor(temp_color);
                            break;
                        case 3:
                            temp_color.setRgb(64, 64, 255);
                            pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->setColor(temp_color);
                            break;
                        case 4:
                            temp_color.setRgb(255, 64, 255);
                            pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->setColor(temp_color);
                            break;
                        case 5:
                            temp_color.setRgb(160, 64, 64);
                            pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->setColor(temp_color);
                            break;
                        case 6:
                            temp_color.setRgb(64, 160, 64);
                            pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->setColor(temp_color);
                            break;
                        case 7:
                            temp_color.setRgb(64, 64, 160);
                            pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->setColor(temp_color);
                            break;
                        case 8:
                            temp_color.setRgb(160, 64, 160);
                            pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->setColor(temp_color);
                            break;
                        default:
                            pps_boards.at(i)->pps_gui[j].pps_offset_variance_series->setColor(Qt::black);
                            break;
                    }
                }
            }
        }
    }

    if ((temp_min == 1.0) && (temp_max == 0.0))
    {
        temp_max = 1.0;
        temp_min = 0.0;
    }
    else
    {
        temp_max = temp_max * 1.1;
        temp_min = temp_min * 0.9;
    }
    pps_offset_variance->removeAxis(pps_offset_variance_y_axis);
    pps_offset_variance_y_axis->setLabelFormat("%E");
    pps_offset_variance_y_axis->setMin(temp_min);
    pps_offset_variance_y_axis->setMax(temp_max);
    pps_offset_variance->addAxis(pps_offset_variance_y_axis, Qt::AlignLeft);
    foreach (QAbstractSeries *i, pps_offset_variance->series())
    {
        i->attachAxis(pps_offset_variance_y_axis);
    }

    pps_offset_variance->removeAxis(pps_offset_variance_x_axis);
    pps_offset_variance_x_axis->setLabelFormat("%E");
    pps_offset_variance_x_axis->setMin(1.0);
    pps_offset_variance_x_axis->setMax(temp_max_values);
    pps_offset_variance->addAxis(pps_offset_variance_x_axis, Qt::AlignBottom);
    foreach (QAbstractSeries *i, pps_offset_variance->series())
    {
        i->attachAxis(pps_offset_variance_x_axis);
    }

    // enable updates
    pps_offset_variance_view->setUpdatesEnabled(true);

    // unlock gui
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_gui_variance.unlock();
    }

    QString temp_string;
    temp_string.append("Nr Of Samples: ");
    temp_string.append(QString::number(((int)temp_max_values)*2));
    temp_string.append(" Calc in: ");
    temp_string.append(QString::number(pps_offset_variance_calc_time));
    temp_string.append("ms");
    ui->PpsOffsetVarianceNrSamplesLabel->setText(temp_string);

    pps_offset_variance->show();

    if ((ui->PpsOffsetVarianceCalculateButton->isEnabled() == false) &&
        (ui->PpsOffsetVarianceCalculateButton->text() == "Wait Calc"))
    {
        ui->PpsOffsetVarianceCalculateButton->setText("Calculate");
        ui->PpsOffsetVarianceCalculateButton->setEnabled(true);
    }
    ui->PpsOffsetVarianceAutoCalculateCheckBox->setEnabled(true);
}

void Upa_PpsTab::pps_threshold_timer_run(void)
{
    unsigned int temp_data = 0;
    unsigned int temp_addr = 0;
    int temp_threshold_high_exceeded;
    int temp_threshold_low_exceeded;
    QPixmap temp_pix;

    //********************************
    // calculations and forwarding
    //********************************
    // lock all passing of samples
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_ctrl_threshold.lock();
    }

    temp_threshold_high_exceeded = 0;
    temp_threshold_low_exceeded = 0;
    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < Upa_PpsPerBoard; j++)
        {
            temp_threshold_high_exceeded |= pps_boards.at(i)->pps_ctrl[j].pps_threshold_high_exceeded;
            temp_threshold_low_exceeded |= pps_boards.at(i)->pps_ctrl[j].pps_threshold_low_exceeded;
        }
    }

    // set Ok/Nok logo

    if ((temp_threshold_high_exceeded == 0) and (temp_threshold_low_exceeded == 0))
    {
        temp_pix.load(":/PpsTab/Ok.png");
    }
    else
    {
        temp_pix.load(":/PpsTab/Nok.png");
    }
    temp_pix = temp_pix.scaled( ui->PpsOkNokLabel->size(),Qt::KeepAspectRatio);
    ui->PpsOkNokLabel->setPixmap(temp_pix);

    // set threshold signals on all PPS shields (if available) and the reference for each
    for (int i = 0; i < pps_boards.size(); i++)
    {
        for (int j = 0; j < upa->io_core_config.size(); j++)
        {
            if (pps_boards.at(i)->com_port == upa->io_core_config.at(j).com_port)
            {
                temp_addr = upa->io_core_config.at(j).address_range_low;
                if (0 == upa->io_core_config.at(j).com_lib->read_reg(temp_addr + Upa_IoConf_OutputDataReg, temp_data))
                {
                    temp_data &= 0xFFFFFFFC;
                    if (temp_threshold_low_exceeded != 0)
                    {
                        temp_data |= 0x00000001;
                    }
                    if (temp_threshold_high_exceeded != 0)
                    {
                        temp_data |= 0x00000002;
                    }
                    if (0 == upa->io_core_config.at(j).com_lib->write_reg(temp_addr + Upa_IoConf_OutputDataReg, temp_data))
                    {
                        temp_data = (temp_data >> 4) & 0xF;
                        if (pps_boards.at(i)->pps_ref_channel != temp_data)
                        {
                            pps_boards.at(i)->pps_ref_channel = temp_data;
                            pps_boards.at(i)->pps_gui[0].pps_name.clear();
                            pps_boards.at(i)->pps_gui[0].pps_name.append(pps_boards.at(i)->com_port);
                            pps_boards.at(i)->pps_gui[0].pps_name.append("_REF (");
                            if (pps_boards.at(i)->pps_ref_channel == 0)
                            {
                                pps_boards.at(i)->pps_gui[0].pps_name.append(pps_boards.at(i)->com_port);
                                pps_boards.at(i)->pps_gui[0].pps_name.append("_PPS_REF");
                            }
                            else
                            {
                                pps_boards.at(i)->pps_gui[0].pps_name.append(pps_boards.at(i)->pps_gui[pps_boards.at(i)->pps_ref_channel].pps_name);
                            }
                            pps_boards.at(i)->pps_gui[0].pps_name.append(")");
                            if (ui_config->ui->PpsAnalyzerComboBox->currentText() == pps_boards.at(i)->com_port)
                            {
                                ui_config->ui->PpsAnalyzerRefSelectComboBox->setCurrentIndex(pps_boards.at(i)->pps_ref_channel);
                            }
                        }
                    }
                    else
                    {
                        cout << "ERROR: " << "could not write threshold values to PPS Analyzer" << endl;
                    }
                }
                else
                {
                    cout << "ERROR: " << "could not write threshold values to PPS Analyzer" << endl;
                }
            }
        }
    }

    // unlock all passing of samples
    for (int i = 0; i < pps_boards.size(); i++)
    {
        pps_boards.at(i)->mutex_ctrl_threshold.unlock();
    }

}

void Upa_PpsTab::pps_zoom_in_button_clicked(void)
{
    unsigned int temp_pps_zoom_factor;
    unsigned int temp_pps_zoom_multiplier = Upa_MinZoom;

    temp_pps_zoom_factor = pps_zoom_factor;

    while ((temp_pps_zoom_multiplier * 10) <= temp_pps_zoom_factor)
    {
        temp_pps_zoom_multiplier *= 10;
    }

    switch (temp_pps_zoom_factor / temp_pps_zoom_multiplier)
    {
        case 1:
            if (((temp_pps_zoom_multiplier / 10) * 8) > Upa_MinZoom)
            {
                temp_pps_zoom_factor = (temp_pps_zoom_multiplier / 10) * 8;
            }
            else
            {
                temp_pps_zoom_factor = Upa_MinZoom;
                ui->PpsZoomInButton->setEnabled(false);
            }
            break;

        case 2:
            if ((temp_pps_zoom_multiplier * 1) > Upa_MinZoom)
            {
                temp_pps_zoom_factor = temp_pps_zoom_multiplier * 1;
            }
            else
            {
                temp_pps_zoom_factor = Upa_MinZoom;
                ui->PpsZoomInButton->setEnabled(false);
            }
            break;

        case 4:
            if ((temp_pps_zoom_multiplier * 2) > Upa_MinZoom)
            {
                temp_pps_zoom_factor = temp_pps_zoom_multiplier * 2;
            }
            else
            {
                temp_pps_zoom_factor = Upa_MinZoom;
                ui->PpsZoomInButton->setEnabled(false);
            }
            break;

        case 6:
            if ((temp_pps_zoom_multiplier * 4) > Upa_MinZoom)
            {
                temp_pps_zoom_factor = temp_pps_zoom_multiplier * 4;
            }
            else
            {
                temp_pps_zoom_factor = Upa_MinZoom;
                ui->PpsZoomInButton->setEnabled(false);
            }
            break;

        case 8:
            if ((temp_pps_zoom_multiplier * 6) > Upa_MinZoom)
            {
                temp_pps_zoom_factor = temp_pps_zoom_multiplier * 6;
            }
            else
            {
                temp_pps_zoom_factor = Upa_MinZoom;
                ui->PpsZoomInButton->setEnabled(false);
            }
            break;

        default:
            temp_pps_zoom_factor = Upa_MinZoom;
            ui->PpsZoomInButton->setEnabled(false);
            break;
    }

    pps_zoom_factor = temp_pps_zoom_factor;
    ui->PpsOffsetScrollBar->setMaximum(0);
    ui->PpsOffsetScrollBar->setMinimum((int)pps_zoom_factor-Upa_MaxSamples);
    if (ui->PpsOffsetScrollBar->sliderPosition() < ui->PpsOffsetScrollBar->minimum())
    {
        ui->PpsOffsetScrollBar->setSliderPosition(-Upa_MaxSamples + (int)pps_zoom_factor);
    }
    ui->PpsOffsetScrollBar->setPageStep(pps_zoom_factor);
    ui->PpsOffsetScrollBar->setSingleStep(1);

    ui->PpsZoomOutButton->setEnabled(true); // now we can zoom out for sure
}

void Upa_PpsTab::pps_zoom_out_button_clicked(void)
{
    unsigned int temp_pps_zoom_factor;
    unsigned int temp_pps_zoom_multiplier = Upa_MinZoom;

    temp_pps_zoom_factor = pps_zoom_factor;

    while ((temp_pps_zoom_multiplier * 10) <= temp_pps_zoom_factor)
    {
        temp_pps_zoom_multiplier *= 10;
    }

    switch (temp_pps_zoom_factor / temp_pps_zoom_multiplier)
    {
        case 1:
            if ((temp_pps_zoom_multiplier * 2) < Upa_MaxSamples)
            {
                temp_pps_zoom_factor = temp_pps_zoom_multiplier * 2;
            }
            else
            {
                temp_pps_zoom_factor = Upa_MaxSamples;
                ui->PpsZoomOutButton->setEnabled(false);
            }
            break;

        case 2:
            if ((temp_pps_zoom_multiplier * 4) < Upa_MaxSamples)
            {
                temp_pps_zoom_factor = temp_pps_zoom_multiplier * 4;
            }
            else
            {
                temp_pps_zoom_factor = Upa_MaxSamples;
                ui->PpsZoomOutButton->setEnabled(false);
            }
            break;

        case 4:
            if ((temp_pps_zoom_multiplier * 6) < Upa_MaxSamples)
            {
                temp_pps_zoom_factor = temp_pps_zoom_multiplier * 6;
            }
            else
            {
                temp_pps_zoom_factor = Upa_MaxSamples;
                ui->PpsZoomOutButton->setEnabled(false);
            }
            break;

        case 6:
            if ((temp_pps_zoom_multiplier * 8) < Upa_MaxSamples)
            {
                temp_pps_zoom_factor = temp_pps_zoom_multiplier * 8;
            }
            else
            {
                temp_pps_zoom_factor = Upa_MaxSamples;
                ui->PpsZoomOutButton->setEnabled(false);
            }
            break;

        case 8:
            if ((temp_pps_zoom_multiplier * 10) < Upa_MaxSamples)
            {
                temp_pps_zoom_factor = temp_pps_zoom_multiplier * 10;
            }
            else
            {
                temp_pps_zoom_factor = Upa_MaxSamples;
                ui->PpsZoomOutButton->setEnabled(false);
            }
            break;

        default:
            temp_pps_zoom_factor = Upa_MaxSamples;
            ui->PpsZoomOutButton->setEnabled(false);
            break;
    }


    pps_zoom_factor = temp_pps_zoom_factor;

    ui->PpsZoomInButton->setEnabled(true); // now we can zoom in for sure
    ui->PpsOffsetScrollBar->setMaximum(0);
    if (pps_zoom_factor == Upa_MaxSamples)
    {
        ui->PpsOffsetScrollBar->setMinimum(0);
    }
    else
    {
        ui->PpsOffsetScrollBar->setMinimum((int)pps_zoom_factor-Upa_MaxSamples);
    }
    if (ui->PpsOffsetScrollBar->sliderPosition() < ui->PpsOffsetScrollBar->minimum())
    {
        ui->PpsOffsetScrollBar->setSliderPosition(-Upa_MaxSamples + (int)pps_zoom_factor);
    }
    ui->PpsOffsetScrollBar->setPageStep(pps_zoom_factor);
    ui->PpsOffsetScrollBar->setSingleStep(1);
}

void Upa_PpsTab::pps_calc_variance_button_clicked(void)
{
    pps_calc_timer_variance_run();
}

void Upa_PpsTab::pps_auto_calc_variance_changed(void)
{
    if (ui->PpsOffsetVarianceAutoCalculateCheckBox->isChecked() == true)
    {
        ui->PpsOffsetVarianceCalculateButton->setEnabled(false);
    }
    else
    {
        pps_calc_timer_variance->stop();
        pps_gui_timer_variance->stop();
        ui->PpsOffsetVarianceCalculateButton->setEnabled(true);
    }
}

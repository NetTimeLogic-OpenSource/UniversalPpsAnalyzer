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

#include <Upa_ConfigTab.h>
#include <ui_Upa_ConfigTab.h>

Upa_ConfigTab::Upa_ConfigTab(Upa_UniversalPpsAnalyzer *parent) : QWidget()
{
    upa = parent;

    ui = new Ui::Upa_ConfigTab();
    ui->setupUi(this);

    config_timer = new QTimer(this);
    config_timer->stop();

    config_init_interfaces();

    connect(ui->ConfigConnectButton, SIGNAL(clicked()), this, SLOT(config_connect_button_clicked()));
    connect(ui->ConfigFullScreenButton, SIGNAL(clicked()), this, SLOT(config_full_screen_button_clicked()));
    connect(config_timer, SIGNAL(timeout()), this, SLOT(config_com_port_check_timer()));
}

Upa_ConfigTab::~Upa_ConfigTab()
{
    config_timer->stop();
    if (upa->pps_tab != 0)
    {
        delete(upa->pps_tab);
        upa->pps_tab = 0;
    }
    delete ui;
    delete config_timer;
}

void Upa_ConfigTab::config_init_interfaces(void)
{
    // Fill in available ports in interface List
    // Serial Ports
    QList<QSerialPortInfo> com_ports = QSerialPortInfo::availablePorts();
    for (int k = 0; k < com_ports.size(); k++)
    {
        ui->ConfigSerialList->addItem(com_ports[k].portName());
    }

    // Ethernet Ports
    QList<QNetworkInterface> eth_interfaces = QNetworkInterface::allInterfaces();
    for (int k = 0; k < eth_interfaces.size(); k++)
    {
        if ((eth_interfaces.at(k).type() == QNetworkInterface::Unknown) ||
            //(eth_interfaces.at(k).type() == QNetworkInterface::Wifi) || // probably not wifi
            (eth_interfaces.at(k).type() == QNetworkInterface::Ethernet))
        {
            ui->ConfigEthernetList->addItem(eth_interfaces.at(k).humanReadableName());
        }
    }

}

int Upa_ConfigTab::config_resize(int height, int width)
{
    int height_delta = 0;
    int width_delta = 0;

    height_delta = (height-Upa_MainHeight);
    width_delta = (width-Upa_MainWidth);

    ui->ConfigAddressMapValue->setFixedHeight(630+height_delta);
    ui->ConfigAddressMapValue->setFixedWidth(1050+width_delta);

    updateGeometry();

    return 0;
}

void Upa_ConfigTab::config_connect_button_clicked(void)
{
    QList<QSerialPortInfo> com_ports = QSerialPortInfo::availablePorts();
    QList<QString> selected_com_itf_names;
    QList<QString> selected_eth_itf_names;
    QList<QString> eth_ports;
    long long* temp_ref_offset;
    unsigned int core_count;
    unsigned int temp_data = 0;
    Upa_CommunicationLib* temp_com_lib;
    Upa_CoreConfig temp_config;
    QString com_port;
    QString temp_string;

    // Build List of selected Serial ports
    for (int k=0; k<ui->ConfigSerialList->selectedItems().size(); k++)
    {
        selected_com_itf_names.append(ui->ConfigSerialList->selectedItems().at(k)->text());
    }

    // Build List of selected Ethernet ports
    for (int k=0; k<ui->ConfigEthernetList->selectedItems().size(); k++)
    {
        selected_eth_itf_names.append(ui->ConfigEthernetList->selectedItems().at(k)->text());
    }

    if (ui->ConfigConnectButton->text() == "Connect")
    {
        ui->ConfigConnectButton->setEnabled(false);
        //********************************
        // com
        //********************************
        for (int k = 0; k < com_ports.size(); k++)
        {
            com_port = com_ports[k].portName();
            // If port has been selected by the user then open it
            if (true == selected_com_itf_names.contains(com_port))
            {
                temp_com_lib = new(Upa_CommunicationLib);
                if (0 == temp_com_lib->open_port(com_port))
                {
                    temp_ref_offset = new(long long);
                    *temp_ref_offset = 0;
                    core_count = 0;

                    // Advanced Tab
                    upa->advanced_tab->advanced_enable();

                    for (int i = 0; i < 256; i++)
                    {
                        if (0 == temp_com_lib->read_reg((0x00000000 + ((i * Upa_Config_BlockSize) + Upa_Config_TypeInstanceReg)), temp_data))
                        {
                            if ((i == 0) &&
                                ((((temp_data >> 16) & 0x0000FFFF) != Upa_CoreConfig_ConfSlaveCoreType) ||
                                 (((temp_data >> 0) & 0x0000FFFF) != 1)))
                            {
                                cout << "ERROR: " << "not a conf block at the address expected" << endl;
                                break;
                            }
                            else if (temp_data == 0)
                            {
                                break;
                            }
                            else
                            {
                                temp_config.core_type = ((temp_data >> 16) & 0x0000FFFF);
                                temp_config.core_instance_nr = ((temp_data >> 0) & 0x0000FFFF);
                            }
                        }
                        else
                        {
                            break;
                        }

                        if (0 == temp_com_lib->read_reg((0x00000000 + ((i * Upa_Config_BlockSize) + Upa_Config_BaseAddrLReg)), temp_data))
                        {
                            temp_config.address_range_low = temp_data;
                        }
                        else
                        {
                            break;
                        }

                        if (0 == temp_com_lib->read_reg((0x00000000 + ((i * Upa_Config_BlockSize) + Upa_Config_BaseAddrHReg)), temp_data))
                        {
                            temp_config.address_range_high= temp_data;
                        }
                        else
                        {
                            break;
                        }

                        if (0 == temp_com_lib->read_reg((0x00000000 + ((i * Upa_Config_BlockSize) + Upa_Config_IrqMaskReg)), temp_data))
                        {
                            temp_config.interrupt_mask = temp_data;
                        }
                        else
                        {
                            break;
                        }
                        temp_config.com_port = com_port;
                        temp_config.com_lib = temp_com_lib;
                        temp_config.ref_offset = temp_ref_offset;

                        if (temp_config.core_type == Upa_CoreConfig_ClkSignalTimestamperCoreType)
                        {
                            core_count++;
                            upa->ts_core_config.append(temp_config);
                        }
                        else if (temp_config.core_type == Upa_CoreConfig_IoConfigurationCoreType)
                        {
                            upa->io_core_config.append(temp_config);
                        }
                        else if (temp_config.core_type == Upa_CoreConfig_I2cConfigurationCoreType)
                        {
                            // Advanced Tab
                            upa->i2c_core_config.append(temp_config);
                        }
                    }

                    if (core_count == 0)
                    {
                        temp_com_lib->close_port();
                        delete(temp_com_lib);
                        delete(temp_ref_offset);
                    }
                    else
                    {
                        upa->com_lib.append(temp_com_lib);
                    }
                }
                else
                {
                    delete(temp_com_lib);
                }
            }
            else
            {
                cout << "INFO: " << "Skipping unselected port " << com_port.toLatin1().constData() << endl ;
            }
        }

        //********************************
        // ethernet
        //********************************
        temp_com_lib = new(Upa_CommunicationLib);
        eth_ports = temp_com_lib->create_eth_ports(selected_eth_itf_names);
        delete(temp_com_lib);

        for (int k = 0; k < eth_ports.size(); k++)
        {
            com_port = eth_ports.at(k);
            temp_com_lib = new(Upa_CommunicationLib);
            if (0 == temp_com_lib->open_port(com_port))
            {
                temp_ref_offset = new(long long);
                *temp_ref_offset = 0;
                core_count = 0;

                // Advanced Tab
                upa->advanced_tab->advanced_enable();

                for (int i = 0; i < 256; i++)
                {
                    if (0 == temp_com_lib->read_reg((0x00000000 + ((i * Upa_Config_BlockSize) + Upa_Config_TypeInstanceReg)), temp_data))
                    {
                        if ((i == 0) &&
                            ((((temp_data >> 16) & 0x0000FFFF) != Upa_CoreConfig_ConfSlaveCoreType) ||
                             (((temp_data >> 0) & 0x0000FFFF) != 1)))
                        {
                            cout << "ERROR: " << "not a conf block at the address expected" << endl;
                            break;
                        }
                        else if (temp_data == 0)
                        {
                            break;
                        }
                        else
                        {
                            temp_config.core_type = ((temp_data >> 16) & 0x0000FFFF);
                            temp_config.core_instance_nr = ((temp_data >> 0) & 0x0000FFFF);
                        }
                    }
                    else
                    {
                        break;
                    }

                    if (0 == temp_com_lib->read_reg((0x00000000 + ((i * Upa_Config_BlockSize) + Upa_Config_BaseAddrLReg)), temp_data))
                    {
                        temp_config.address_range_low = temp_data;
                    }
                    else
                    {
                        break;
                    }

                    if (0 == temp_com_lib->read_reg((0x00000000 + ((i * Upa_Config_BlockSize) + Upa_Config_BaseAddrHReg)), temp_data))
                    {
                        temp_config.address_range_high = temp_data;
                    }
                    else
                    {
                        break;
                    }

                    if (0 == temp_com_lib->read_reg((0x00000000 + ((i * Upa_Config_BlockSize) + Upa_Config_IrqMaskReg)), temp_data))
                    {
                        temp_config.interrupt_mask = temp_data;
                    }
                    else
                    {
                        break;
                    }
                    temp_config.com_port = com_port;
                    temp_config.com_lib = temp_com_lib;
                    temp_config.ref_offset = temp_ref_offset;

                    if (temp_config.core_type == Upa_CoreConfig_ClkSignalTimestamperCoreType)
                    {
                        core_count++;
                        upa->ts_core_config.append(temp_config);
                    }
                    else if (temp_config.core_type == Upa_CoreConfig_IoConfigurationCoreType)
                    {
                        upa->io_core_config.append(temp_config);
                    }
                    else if (temp_config.core_type == Upa_CoreConfig_I2cConfigurationCoreType)
                    {
                        // Advanced Tab
                        upa->i2c_core_config.append(temp_config);
                    }
                }

                if (core_count == 0)
                {
                    temp_com_lib->close_port();
                    delete(temp_com_lib);
                    delete(temp_ref_offset);
                }
                else
                {
                    upa->com_lib.append(temp_com_lib);
                }
            }
            else
            {
                delete(temp_com_lib);
            }
        }

        //********************************
        // config
        //********************************
        if (false == upa->ts_core_config.isEmpty())
        {
            ui->ConfigAddressMapValue->clear();
        }
        else
        {
            cout << "ERROR: " << "no device found" << endl;
            temp_string.append("NA");
            QMessageBox::information(this, tr("Connect"), tr("No device found"));
            ui->ConfigConnectButton->setEnabled(true);
            // Advanced Tab
            upa->advanced_tab->advanced_disable();
            return;
        }

        if (false == upa->i2c_core_config.isEmpty())
        {
            upa->advanced_tab->advanced_enable_calibration();
        }
        else
        {
            upa->advanced_tab->advanced_disable_calibration();
        }

        for (int i = 0; i < upa->ts_core_config.size(); i++)
        {
            temp_string.append("0x");
            temp_string.append(QString("%1").arg(upa->ts_core_config.at(i).address_range_low, 8, 16, QLatin1Char('0')).toUpper());
            temp_string.append(" - ");
            temp_string.append("0x");
            temp_string.append(QString("%1").arg(upa->ts_core_config.at(i).address_range_high, 8, 16, QLatin1Char('0')).toUpper());
            temp_string.append(", ");
            temp_string.append("InstNr: ");
            temp_string.append(QString("%1").arg(upa->ts_core_config.at(i).core_instance_nr, 4, 10, QLatin1Char('0')).toUpper());
            temp_string.append(", ");
            temp_string.append("Type: ");

            switch (upa->ts_core_config.at(i).core_type)
            {
            case Upa_CoreConfig_ClkSignalTimestamperCoreType:
                temp_string.append("CLK SignalTimestamper");
                break;
             default:
                break;
            }
            temp_string.append(", ");
            temp_string.append("Com Port: ");
            temp_string.append(upa->ts_core_config.at(i).com_port);
            temp_string.append("\n");
        }

        for (int i = 0; i < upa->io_core_config.size(); i++)
        {
            temp_string.append("0x");
            temp_string.append(QString("%1").arg(upa->io_core_config.at(i).address_range_low, 8, 16, QLatin1Char('0')).toUpper());
            temp_string.append(" - ");
            temp_string.append("0x");
            temp_string.append(QString("%1").arg(upa->io_core_config.at(i).address_range_high, 8, 16, QLatin1Char('0')).toUpper());
            temp_string.append(", ");
            temp_string.append("InstNr: ");
            temp_string.append(QString("%1").arg(upa->io_core_config.at(i).core_instance_nr, 4, 10, QLatin1Char('0')).toUpper());
            temp_string.append(", ");
            temp_string.append("Type: ");

            switch (upa->io_core_config.at(i).core_type)
            {
            case Upa_CoreConfig_IoConfigurationCoreType:
                temp_string.append("IO Configuration");
                break;
             default:
                break;
            }
            temp_string.append(", ");
            temp_string.append("Com Port: ");
            temp_string.append(upa->io_core_config.at(i).com_port);
            temp_string.append("\n");
        }

        for (int i = 0; i < upa->i2c_core_config.size(); i++)
        {
            temp_string.append("0x");
            temp_string.append(QString("%1").arg(upa->i2c_core_config.at(i).address_range_low, 8, 16, QLatin1Char('0')).toUpper());
            temp_string.append(" - ");
            temp_string.append("0x");
            temp_string.append(QString("%1").arg(upa->i2c_core_config.at(i).address_range_high, 8, 16, QLatin1Char('0')).toUpper());
            temp_string.append(", ");
            temp_string.append("InstNr: ");
            temp_string.append(QString("%1").arg(upa->i2c_core_config.at(i).core_instance_nr, 4, 10, QLatin1Char('0')).toUpper());
            temp_string.append(", ");
            temp_string.append("Type: ");

            switch (upa->i2c_core_config.at(i).core_type)
            {
            case Upa_CoreConfig_I2cConfigurationCoreType:
                temp_string.append("I2C Configuration");
                break;
             default:
                break;
            }
            temp_string.append(", ");
            temp_string.append("Com Port: ");
            temp_string.append(upa->i2c_core_config.at(i).com_port);
            temp_string.append("\n");
        }

        ui->ConfigAddressMapValue->setText(temp_string);


        // Pps Tab
        upa->pps_tab = new Upa_PpsTab(upa);
        upa->Upa_MainTab->addTab(upa->pps_tab, "Pps");
        QSize new_size = upa->Upa_MainWindow->size();
        upa->pps_tab->pps_resize((new_size.height()+21), new_size.width());
        upa->pps_tab->pps_enable();
        upa->Upa_MainTab->setCurrentIndex(upa->Upa_MainTab->indexOf(upa->pps_tab)); // directly go to PPS

        config_timer->start(2000);
        ui->ConfigConnectButton->setText("Disconnect");
        ui->ConfigConnectButton->setEnabled(true);

    }
    else
    {
        ui->ConfigConnectButton->setEnabled(false);
        // Config Tab
        ui->ConfigAddressMapValue->setText("NA");
        config_timer->stop();
        ui->ConfigConnectButton->setText("Connect");
        //Remove all other tabs
        while (upa->Upa_MainTab->count() > 2)
        {
            upa->Upa_MainTab->removeTab(2);
        }

        //PPS Tab
        if (upa->pps_tab != NULL)
        {
            upa->pps_tab->pps_disable();
            delete(upa->pps_tab);
            upa->pps_tab = NULL;
        }

        //Delete Ref offset for this board
        for (int i = 0; i < upa->ts_core_config.size(); i++)
        {
            if (upa->ts_core_config.at(i).core_instance_nr == 1)
            {
                delete(upa->ts_core_config.at(i).ref_offset);
            }
        }
        upa->ts_core_config.clear();
        upa->io_core_config.clear();
        upa->i2c_core_config.clear();

        // Advanced Tab
        upa->advanced_tab->advanced_disable_calibration();
        upa->advanced_tab->advanced_disable();
        for (int i = 0; i < upa->com_lib.size(); i++)
        {
            if (0 == upa->com_lib.at(i)->close_port())
            {
                // nothing
            }
        }
        for (int i = 0; i < upa->com_lib.size(); i++)
        {
            delete(upa->com_lib.at(i));
        }
        upa->com_lib.clear();

        // get ethernet ports
        temp_com_lib = new(Upa_CommunicationLib);
        temp_com_lib->destroy_eth_ports();
        delete(temp_com_lib);

        ui->ConfigConnectButton->setEnabled(true);
    }
}

void Upa_ConfigTab::config_full_screen_button_clicked(void)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    int height = screenGeometry.height();
    int width = screenGeometry.width();

    if (ui->ConfigFullScreenButton->text() == "Full Screen")
    {
        upa->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
        upa->showFullScreen();
        ui->ConfigFullScreenButton->setText("Normal Screen");
    }
    else
    {
        upa->setWindowFlags(Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
        upa->showNormal();
        ui->ConfigFullScreenButton->setText("Full Screen");
        if (height > (Upa_MainHeight+2))
        {
            height = (Upa_MainHeight+2);
        }
        if (width > Upa_MainWidth)
        {
            width = Upa_MainWidth;
        }
        upa->resize(width, height);
    }

}

void Upa_ConfigTab::config_com_port_check_timer(void)
{
    static unsigned int error_cnt[100];

    for (int i = 0; i < upa->com_lib.size(); i++)
    {
        if (0 == upa->com_lib.at(i)->check_port())
        {
            error_cnt[i] = 0;
        }
        else
        {
            error_cnt[i]++;
        }
        if (error_cnt[i] > 2)
        {
            // we are ok if it would reconnect
            cout << "ERROR: " << "port is not ok anymore closing it" << endl;
            upa->com_lib.at(i)->close_port();
            error_cnt[i] = 0;
        }
    }
}


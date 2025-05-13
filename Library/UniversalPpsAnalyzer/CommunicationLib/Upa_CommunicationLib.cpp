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

#include <Upa_CommunicationLib.h>

#include <string>
#include <iostream>
#include <QThread>
#include <QNetworkInterface>
#include <QNetworkDatagram>


using namespace std;

static QList<QString> eth_ports;
static QList<QUdpSocket*> eth_sockets;

Upa_CommunicationLib::Upa_CommunicationLib()
{
#if QT_MAJOR_VERSION < 6
    com_lock = new QMutex(QMutex::Recursive);
#else
    com_lock = new QRecursiveMutex();
#endif
    com_lock->lock();
    is_open = false;
    port_type = 0;
    com_lock->unlock();
}

Upa_CommunicationLib::~Upa_CommunicationLib()
{
    com_lock->lock();
    if (is_open == true)
    {
        close_port();
    }
    com_lock->unlock();
}

QList<QString> Upa_CommunicationLib::create_eth_ports (QList<QString> selected_eth_itf_names)
{
    QList<QNetworkInterface> eth_interfaces = QNetworkInterface::allInterfaces();
    QNetworkInterface temp_interface;
    int data_length;
    QByteArray read_data;
    QByteArray write_data;
    unsigned char checksum;
    QString temp_string;
    int nr_of_devices;
    int temp_response_count = 0;

    // Test command preparation
    write_data.append("$CC");
    checksum = 0;
    for (int i = 1; i < write_data.size(); i++)
    {
        checksum = checksum ^ write_data[i];
    }
    write_data.append('*');
    write_data.append(QString("%1").arg(checksum, 2, 16, QLatin1Char('0')).toUtf8());
    write_data.append(0x0D);
    write_data.append(0x0A);

    temp_string = write_data.constData();
    temp_string.chop(2);
    cout << "VERBOSE: " << "Command to be sent: " << temp_string.toLatin1().constData() << endl;

    for (int i = 0; i < eth_ports.size(); i++)
    {
        eth_sockets.at(i)->close();
        delete(eth_sockets.at(i));
    }
    eth_ports.clear();
    eth_sockets.clear();

    for (int i = write_data.size(); i < 18; i++)
    {
        write_data.append((char)0x00);
    }

    for (int i = 0; i < eth_interfaces.size(); i++)
    {
        temp_interface = eth_interfaces.at(i);

        // if interface name was selected by user
        if (true == selected_eth_itf_names.contains(temp_interface.humanReadableName()))
        {
            nr_of_devices = 0;
            cout << "VERBOSE: " << "trying on interface: " << temp_interface.name().toLatin1().constData() << "; " <<  endl ;
            QUdpSocket* temp_socket = new(QUdpSocket);
            if (false == temp_socket->bind(QHostAddress::AnyIPv4, 0xBEEF, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
            {
                cout << "ERROR: " << "bind port failed" << endl;
                continue;
            }
            else
            {
                cout << "VERBOSE: " << "bind port succeeded " << endl;
            }

            // Check that the interface UP before proceeding
            if (temp_interface.flags() & QNetworkInterface::IsUp)
            {
                cout << "VERBOSE: Interface is up" << endl ;
            }
            else
            {
                cout << "VERBOSE: Interface is down...  "  << endl ;
                goto cleanup;
            }
            // Check that IP stack is congfigured for this interface
            if (false == (temp_interface.addressEntries().size() > 0) )
            {
                cout << "ERROR: " << "IP stack not configured for this interface ->  Skipped ! " << endl ;
                goto cleanup;
            }
            else
            {
                QNetworkDatagram temp_datagram;
                QHostAddress temp_src_ip;
                QList<QNetworkAddressEntry> temp_address_entries = temp_interface.addressEntries();
                for (int j = 0; j < temp_address_entries.size(); j++)
                {
                    if (temp_address_entries.at(j).ip().protocol() == QAbstractSocket::IPv4Protocol)
                    {
                        temp_src_ip = temp_address_entries.at(j).ip();
                    }
                }
                if (temp_src_ip.isNull())
                {
                    cout << "ERROR: " << "No IP on that interface" << endl ;
                    goto cleanup;
                }
                temp_datagram.setData(write_data);
                temp_datagram.setDestination(QHostAddress::Broadcast, 0xBEEF);
                temp_datagram.setInterfaceIndex(temp_interface.index());
                temp_datagram.setSender(temp_src_ip, 0xBEEF);
                data_length = temp_socket->writeDatagram(temp_datagram);
            }

            if (data_length == -1)
            {
                cout << "ERROR: " << "write failed" << endl;
                goto cleanup;
            }
            else if (data_length != write_data.size())
            {
                cout << "ERROR: " << "write failed to send all data" << endl;
                goto cleanup;
            }

            // check response
            temp_response_count = 0;
            while (1)
            {
                if (false == temp_socket->waitForReadyRead(50))
                {
                    if (temp_response_count == 0)
                    {
                        cout << "ERROR: " << "no response received" << endl;
                    }
                    goto cleanup;
                }

                if (true == temp_socket->hasPendingDatagrams())
                {
                    QHostAddress src_addr;
                    quint16 src_port;

                    read_data.resize(temp_socket->pendingDatagramSize());
                    temp_socket->readDatagram(read_data.data(), read_data.size(), &src_addr, &src_port);

                    temp_string.clear();
                    temp_string = read_data.constData();
                    temp_string.chop(2);
                    cout << "VERBOSE: " << "received command: " << temp_string.toLatin1().constData() << endl;

                    if (true == read_data.startsWith("$CC"))
                    {
                        continue;
                    }
                    if (true == read_data.startsWith("$CR"))
                    {
                        cout << "VERBOSE: " << "found eth node at: " << src_addr.toString().toLatin1().constData() << endl;
                        nr_of_devices++;
                        temp_response_count++;
                        eth_ports.append(src_addr.toString());
                        eth_sockets.append(temp_socket);
                    }

                }
                else
                {
                    cout << "ERROR: " << "no packet available" << endl;
                    goto cleanup;
                }
            }

cleanup:
            if (nr_of_devices == 0)
            {
                temp_socket->close();
                delete(temp_socket);
            }
        }
        else
        {
            cout << "VERBOSE: " << "skipped unselected interface: " << temp_interface.name().toLatin1().constData() << "; " <<  endl ;
        }

    }
    return eth_ports;
}

int Upa_CommunicationLib::destroy_eth_ports(void)
{
    cout << "VERBOSE: " << "destroying eth ports!" << endl;
    for (int i = 0; i < eth_ports.size(); i++)
    {
        cout << "VERBOSE: " << "deleting eth node at: " << eth_ports.at(i).toLatin1().constData() << endl;
        eth_sockets.at(i)->close();
        delete(eth_sockets.at(i));
    }
    eth_ports.clear();
    eth_sockets.clear();

    return 0;
}

int Upa_CommunicationLib::detect_baudrate(void)
{
    int data_length;
    QByteArray read_data;
    QByteArray write_data;
    unsigned char checksum;
    QString temp_string;
    int baud_rate;
    int test_index;

    write_data.append("$CC");
    checksum = 0;
    for (int i = 1; i < write_data.size(); i++)
    {
        checksum = checksum ^ write_data[i];
    }
    write_data.append('*');
    write_data.append(QString("%1").arg(checksum, 2, 16, QLatin1Char('0')).toUtf8());
    write_data.append(0x0D);
    write_data.append(0x0A);

    // detect baud rate
    if (port_type == Upa_CommunicationLib_ComType)
    {
        for (test_index = 0; test_index < 4; test_index++)
        {
            switch (test_index)
            {
                case 0:
                    baud_rate = 1000000;
                    break;
                case 1:
                    baud_rate = 500000;
                    break;
                case 2:
                    baud_rate = 460800;
                    break;
                default:
                    baud_rate = 115200;
                    break;
            }

            // clear the port before we do this
            com_port.clear();

            if (false == com_port.setBaudRate(baud_rate))
            {
                cout << "ERROR: could not set baudrate to: " << baud_rate << endl;
                continue;
            }

            // send
            data_length = com_port.write(write_data);

            if (data_length == -1)
            {
                cout << "VERBOSE: " << "write failed" << endl;
                continue;
            }
            else if (data_length != write_data.size())
            {
                cout << "VERBOSE: " << "write failed to send all data" << endl;
                continue;
            }
            else if (false == com_port.waitForBytesWritten(500))
            {
                cout << "VERBOSE: " << "write timed out" << endl;
                continue;
            }

            // receive
            read_data = com_port.readAll();
            com_port.waitForReadyRead(50);
            for (int i=0; i < 32; i++)
            {
                if (read_data.endsWith(0x0A))
                {
                    break;
                }

                if(0 != com_port.bytesAvailable())
                {
                    read_data.append(com_port.readAll());
                }
                else
                {
                    com_port.waitForReadyRead(1);
                }
            }

            if (com_port.error() == QSerialPort::ReadError)
            {
                cout << "VERBOSE: " << "read failed" << endl;
                continue;
            }
            else if (com_port.error() == QSerialPort::TimeoutError && read_data.isEmpty())
            {
                cout << "VERBOSE: " << "no response received" << endl;
                continue;
            }

            temp_string = read_data.constData();
            temp_string.chop(2);
            cout << "VERBOSE: " << "received command: " << temp_string.toLatin1().constData() << endl;

            if (false == read_data.startsWith("$"))
            {
                cout << "WARNING " << "no correct response received (not our device?)" << endl;
                continue;
            }
            else
            {
                cout << "INFO: baudrate detected at: " << baud_rate << endl;
                return baud_rate;
            }
        }

        cout << "ERROR: no valid baudrate detected" << endl;
        return -1;
    }
    else
    {
        cout << "ERROR: not a serial port" << endl;
        return -1;
    }
}

int Upa_CommunicationLib::open_port(QString name)
{
    int data_length=-1;
    QByteArray read_data;
    QByteArray write_data;
    unsigned char checksum;
    QString temp_string;

    cout << "INFO: " << "Opening Port: " << name.toLatin1().constData() << endl;

    com_lock->lock();
    if (is_open == true)
    {
        cout << "port is already open" << endl;
        com_lock->unlock();
        return -1;
    }

    if ((true == name.startsWith("COM")) ||
        (true == name.startsWith("tty")))
    {
        port_name = name;
        com_port.setPortName(port_name);
        int baud_rate = QSerialPort::Baud115200;
        com_port.setBaudRate(baud_rate);

        if (false == com_port.open(QIODevice::ReadWrite))
        {
            cout << "ERROR: " << "opening port failed" << endl;
            com_lock->unlock();
            return -1;
        }
        else
        {
            cout << "INFO: " << "opening port: " << port_name.toLatin1().constData() << " successful" << endl;
        }

        port_type = Upa_CommunicationLib_ComType;

        // now test the port rate
        if (detect_baudrate() < 0)
        {
            com_port.close();
            com_lock->unlock();
            return -1;
        }
    }
    else
    {
        port_name = name;

        int port_index = 0;
        for (port_index = 0; port_index < eth_ports.size(); port_index++)
        {
            if (eth_ports.at(port_index).compare(port_name) == 0)
            {
                cout << "INFO: " << "eth port in list" << endl;
                eth_port = eth_sockets.at(port_index);
                break;
            }
        }
        if (port_index >= eth_ports.size())
        {
            cout << "INFO: " << "eth port not in list" << endl;
            eth_port =new(QUdpSocket);
            if (false == eth_port->bind(QHostAddress::AnyIPv4, 0xBEEF, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
            {
                cout << "ERROR: " << "bind port failed" << endl;
                com_lock->unlock();
                return -1;
            }
            else
            {
                cout << "INFO: " << "binding port: " << port_name.toLatin1().constData() << " successful" << endl;
            }
        }


        port_type = Upa_CommunicationLib_EthType;
    }

    write_data.append("$CC");
    checksum = 0;
    for (int i = 1; i < write_data.size(); i++)
    {
        checksum = checksum ^ write_data[i];
    }
    write_data.append('*');
    write_data.append(QString("%1").arg(checksum, 2, 16, QLatin1Char('0')).toUtf8());
    write_data.append(0x0D);
    write_data.append(0x0A);

    temp_string = write_data.constData();
    temp_string.chop(2);
    cout << "VERBOSE: " << "command to be sent : " << temp_string.toLatin1().constData() << endl;

    if (port_type == Upa_CommunicationLib_ComType)
    {
        data_length = com_port.write(write_data);

        if (data_length == -1)
        {
            com_port.close();
            cout << "ERROR: " << "write failed" << endl;
            com_lock->unlock();
            return -1;
        }
        else if (data_length != write_data.size())
        {
            com_port.close();
            cout << "ERROR: " << "write failed to send all data" << endl;
            com_lock->unlock();
            return -1;
        }
        else if (false == com_port.waitForBytesWritten(500))
        {
            com_port.close();
            cout << "ERROR: " << "write timed out" << endl;
            com_lock->unlock();
            return -1;
        }
    }
    else if (port_type == Upa_CommunicationLib_EthType)
    {
        for (int i = write_data.size(); i < 18; i++)
        {
            write_data.append((char)0x00);
        }

        data_length = eth_port->writeDatagram(write_data.data(), write_data.size(), QHostAddress(port_name), 0xBEEF);
        if (data_length == -1)
        {
            //eth_port->close();
            cout << "ERROR: " << "write failed" << endl;
            com_lock->unlock();
            return -1;
        }
        else if (data_length != write_data.size())
        {
            //eth_port->close();
            cout << "ERROR: " << "write failed to send all data" << endl;
            com_lock->unlock();
            return -1;
        }
    }
    else
    {
        cout << "ERROR: " << "unknown com type" << endl;
        com_lock->unlock();
        return -1;
    }

    // check response
    if (port_type == Upa_CommunicationLib_ComType)
    {
        read_data = com_port.readAll();
        com_port.waitForReadyRead(50);
        for (int i=0; i < 32; i++)
        {
            if (read_data.endsWith(0x0A))
            {
                break;
            }

            if(0 != com_port.bytesAvailable())
            {
                read_data.append(com_port.readAll());
            }
            else
            {
                com_port.waitForReadyRead(1);
            }
        }

        if (com_port.error() == QSerialPort::ReadError)
        {
            com_port.close();
            cout << "ERROR: " << "read failed" << endl;
            com_lock->unlock();
            return -1;
        }
        else if (com_port.error() == QSerialPort::TimeoutError && read_data.isEmpty())
        {
            com_port.close();
            cout << "ERROR: " << "no response received" << endl;
            com_lock->unlock();
            return -1;
        }
    }
    else if (port_type == Upa_CommunicationLib_EthType)
    {
        while (1)
        {
            if (false == eth_port->waitForReadyRead(50))
            {
                //eth_port->close();
                cout << "ERROR: " << "no response received" << endl;
                com_lock->unlock();
                return -1;
            }

            if (true == eth_port->hasPendingDatagrams())
            {
                QHostAddress src_addr;
                quint16 src_port;

                read_data.resize(eth_port->pendingDatagramSize());
                eth_port->readDatagram(read_data.data(), read_data.size(), &src_addr, &src_port);

                if (src_addr == QHostAddress(port_name))
                {
                    break;
                }
                else
                {
                    read_data.clear();
                    cout << "VERBOSE: " << "frame received from an unexpected source:" << src_addr.toString().constData() << endl;
                    continue;
                }
            }
            else
            {
                //eth_port->close();
                cout << "ERROR: " << "no packet available" << endl;
                com_lock->unlock();
                return -1;
            }
        }
    }
    else
    {
        cout << "ERROR: " << "unknown com type" << endl;
        com_lock->unlock();
        return -1;
    }

    temp_string = read_data.constData();
    temp_string.chop(2);
    cout << "VERBOSE: " << "received command: " << temp_string.toLatin1().constData() << endl;

    if (false == read_data.startsWith("$CR"))
    {
        if (port_type == Upa_CommunicationLib_ComType)
        {
            com_port.close();
        }
        else if (port_type == Upa_CommunicationLib_EthType)
        {
            //eth_port->close();
        }
        cout << "WARNING " << "no correct response received (not our device?)" << endl;
        com_lock->unlock();
        return -1;
    }

    cout << "INFO: " << "connected correctly to target on: " << name.toLatin1().constData() << endl;
    cout << endl;

    is_open = true;

    com_lock->unlock();
    return 0;
}

int Upa_CommunicationLib::check_port()
{
    int data_length;
    QByteArray read_data;
    QByteArray write_data;
    unsigned char checksum;
    QString temp_string;

    cout << "INFO: " << "Checking Port:" << endl;

    com_lock->lock();
    // we don't check if it is open since we use this for reconnect as well
    //if (is_open == false)
    //{
    //    cout << "ERROR: " << "port is not open" << endl;
    //    com_lock->unlock();
    //    return -1;
    //}

    write_data.append("$CC");
    checksum = 0;
    for (int i = 1; i < write_data.size(); i++)
    {
        checksum = checksum ^ write_data[i];
    }
    write_data.append('*');
    write_data.append(QString("%1").arg(checksum, 2, 16, QLatin1Char('0')).toUtf8());
    write_data.append(0x0D);
    write_data.append(0x0A);

    temp_string = write_data.constData();
    temp_string.chop(2);
    cout << "VERBOSE: " << "sent command: " << temp_string.toLatin1().constData() << endl;

    if (port_type == Upa_CommunicationLib_ComType)
    {
        data_length = com_port.write(write_data);

        if (data_length == -1)
        {
            com_port.close();
            cout << "ERROR: " << "write failed" << endl;
            com_lock->unlock();
            return -1;
        }
        else if (data_length != write_data.size())
        {
            com_port.close();
            cout << "ERROR: " << "write failed to send all data" << endl;
            com_lock->unlock();
            return -1;
        }
        else if (false == com_port.waitForBytesWritten(500))
        {
            com_port.close();
            cout << "ERROR: " << "write timed out" << endl;
            com_lock->unlock();
            return -1;
        }
    }
    else if (port_type == Upa_CommunicationLib_EthType)
    {
        for (int i = write_data.size(); i < 18; i++)
        {
            write_data.append((char)0x00);
        }

        data_length = eth_port->writeDatagram(write_data.data(), write_data.size(), QHostAddress(port_name), 0xBEEF);
        if (data_length == -1)
        {
            //eth_port->close();
            cout << "ERROR: " << "write failed" << endl;
            com_lock->unlock();
            return -1;
        }
        else if (data_length != write_data.size())
        {
            //eth_port->close();
            cout << "ERROR: " << "write failed to send all data" << endl;
            com_lock->unlock();
            return -1;
        }
    }
    else
    {
        cout << "ERROR: " << "unknown com type" << endl;
        com_lock->unlock();
        return -1;
    }

    // check response
    if (port_type == Upa_CommunicationLib_ComType)
    {
        read_data = com_port.readAll();
        com_port.waitForReadyRead(50);
        for (int i=0; i < 32; i++)
        {
            if (read_data.endsWith(0x0A))
            {
                break;
            }

            if(0 != com_port.bytesAvailable())
            {
                read_data.append(com_port.readAll());
            }
            else
            {
                com_port.waitForReadyRead(1);
            }
        }

        if (com_port.error() == QSerialPort::ReadError)
        {
            //com_port.close();
            cout << "ERROR: " << "read failed" << endl;
            com_lock->unlock();
            return -1;
        }
        else if (com_port.error() == QSerialPort::TimeoutError && read_data.isEmpty())
        {
            //com_port.close();
            cout << "ERROR: " << "no response received" << endl;
            com_lock->unlock();
            return -1;
        }
    }
    else if (port_type == Upa_CommunicationLib_EthType)
    {
        while (1)
        {
            if (false == eth_port->waitForReadyRead(50))
            {
                //eth_port->close();
                cout << "ERROR: " << "no response received" << endl;
                com_lock->unlock();
                return -1;
            }

            if (true == eth_port->hasPendingDatagrams())
            {
                QHostAddress src_addr;
                quint16 src_port;

                read_data.resize(eth_port->pendingDatagramSize());
                eth_port->readDatagram(read_data.data(), read_data.size(), &src_addr, &src_port);

                if (src_addr == QHostAddress(port_name))
                {
                    break;
                }
                else
                {
                    read_data.clear();
                    //cout << "VERBOSE: " << "frame received from an unexpected source:" << src_addr.toString().constData() << endl;
                    continue;
                }
            }
            else
            {
                //eth_port->close();
                cout << "ERROR: " << "no packet available" << endl;
                com_lock->unlock();
                return -1;
            }
        }
    }
    else
    {
        cout << "ERROR: " << "unknown com type" << endl;
        com_lock->unlock();
        return -1;
    }

    temp_string = read_data.constData();
    temp_string.chop(2);
    cout << "VERBOSE: " << "received command: " << temp_string.toLatin1().constData() << endl;

    if (false == read_data.startsWith("$CR"))
    {
        if (port_type == Upa_CommunicationLib_ComType)
        {
            com_port.close();
        }
        else if (port_type == Upa_CommunicationLib_EthType)
        {
            //eth_port->close();
        }
        cout << "WARNING " << "no correct response received (not our device?)" << endl;
        com_lock->unlock();
        return -1;
    }

    is_open = true;
    cout << "INFO: " << "connection io" << endl;
    cout << endl;
    com_lock->unlock();

    return 0;
}


int Upa_CommunicationLib::close_port()
{
    cout << "INFO: " << "Closing Port:" << endl;

    com_lock->lock();
    if (is_open == false)
    {
        cout << "ERROR: " << "port is already closed" << endl;
        com_lock->unlock();
        return -1;
    }

    if (port_type == Upa_CommunicationLib_ComType)
    {
        com_port.close();
    }
    else if (port_type == Upa_CommunicationLib_EthType)
    {
        //eth_port->close();
    }

    cout << "INFO: " << "closing port successful" << endl;
    cout << endl;

    is_open = false;

    com_lock->unlock();
    return 0;
}

int Upa_CommunicationLib::write_reg(const unsigned int addr, unsigned int& data)
{
    int data_length;
    QByteArray temp_data;
    QByteArray read_data;
    QByteArray write_data;
    unsigned char checksum;
    QString temp_string;

    cout << "INFO: " << "Write Register:" << endl;

    com_lock->lock();
    if (is_open == false)
    {
        cout << "ERROR: " << "port is not open" << endl;
        com_lock->unlock();
        return -1;
    }

    write_data.append("$WC,");
    write_data.append(QString("0x%1").arg(addr, 8, 16, QLatin1Char('0')).toUtf8());
    write_data.append(',');
    write_data.append(QString("0x%1").arg(data, 8, 16, QLatin1Char('0')).toUtf8());
    checksum = 0;
    for (int i = 1; i < write_data.size(); i++)
    {
        checksum = checksum ^ write_data[i];
    }
    write_data.append('*');
    write_data.append(QString("%1").arg(checksum, 2, 16, QLatin1Char('0')).toUtf8());
    write_data.append(0x0D);
    write_data.append(0x0A);

    temp_string = write_data.constData();
    temp_string.chop(2);
    cout << "VERBOSE: " << "sent command: " << temp_string.toLatin1().constData() << endl;

    if (port_type == Upa_CommunicationLib_ComType)
    {
    data_length = com_port.write(write_data);

    if (data_length == -1)
    {
        cout << "ERROR: " << "write failed" << endl;
        com_lock->unlock();
        return -1;
    }
    else if (data_length != write_data.size())
    {
        cout << "ERROR: " << "write failed to send all data" << endl;
        com_lock->unlock();
        return -1;
    }
    else if (false == com_port.waitForBytesWritten(500))
    {
        cout << "ERROR: " << "write timed out" << endl;
        com_lock->unlock();
        return -1;
    }
    }
    else if (port_type == Upa_CommunicationLib_EthType)
    {
        for (int i = write_data.size(); i < 18; i++)
        {
            write_data.append((char)0x00);
        }

        data_length = eth_port->writeDatagram(write_data.data(), write_data.size(), QHostAddress(port_name), 0xBEEF);
        if (data_length == -1)
        {
            cout << "ERROR: " << "write failed" << endl;
            com_lock->unlock();
            return -1;
        }
        else if (data_length != write_data.size())
        {
            cout << "ERROR: " << "write failed to send all data" << endl;
            com_lock->unlock();
            return -1;
        }
    }
    else
    {
        cout << "ERROR: " << "unknown com type" << endl;
        com_lock->unlock();
        return -1;
    }

    // check response
    if (port_type == Upa_CommunicationLib_ComType)
    {
        read_data = com_port.readAll();
        com_port.waitForReadyRead(50);
        for (int i=0; i < 32; i++)
        {
            if (read_data.endsWith(0x0A))
            {
                break;
            }

            if(0 != com_port.bytesAvailable())
            {
                read_data.append(com_port.readAll());
            }
            else
            {
                com_port.waitForReadyRead(1);
            }
        }

        if (com_port.error() == QSerialPort::ReadError)
        {
            cout << "ERROR: " << "read failed" << endl;
            com_lock->unlock();
            return -1;
        }
        else if (com_port.error() == QSerialPort::TimeoutError && read_data.isEmpty())
        {
            cout << "ERROR: " << "no response received" << endl;
            com_lock->unlock();
            return -1;
        }
    }
    else if (port_type == Upa_CommunicationLib_EthType)
    {
        while (1)
        {
            if (false == eth_port->waitForReadyRead(50))
            {
                cout << "ERROR: " << "no response received" << endl;
                com_lock->unlock();
                return -1;
            }

            if (true == eth_port->hasPendingDatagrams())
            {
                QHostAddress src_addr;
                quint16 src_port;

                read_data.resize(eth_port->pendingDatagramSize());
                eth_port->readDatagram(read_data.data(), read_data.size(), &src_addr, &src_port);

                if (src_addr == QHostAddress(port_name))
                {
                    break;
                }
                else
                {
                    read_data.clear();
                    //cout << "VERBOSE: " << "frame received from an unexpected source:" << src_addr.toString().constData() << endl;
                    continue;
                }
            }
            else
            {
                cout << "ERROR: " << "no packet available" << endl;
                com_lock->unlock();
                return -1;
            }
        }
    }
    else
    {
        cout << "ERROR: " << "unknown com type" << endl;
        com_lock->unlock();
        return -1;
    }

    temp_string = read_data.constData();
    temp_string.chop(2);
    cout << "VERBOSE: " << "received command: " << temp_string.toLatin1().constData() << endl;

    temp_data.clear();
    temp_data.append("$WR,0x");
    temp_string = QString("%1").arg(addr, 8, 16, QLatin1Char('0'));
    temp_string = temp_string.toUpper();
    temp_data.append(temp_string.toUtf8());

    if (false == read_data.startsWith(temp_data))
    {
        cout << "ERROR: " << "no correct response received" << endl;
        com_lock->unlock();
        return -1;
    }

    checksum = 0;
    for (int i = 1; i < read_data.size(); i++)
    {
        if (read_data[i] == '*')
        {
            break;
        }
        else
        {
            checksum = checksum ^ read_data[i];
        }
    }

    temp_data.clear();
    temp_data.append(QString("%1").arg(checksum, 2, 16, QLatin1Char('0')).toUtf8());
    temp_data = temp_data.toUpper();
    temp_data.append(0x0D);
    temp_data.append(0x0A);
    if (false == read_data.endsWith(temp_data))
    {
        cout << "ERROR: " << "checksum wrong" << endl;
        cout << "checksum: " << temp_data.constData() << endl;
    }

    cout << endl;
    com_lock->unlock();
    return 0;
}

int Upa_CommunicationLib::read_reg(const unsigned int addr, unsigned int& data)
{
    int data_length;
    QByteArray temp_data;
    QByteArray read_data;
    QByteArray write_data;
    unsigned char checksum;
    QString temp_string;

    cout << "INFO: " << "Read Register:" << endl;

    com_lock->lock();
    if (is_open == false)
    {
        cout << "ERROR: " << "port is not open" << endl;
        com_lock->unlock();
        return -1;
    }

    write_data.append("$RC,");
    write_data.append(QString("0x%1").arg(addr, 8, 16, QLatin1Char('0')).toUtf8());
    checksum = 0;
    for (int i = 1; i < write_data.size(); i++)
    {
        checksum = checksum ^ write_data[i];
    }
    write_data.append('*');
    write_data.append(QString("%1").arg(checksum, 2, 16, QLatin1Char('0')).toUtf8());
    write_data.append(0x0D);
    write_data.append(0x0A);

    temp_string = write_data.constData();
    temp_string.chop(2);
    cout << "VERBOSE: " << "sent command: " << temp_string.toLatin1().constData() << endl;

    if (port_type == Upa_CommunicationLib_ComType)
    {
        data_length = com_port.write(write_data);

        if (data_length == -1)
        {
            cout << "ERROR: " << "write failed" << endl;
            com_lock->unlock();
            return -1;
        }
        else if (data_length != write_data.size())
        {
            cout << "ERROR: " << "write failed to send all data" << endl;
            com_lock->unlock();
            return -1;
        }
        else if (false == com_port.waitForBytesWritten(500))
        {
            cout << "ERROR: " << "write timed out" << endl;
            com_lock->unlock();
            return -1;
        }
    }
    else if (port_type == Upa_CommunicationLib_EthType)
    {
        for (int i = write_data.size(); i < 18; i++)
        {
            write_data.append((char)0x00);
        }

        data_length = eth_port->writeDatagram(write_data.data(), write_data.size(), QHostAddress(port_name), 0xBEEF);
        if (data_length == -1)
        {
            cout << "ERROR: " << "write failed" << endl;
            com_lock->unlock();
            return -1;
        }
        else if (data_length != write_data.size())
        {
            cout << "ERROR: " << "write failed to send all data" << endl;
            com_lock->unlock();
            return -1;
        }
    }
    else
    {
        cout << "ERROR: " << "unknown com type" << endl;
        com_lock->unlock();
        return -1;
    }

    // check response
    if (port_type == Upa_CommunicationLib_ComType)
    {
        read_data = com_port.readAll();
        com_port.waitForReadyRead(50);
        for (int i=0; i < 32; i++)
        {
            if (read_data.endsWith(0x0A))
            {
                break;
            }

            if(0 != com_port.bytesAvailable())
            {
                read_data.append(com_port.readAll());
            }
            else
            {
                com_port.waitForReadyRead(1);
            }
        }

        if (com_port.error() == QSerialPort::ReadError)
        {
            cout << "ERROR: " << "read failed" << endl;
            com_lock->unlock();
            return -1;
        }
        else if (com_port.error() == QSerialPort::TimeoutError && read_data.isEmpty())
        {
            cout << "ERROR: " << "no response received" << endl;
            com_lock->unlock();
            return -1;
        }
    }
    else if (port_type == Upa_CommunicationLib_EthType)
    {
        while (1)
        {
            if (false == eth_port->waitForReadyRead(50))
            {
                cout << "ERROR: " << "no response received" << endl;
                com_lock->unlock();
                return -1;
            }

            if (true == eth_port->hasPendingDatagrams())
            {
                QHostAddress src_addr;
                quint16 src_port;

                read_data.resize(eth_port->pendingDatagramSize());
                eth_port->readDatagram(read_data.data(), read_data.size(), &src_addr, &src_port);

                if (src_addr == QHostAddress(port_name))
                {
                    break;
                }
                else
                {
                    read_data.clear();
                    //cout << "VERBOSE: " << "frame received from an unexpected source:" << src_addr.toString().constData() << endl;
                    continue;
                }
            }
            else
            {
                cout << "ERROR: " << "no packet available" << endl;
                com_lock->unlock();
                return -1;
            }
        }
    }
    else
    {
        cout << "ERROR: " << "unknown com type" << endl;
        com_lock->unlock();
        return -1;
    }

    temp_string = read_data.constData();
    temp_string.chop(2);
    cout << "VERBOSE: " << "received command: " << temp_string.toLatin1().constData() << endl;

    temp_data.clear();
    temp_data.append("$RR,0x");
    temp_string = QString("%1").arg(addr, 8, 16, QLatin1Char('0'));
    temp_string = temp_string.toUpper();
    temp_data.append(temp_string.toUtf8());
    if (false == read_data.startsWith(temp_data))
    {
        cout << "ERROR: " << "no correct response received" << endl;
        com_lock->unlock();
        return -1;
    }


    checksum = 0;
    for (int i = 1; i < read_data.size(); i++)
    {
        if (read_data[i] == '*')
        {
            break;
        }
        else
        {
            checksum = checksum ^ read_data[i];
        }
    }

    temp_data.clear();
    temp_data.append(QString("%1").arg(checksum, 2, 16, QLatin1Char('0')).toUtf8());
    temp_data = temp_data.toUpper();
    temp_data.append(0x0D);
    temp_data.append(0x0A);
    if (false == read_data.endsWith(temp_data))
    {
        cout << "ERROR: " << "checksum wrong" << endl;
        cout << "checksum: " << temp_data.constData() << endl;
    }

    temp_data.clear();
    temp_data = read_data.mid(17,8);

    data = temp_data.toUInt(nullptr, 16);

    cout << endl;
    com_lock->unlock();
    return 0;
}

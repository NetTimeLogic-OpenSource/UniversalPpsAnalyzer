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

#ifndef UPA_COMMUNICATIONLIB_H
#define UPA_COMMUNICATIONLIB_H

#include <QSerialPortInfo>
#include <QSerialPort>
#include <QNetworkInterface>
#include <QUdpSocket>
#if QT_MAJOR_VERSION < 6
    #include <QMutex>
#else
    #include <QRecursiveMutex>
#endif
#include <QListWidgetItem>

#include <string>
using namespace std;

#define Upa_CommunicationLib_ComType    1
#define Upa_CommunicationLib_EthType    2

class Upa_CommunicationLib
{
    private:
        bool is_open;
        QSerialPort com_port;
        QUdpSocket* eth_port;
        QString port_name;
#if QT_MAJOR_VERSION < 6
        QMutex* com_lock;
#else
        QRecursiveMutex* com_lock;
#endif
        int detect_baudrate(void);

    public:
        Upa_CommunicationLib();
        ~Upa_CommunicationLib();

        int port_type;

        int open_port(QString name);
        int close_port();
        int check_port();
        int write_reg(const unsigned int addr, unsigned int& data);
        int read_reg(const unsigned int addr, unsigned int& data);
        QList<QString> create_eth_ports(QList<QString>);
        int destroy_eth_ports(void);
};

#endif // Upa_CommunicationLib_H

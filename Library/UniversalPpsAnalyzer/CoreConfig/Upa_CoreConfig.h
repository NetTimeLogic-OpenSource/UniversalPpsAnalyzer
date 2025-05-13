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

#ifndef UPA_CORECONFIG_H
#define UPA_CORECONFIG_H

#include <QString>
#include <string>
using namespace std;

#define Upa_CoreConfig_ConfSlaveCoreType                1
#define Upa_CoreConfig_ClkSignalTimestamperCoreType     4
#define Upa_CoreConfig_I2cConfigurationCoreType         10001
#define Upa_CoreConfig_IoConfigurationCoreType          10002

class Upa_CommunicationLib;

class Upa_CoreConfig
{
    public:
        unsigned int core_type;
        unsigned int core_instance_nr;
        unsigned int address_range_low;
        unsigned int address_range_high;
        unsigned int interrupt_mask;
        QString com_port;
        Upa_CommunicationLib* com_lib;
        long long* ref_offset;

        Upa_CoreConfig();
        ~Upa_CoreConfig();
};

#endif // Upa_CoreConfig_H

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
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    int open_gl;
    QApplication app(argc, argv);
    
    // you can disable OpenGL rendering from command line
    QCommandLineParser parser;
    QCommandLineOption NoOpenGlOption("NoOpenGL", QCoreApplication::translate("main", "Rendering of Graph without Open GL"));
    parser.addOption(NoOpenGlOption);
    parser.process(app);
    if (parser.isSet(NoOpenGlOption))
    {
        open_gl = 0;
        cout << "INFO: Don't use OpenGL for rendering" << endl;
    }
    else
    {
        open_gl = 1;
        cout << "INFO: Use OpenGL for rendering" << endl;
    }
    
    Upa_UniversalPpsAnalyzer gui(0, open_gl);

    gui.show();

    return app.exec();
}

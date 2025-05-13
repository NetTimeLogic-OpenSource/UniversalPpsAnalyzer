#-------------------------------------------------
#
# Project created by QtCreator 2016-12-14T14:32:07
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
QT       += charts
QT       += network
QT       += widgets
greaterThan(QT_MAJOR_VERSION, 6): QT += core5compat

TARGET = UniversalPpsAnalyzer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += CommunicationLib \
    CoreConfig \
    PpsTab \
    CalibrateTab \
    AdvancedTab \
    ConfigTab \
    DebugStream \
    HelpMenu

SOURCES += main.cpp\
    Upa_UniversalPpsAnalyzer.cpp \
    CommunicationLib/Upa_CommunicationLib.cpp \
    CoreConfig/Upa_CoreConfig.cpp \
    PpsTab/Upa_PpsConfigScreen.cpp \
    PpsTab/Upa_PpsThresholdScreen.cpp \
    PpsTab/Upa_PpsTab.cpp \
    CalibrateTab/Upa_CalibrateTab.cpp \
    AdvancedTab/Upa_AdvancedTab.cpp \
    ConfigTab/Upa_ConfigTab.cpp \
    HelpMenu/Upa_HelpMenuAbout.cpp

HEADERS  += Upa_UniversalPpsAnalyzer.h \
    CommunicationLib/Upa_CommunicationLib.h \
    CoreConfig/Upa_CoreConfig.h \
    PpsTab/Upa_PpsConfigScreen.h \
    PpsTab/Upa_PpsThresholdScreen.h \
    PpsTab/Upa_PpsTab.h \
    CalibrateTab/Upa_CalibrateTab.h \
    AdvancedTab/Upa_AdvancedTab.h \
    ConfigTab/Upa_ConfigTab.h \
    DebugStream/Upa_DebugStream.h \
    HelpMenu/Upa_HelpMenuAbout.h

FORMS    += Upa_UniversalPpsAnalyzer.ui \
    PpsTab/Upa_PpsConfigScreen.ui \
    PpsTab/Upa_PpsThresholdScreen.ui \
    PpsTab/Upa_PpsTab.ui \
    CalibrateTab/Upa_CalibrateTab.ui \
    AdvancedTab/Upa_AdvancedTab.ui \
    ConfigTab/Upa_ConfigTab.ui \
    HelpMenu/Upa_HelpMenuAbout.ui

RC_FILE = Upa_UniversalPpsAnalyzer.rc
RESOURCES = Upa_UniversalPpsAnalyzer.qrc

DISTFILES += \
    Upa_UniversalPpsAnalyzer.ico \
    Upa_UniversalPpsAnalyzer.rc

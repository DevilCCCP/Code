!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql

SOURCES += \
    Db.cpp \
    TableNamed.cpp \
    Table.cpp \
    ObjectType.cpp \
    ObjectState.cpp \
    Event.cpp \
    TableB.cpp \
    MapTable.cpp \
    MapTableB.cpp \
    ObjectSettings.cpp \
    ObjectSettingsType.cpp \
    StateInformer.cpp \
    ArmMonitors.cpp \
    MonitorLayouts.cpp \
    AmlCamMap.cpp \
    Variables.cpp \
    Report.cpp \
    ReportSend.cpp \
    ReportFilesMap.cpp \
    Files.cpp \
    VaStatDays.cpp \
    VaStatHours.cpp \
    VaStatType.cpp \
    VaStat.cpp \
    DbBackup.cpp \
    ObjectStateHours.cpp \
    DataTable.cpp \
    DbTransaction.cpp \
    ObjectLog.cpp \
    ObjectLogInfo.cpp \
    ObjectState2.cpp \
    Job.cpp

HEADERS += \
    Db.h \
    TableNamed.h \
    Table.h \
    ObjectType.h \
    ObjectState.h \
    Event.h \
    TableB.h \
    TableItem.h \
    MapTable.h \
    MapTableB.h \
    ObjectSettings.h \
    ObjectSettingsType.h \
    StateInformer.h \
    ArmMonitors.h \
    DbTable.h \
    MonitorLayouts.h \
    AmlCamMap.h \
    Variables.h \
    Report.h \
    ReportSend.h \
    ReportFilesMap.h \
    Files.h \
    VaStatDays.h \
    VaStatHours.h \
    VaStatType.h \
    VaStat.h \
    DbBackup.h \
    ObjectStateHours.h \
    DataTable.h \
    DbDef.h \
    DbTransaction.h \
    DbInline.h \
    ObjectLog.h \
    ObjectLogInfo.h \
    ObjectState2.h \
    DbTableA.h \
    Job.h


LIBS += \
    -lCrypto \
    -lCommon \
    -lLog


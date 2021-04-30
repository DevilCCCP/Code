!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql gui concurrent widgets


SOURCES += \
    Admin/AdminWidget.cpp \
    Admin/FormScript.cpp \
    Admin/ToolForm.cpp \
    Admin/ToolSchema.cpp \
    Admin/ActionIcons.cpp \
    Admin/DialogActions.cpp \
    Admin/PropertyForm.cpp \
    Admin/PropertyItemModel.cpp \
    Admin/PropertyVariantDelegate.cpp \
    ColumnEdit/ColumnEditInt.cpp \
    ColumnEdit/ColumnEditKey.cpp \
    ColumnEdit/ColumnEditKeyList.cpp \
    ColumnEdit/ColumnEditLine.cpp \
    ColumnEdit/ColumnEditReal.cpp \
    ColumnEdit/ColumnEditText.cpp \
    ColumnEdit/ColumnEditCheck.cpp \
    ColumnEdit/ColumnEditImage.cpp \
    ColumnEdit/ColumnEditPack.cpp \
    ColumnEdit/FormKeyList.cpp \
    ColumnEdit/FormEditPack.cpp \
    PropertyEdit/DialogText.cpp \
    PropertyEdit/FormEditBool.cpp \
    PropertyEdit/FormEditInt.cpp \
    PropertyEdit/FormEditReal.cpp \
    PropertyEdit/FormEditSize.cpp \
    PropertyEdit/FormEditText.cpp \
    PropertyEdit/FormEditTimePeriod.cpp \
    PropertyEdit/FormEditTimeRange.cpp \
    PropertyEdit/FormEditVariant.cpp \
    PropertyEdit/TimeEdit2.cpp \
    Tree/TreeItemA.cpp \
    Tree/TreeItemB.cpp \
    Tree/TreeItemObject.cpp \
    Tree/TreeItemStandard.cpp \
    Tree/TreeModelA.cpp \
    Tree/TreeModelB.cpp \
    Tree/TreeSchema.cpp \
    Filters/FormTableTime.cpp \
    DialogDbEditSelect.cpp \
    EventModel.cpp \
    AdminWindow.cpp \
    FormDbBackup.cpp \
    FormDbEditor.cpp \
    DbBackupThread.cpp \
    FormStatistics.cpp \
    FormTable.cpp \
    FormTableEdit.cpp \
    MonitoringWindow.cpp \
    ObjectModel.cpp \
    QLineEdit2.cpp \
    StatisticsLoader.cpp \
    FormTableEditAdapterB.cpp \
    FormObjectLog.cpp \
    ObjectLogWidget.cpp \
    ColumnEdit/ColumnEditSwitch.cpp \
    ObjectItemModel.cpp


HEADERS += \
    Admin/AdminWidget.h \
    Admin/FormScript.h \
    Admin/ToolAction.h \
    Admin/ToolForm.h \
    Admin/ToolSchema.h \
    Admin/ActionIcons.h \
    Admin/DialogActions.h \
    Admin/PropertyForm.h \
    Admin/PropertyItemModel.h \
    Admin/PropertyVariantDelegate.h \
    ColumnEdit/ColumnEditA.h \
    ColumnEdit/ColumnEditInt.h \
    ColumnEdit/ColumnEditKey.h \
    ColumnEdit/ColumnEditKeyList.h \
    ColumnEdit/ColumnEditLine.h \
    ColumnEdit/ColumnEditReal.h \
    ColumnEdit/ColumnEditText.h \
    ColumnEdit/ColumnEditCheck.h \
    ColumnEdit/ColumnEditImage.h \
    ColumnEdit/ColumnEditPack.h \
    ColumnEdit/FormKeyList.h \
    ColumnEdit/FormEditPack.h \
    PropertyEdit/DialogText.h \
    PropertyEdit/FormEditBool.h \
    PropertyEdit/FormEditInt.h \
    PropertyEdit/FormEditReal.h \
    PropertyEdit/FormEditSize.h \
    PropertyEdit/FormEditText.h \
    PropertyEdit/FormEditTimePeriod.h \
    PropertyEdit/FormEditTimeRange.h \
    PropertyEdit/FormEditVariant.h \
    PropertyEdit/TimeEdit2.h \
    Tree/DbTreeSchema.h \
    Tree/TreeItemA.h \
    Tree/TreeItemB.h \
    Tree/TreeItemObject.h \
    Tree/TreeItemStandard.h \
    Tree/TreeModelA.h \
    Tree/TreeModelB.h \
    Tree/TreeSchema.h \
    Filters/FormTableTime.h \
    AdminWindow.h \
    DialogDbEditSelect.h \
    EventModel.h \
    FormDbBackup.h \
    FormDbEditor.h \
    DbBackupThread.h \
    DbTableModel.h \
    FormStatistics.h \
    FormTable.h \
    FormTableAdapter.h \
    FormTableEdit.h \
    FormTableEditAdapter.h \
    MonitoringWindow.h \
    ObjectModel.h \
    QLineEdit2.h \
    StatisticsLoader.h \
    TableModel.h \
    Def.h \
    FormTableEditAdapterB.h \
    TableEditSchema.h \
    FormObjectLog.h \
    ObjectLogWidget.h \
    LogSchema.h \
    DbTableAutoModel.h \
    ColumnEdit/ColumnEditSwitch.h \
    ObjectItemModel.h

FORMS += \
    Admin/FormScript.ui \
    Admin/ToolForm.ui \
    Admin/DialogActions.ui \
    Admin/PropertyForm.ui \
    ColumnEdit/FormKeyList.ui \
    ColumnEdit/FormEditPack.ui \
    PropertyEdit/DialogText.ui \
    PropertyEdit/FormEditBool.ui \
    PropertyEdit/FormEditInt.ui \
    PropertyEdit/FormEditReal.ui \
    PropertyEdit/FormEditSize.ui \
    PropertyEdit/FormEditText.ui \
    PropertyEdit/FormEditTimePeriod.ui \
    PropertyEdit/FormEditTimeRange.ui \
    Filters/FormTableTime.ui \
    DialogDbEditSelect.ui \
    FormDbBackup.ui \
    AdminWindow.ui \
    FormDbEditor.ui \
    FormStatistics.ui \
    FormTable.ui \
    FormTableEdit.ui \
    MonitoringWindow.ui \
    FormObjectLog.ui


TRANSLATIONS += DbUi_ru.ts
#"/usr/lib/qt5/bin/lrelease" '/home/devil/!Code/!Code/Lib/DbUi/DbUi.pro'

LIBS += \
    -lDb \
    -lUi \
    -lLog \
    -lCommon

RESOURCES += \
    DbUi.qrc


#pragma once

#include <Lib/Include/Common.h>


#define DefineDbClassS(NAME) \
  DefineClassS(NAME##Table); \
  DefineClassS(NAME)


#define DefineViewClassS(NAME) \
  DefineClassS(NAME##View); \
  DefineClassS(NAME##Row)

#define UseDbTables \
  UseDbTableOne(Object) \
  UseDbTableOne(ObjectType) \
  \
  UseDbTableOne(ObjectSettings) \
  UseDbTableOne(ObjectSettingsType) \
  UseDbTableOne(ObjectState) \
  UseDbTableOne(ObjectStateValues) \
  UseDbTableOne(ObjectStateHours) \
  \
  UseDbTableOne(ObjectLog) \
  \
  UseDbTableOne(Files) \
  \
  UseDbTableOne(EventType) \
  UseDbTableOne(Event) \
  \
  UseDbTableOne(VaStatType) \
  UseDbTableOne(VaStat) \
  UseDbTableOne(VaStatHours) \
  UseDbTableOne(VaStatDays) \
  \
  UseDbTableOne(Report) \
  UseDbTableOne(ReportSend) \
  \
  UseDbTableOne(ArmMonitors) \
  UseDbTableOne(MonitorLayouts) \
  UseDbTableOne(AmlCamMap) \
  \
  UseDbTableOne(Variables)

#define UseDbTableOne(XXX) DefineDbClassS(XXX);
UseDbTables
#undef UseDbTableOne

DefineClassS(ObjectItem);
DefineClassS(ObjectTypeItem);
DefineClassS(ReportFilesMap);

#define UseDbViews \
//  UseDbViewOne(NewView)

#define UseDbViewOne(XXX) DefineViewClassS(XXX);
UseDbViews
#undef UseDbViewOne

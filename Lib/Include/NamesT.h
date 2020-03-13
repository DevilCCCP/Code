#pragma once

#define PROGRAM_ABBR "yax"
const char gOrganizationName[] = "X-Org";
const char gProgramName[] = "X-Program";

#define DefineModuleName_(module, exe, prefix) \
  const char k##module[] = prefix "_" exe;\
  const char k##module##Exe[] = "./" prefix "_" exe ".exe"
#define DefineModuleName(module, exe) DefineModuleName_(module, #exe, PROGRAM_ABBR)

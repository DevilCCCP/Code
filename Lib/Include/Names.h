#pragma once

#ifndef PROGRAM_ABBR
#error PROGRAM_ABBR must be defined
#endif

#define MAKE_STRING_(A) #A
#define MAKE_STRING(A) MAKE_STRING_(A)

#define DefineModuleName_(module, exe, prefix) \
  const char k##module[] = MAKE_STRING_(prefix) "_" exe;\
  const char k##module##Exe[] = "./" MAKE_STRING_(prefix) "_" exe ".exe"
#define DefineModuleName(module, exe) DefineModuleName_(module, #exe, PROGRAM_ABBR)

const char kLicenseExe[] = "./License.exe";
DefineModuleName(LicenseLoader, lilo);

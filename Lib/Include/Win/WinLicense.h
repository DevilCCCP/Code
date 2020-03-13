#pragma once

#include <QThread>
#define _WIN32_DCOM
#include <comdef.h>
#include <Wbemidl.h>
#undef SendMessage

#include <QByteArray>
#include <QString>

#include "../License_h.h"

inline QByteArray GetSn(bool gui)
{
  //
  // MSDN Example: Getting WMI Data from the Local Computer
  //
  HRESULT res;

  // Step 1: --------------------------------------------------
  // Initialize COM. ------------------------------------------

  QByteArray result("result:\n");
  result.append(LICENSE_PREFIX);

  forever {
    if (!gui) {
      res = CoInitializeEx(0, COINIT_MULTITHREADED);
    } else {
      break;
    }
    if (!FAILED(res)) {
      break;
    } else if (res == (HRESULT)0x80010106) {
      CoUninitialize();
    }
    QThread::msleep(10);
  }
  result.append('1');

  // Step 2: --------------------------------------------------
  // Set general COM security levels --------------------------
  forever {
    res = CoInitializeSecurity(
          NULL,
          -1,                          // COM authentication
          NULL,                        // Authentication services
          NULL,                        // Reserved
          RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
          RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
          NULL,                        // Authentication info
          EOAC_NONE,                   // Additional capabilities
          NULL                         // Reserved
          );

    if (!FAILED(res)) {
      break;
    }
    QThread::msleep(10);
  }
  result.append('2');

  // Step 3: ---------------------------------------------------
  // Obtain the initial locator to WMI -------------------------
  IWbemLocator *pLoc = NULL;
  forever {
    res = CoCreateInstance(
          CLSID_WbemLocator,
          0,
          CLSCTX_INPROC_SERVER,
          IID_IWbemLocator, (LPVOID *) &pLoc);

    if (!FAILED(res)) {
      if (pLoc) {
        break;
      }
    }
    QThread::msleep(10);
  }
  result.append('3');

  // Step 4: -----------------------------------------------------
  // Connect to WMI through the IWbemLocator::ConnectServer method
  IWbemServices *pSvc = NULL;
  // Connect to the root\cimv2 namespace with
  // the current user and obtain pointer pSvc
  // to make IWbemServices calls.
  forever {
    res = pLoc->ConnectServer(
          bstr_t(L"ROOT\\CIMV2"),   // Object path of WMI namespace
          NULL,                    // User name. NULL = current user
          NULL,                    // User password. NULL = current
          NULL,                    // Locale. NULL indicates current
          0,                       // Security flags.
          NULL,                    // Authority (for example, Kerberos)
          NULL,                    // Context object
          &pSvc                    // pointer to IWbemServices proxy
          );

    if (!FAILED(res)) {
      break;
    }
    QThread::msleep(10);
  }
  result.append('4');

  // Step 5: --------------------------------------------------
  // Set security levels on the proxy -------------------------
  forever {
    res = CoSetProxyBlanket(
          pSvc,                        // Indicates the proxy to set
          RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
          RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
          NULL,                        // Server principal name
          RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
          RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
          NULL,                        // client identity
          EOAC_NONE                    // proxy capabilities
          );

    if (!FAILED(res)) {
      break;
    }
    QThread::msleep(10);
  }
  result.append('5');

  // Step 6: --------------------------------------------------
  // Use the IWbemServices pointer to make requests of WMI ----
  IEnumWbemClassObject* pEnumerator = NULL;
  forever {
    res = pSvc->ExecQuery(
          bstr_t(L"WQL"),
          bstr_t(L"SELECT * FROM Win32_ComputerSystemProduct"),
          WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
          NULL,
          &pEnumerator);

    if (!FAILED(res)) {
      break;
    }
    QThread::msleep(10);
  }
  result.append('6');

  // Step 7: -------------------------------------------------
  // Get the data from the query in step 6 -------------------
  result.append(">> ");
  while (pEnumerator) {
    IWbemClassObject *pclsObj;
    ULONG uReturn = 0;
    pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

    if (0 == uReturn) {
      break;
    }

    VARIANT vtProp;

    const char* vars[] = { "Caption"
                           , "Description"
                           , "IdentifyingNumber"
                           , "Name"
                           , "SKUNumber"
                           , "UUID"
                           , "Vendor"
                           , "Version"
                           , ""
                         };
    for (int i = 0; *vars[i]; i++) {
      QString name = QString(vars[i]);
      pclsObj->Get((const wchar_t*)name.utf16(), 0, &vtProp, 0, 0);
      QString add = QString("\n%1=%2").arg(name).arg(QString((QChar*)vtProp.bstrVal));
      result.append(add.toUtf8());
      VariantClear(&vtProp);
    }
    pclsObj->Release();
  }

  // Cleanup
  // ========
  pSvc->Release();
  pLoc->Release();
  pEnumerator->Release();
  if (!gui) {
    CoUninitialize();
  }

  return result;
}

inline QByteArray GetSnShort(int iv)
{
  //
  // MSDN Example: Getting WMI Data from the Local Computer
  //
  HRESULT res = iv;

  // Step 1: --------------------------------------------------
  // Initialize COM. ------------------------------------------

  QByteArray result("result:\n");
  result.append(LICENSE_PREFIX);
  res = CoInitializeEx(0, COINIT_MULTITHREADED);
  bool coInit = res == 0;
  res = 0;

  result.append('1');
  if (FAILED(res)) {
    return result;
  }

  // Step 2: --------------------------------------------------
  // Set general COM security levels --------------------------
  if (coInit) {
    res = CoInitializeSecurity(
          NULL,
          -1,                          // COM authentication
          NULL,                        // Authentication services
          NULL,                        // Reserved
          RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
          RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
          NULL,                        // Authentication info
          EOAC_NONE,                   // Additional capabilities
          NULL                         // Reserved
          );
  }
  res = 0;

  result.append('2');
  if (FAILED(res)) {
    if (coInit) {
      CoUninitialize();
    }
    return result;
  }

  // Step 3: ---------------------------------------------------
  // Obtain the initial locator to WMI -------------------------
  IWbemLocator *pLoc = NULL;
  res = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID *) &pLoc);

  result.append('3');
  if (FAILED(res)) {
    QByteArray r((const char*)&res, 4);
    result.append(r.toHex());
    if (coInit) {
      CoUninitialize();
    }
    return result;
  }

  // Step 4: -----------------------------------------------------
  // Connect to WMI through the IWbemLocator::ConnectServer method
  IWbemServices *pSvc = NULL;
  // Connect to the root\cimv2 namespace with
  // the current user and obtain pointer pSvc
  // to make IWbemServices calls.
  res = pLoc->ConnectServer(
        bstr_t(L"ROOT\\CIMV2"),   // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        NULL,                    // Locale. NULL indicates current
        0,                       // Security flags.
        NULL,                    // Authority (for example, Kerberos)
        NULL,                    // Context object
        &pSvc                    // pointer to IWbemServices proxy
        );

  result.append('4');
  if (FAILED(res)) {
    pLoc->Release();
    QByteArray r((const char*)&res, 4);
    result.append(r.toHex());
    if (coInit) {
      CoUninitialize();
    }
    return result;
  }

  // Step 5: --------------------------------------------------
  // Set security levels on the proxy -------------------------
  res = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities
        );

  result.append('5');
  if (FAILED(res)) {
    pSvc->Release();
    pLoc->Release();
    QByteArray r((const char*)&res, 4);
    result.append(r.toHex());
    if (coInit) {
      CoUninitialize();
    }
    return result;
  }

  // Step 6: --------------------------------------------------
  // Use the IWbemServices pointer to make requests of WMI ----
  IEnumWbemClassObject* pEnumerator = NULL;
  res = pSvc->ExecQuery(
        bstr_t(L"WQL"),
        bstr_t(L"SELECT * FROM Win32_ComputerSystemProduct"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

  result.append('6');
  if (FAILED(res)) {
    pSvc->Release();
    pLoc->Release();
    QByteArray r((const char*)&res, 4);
    result.append(r.toHex());
    if (coInit) {
      CoUninitialize();
    }
    return result;
  }

  // Step 7: -------------------------------------------------
  // Get the data from the query in step 6 -------------------
  result.append(">> ");
  while (pEnumerator) {
    IWbemClassObject *pclsObj;
    ULONG uReturn = 0;
    pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

    if (0 == uReturn) {
      int line = ((iv & 0xffff) % kLcCount) + 1;
      result.append((char*)&line, 4);
      break;
    }

    VARIANT vtProp;

    const char* vars[] = { "C`nqejh"
                           , "Ddq`ndjmafd"
                           , "Icckpd`rae]Ci`TVb"
                           , "N`kb"
                           , "SJSKqh\\^j"
                           , "UTGA"
                           , "Vdlakm"
                           , "Vdppejh"
                           , ""
                         };
    for (int i = 0; *vars[i]; i++) {
      QByteArray n(vars[i]);
      for (int j = 0; j < n.size(); j++) {
        n[j] = (char)(n[j] + j);
      }
      QString name = QString(n);
      pclsObj->Get((const wchar_t*)name.utf16(), 0, &vtProp, 0, 0);
      QString add = QString("\n%1=%2").arg(name).arg(QString((QChar*)vtProp.bstrVal));
      result.append(add.toUtf8());
      VariantClear(&vtProp);
    }
    pclsObj->Release();
  }

  // Cleanup
  // ========
  pSvc->Release();
  pLoc->Release();
  pEnumerator->Release();

  if (coInit) {
    CoUninitialize();
  }
  return result;
}

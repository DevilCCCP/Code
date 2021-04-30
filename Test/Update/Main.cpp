#include <qsystemdetection.h>
#ifdef Q_OS_UNIX
#include <signal.h>
#endif

#include <Lib/Log/Log.h>
#include <Lib/Include/QtAppCon.h>
#include <Lib/Updater/Installer.h>


#ifdef Q_OS_UNIX
static void StopSignalHandler(int)
{
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = SIG_DFL;
  ::sigaction(SIGTERM, &action, nullptr);
}
#endif

int qmain(int argc, char* argv[])
{
#ifdef Q_OS_UNIX
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = StopSignalHandler;
  ::sigaction(SIGTERM, &action, nullptr);
#endif

  return Installer::Exec(argc, argv);
}

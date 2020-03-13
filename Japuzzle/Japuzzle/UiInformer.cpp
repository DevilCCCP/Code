#include "UiInformer.h"
#include "MainWindow.h"


void UiInformer::Info(const QString& text)
{
  mMainWindow->Info(text);
}

void UiInformer::Warning(const QString& text)
{
  mMainWindow->Warning(text);
}

void UiInformer::Error(const QString& text)
{
  mMainWindow->Error(text);
}


UiInformer::UiInformer(MainWindow* _MainWindow)
  : mMainWindow(_MainWindow)
{
}

UiInformer::~UiInformer()
{
}

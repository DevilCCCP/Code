#include <QApplication>
#include <QDir>
#include <QFile>
#include <QPixmap>

#include "Style.h"


bool Style::Load(const StyleInfo& info)
{
  QDir dir(qApp->applicationDirPath());
  dir.cd("Styles");
  dir.cd(info.Name);
  if (!dir.exists()) {
    return false;
  }

  MakeColorFromInt(info.BackColor, mBackColor);
  MakeColorFromInt(info.LineColor, mLineColor);
  MakeColorFromInt(info.HighlightColor, mHighlightColor);
  MakeColorFromInt(info.DigitColor1, mDigitWhiteColor);
  MakeColorFromInt(info.DigitColor2, mDigitBlackColor);
  MakeColorFromInt(info.DigitColor3, mDigitRedColor);
  MakeColorFromInt(info.PreviewColorY, mYesColor);
  MakeColorFromInt(info.PreviewColorN, mNoColor);
  MakeColorFromInt(info.PreviewColorY1, mYes1Color);
  MakeColorFromInt(info.PreviewColorN1, mNo1Color);
  MakeColorFromInt(info.PreviewLine, mPreviewLineColor);
  mFont = info.FontName;

  mBackground.load(dir.absoluteFilePath("Fon.png"));
  mYes.resize(4);
  mNo.resize(4);
  for (int i = 0; i < 4; i++) {
    mYes[i].load(dir.absoluteFilePath(QString("Yes%1.png").arg(i)));
    mNo[i].load(dir.absoluteFilePath(QString("No%1.png").arg(i)));
  }
  mProp.load(dir.absoluteFilePath("Prop.png"));
  mSpot.load(dir.absoluteFilePath("Spot.png"));
  mNull.load(dir.absoluteFilePath("Null.png"));

  mCursorArrow = QCursor(Qt::ArrowCursor);
  mCursorYes.clear();
  mCursorNo.clear();
  for (int i = 0; i < 4; i++) {
    mCursorYes.append(QCursor(QPixmap(dir.absoluteFilePath(QString("CursorYes%1.png").arg(i, 2, 2, QChar('0'))))
                              , info.CursorYesSpot.value(i).x(), info.CursorYesSpot.value(i).y()));
    mCursorNo.append(QCursor(QPixmap(dir.absoluteFilePath(QString("CursorNo%1.png").arg(i, 2, 2, QChar('0'))))
                             , info.CursorNoSpot.value(i).x(), info.CursorNoSpot.value(i).y()));
  }
  mCursorErase = QCursor(QPixmap(dir.absoluteFilePath(QString("CursorErase.png")))
                        , info.CursorESpot.x(), info.CursorESpot.y());
  return true;
}

void Style::MakeColorFromInt(uint colorHex, QColor& color)
{
  int alpha = (colorHex >> 24) & 0xff;
  if (alpha) {
    color.setRgb((colorHex) & 0xff, (colorHex >> 8) & 0xff, (colorHex >> 16) & 0xff, alpha);
  } else {
    color.setRgb((colorHex) & 0xff, (colorHex >> 8) & 0xff, (colorHex >> 16) & 0xff);
  }
}


Style::Style()
{
}


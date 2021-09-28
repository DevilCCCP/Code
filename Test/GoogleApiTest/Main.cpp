#include <QCoreApplication>
#include <QFile>
#include <QDebug>

#include <Lib/GoogleApi/GoogleApi.h>
#include <Lib/GoogleApi/GDrive.h>
#include <Lib/GoogleApi/GSheet.h>


const int kTestDrive = false;
const int kTestSheet = true;

void ShowUsage()
{
  qDebug() << "Usage: exe google_app_json_path";
}

int main(int argc, char* argv[])
{
  QCoreApplication a(argc, argv);

  if (qApp->arguments().size() < 2) {
    ShowUsage();
    return 1;
  }
  QString filePath(qApp->arguments()[1]);
  if (!QFile::exists(filePath)) {
    ShowUsage();
    return 2;
  }

  GoogleApi api;
  if (!api.LoadFromJson(filePath)) {
    qDebug() << "Load settings json fail";
    return 10;
  }

  api.SetScopes(GoogleApi::DriveScope | GoogleApi::SheetScope);
  api.SetDebug(true);
  printf("Api\n");
  printf("authorize .. ");
  if (!api.Authorize()) {
    printf("fail\n");
    return 20;
  }

  if (kTestDrive) {
    printf("ok\nDrive\n");
    printf("send .. ");
    QByteArray testText("Test text");
//    QString fileId;
//    if (!api.Drive()->UploadSimple(testText, &fileId)) {
//      printf("fail\n");
//      return 20;
//    }
    QString fileId = "1ZWMBidEJ3aA0b0Es8DdpWPyhehBMSXm0FT3APrxC2U8";
    printf("%s\nshare .. ", qPrintable(fileId));
    if (!api.Drive()->Share(fileId)) {
      printf("fail\n");
      return 23;
    }
    printf("%s\nreceive .. ", qPrintable(fileId));
    QByteArray data;
    if (!api.Drive()->DownloadSimple(fileId, &data) || data != testText) {
      printf("fail\n");
      return 21;
    }
    api.Drive()->DownloadParams(fileId, &data);
  }

  if (kTestSheet) {
    printf("ok\nSheet\n");
    QString fileId;
//    if (!api.Sheet()->CreateSimple("Sample sheet", &fileId)) {
//      printf("fail\n");
//      return 30;
//    }
    fileId = "1APH7uY_cTtdh89_hsu2LiD9dVurigjqbC36HdONfVFM";
    printf("%s\nappend .. ", qPrintable(fileId));
    if (!api.Sheet()->AppendRow(fileId, QStringList() << "123" << "231" << "312")) {
      printf("fail\n");
      return 31;
    }
//    printf("ok\ndownload .. ");
//    QByteArray data;
//    if (!api.Sheet()->Download(fileId, &data)) {
//      printf("fail\n");
//      return 32;
//    }
  }
  printf("ok\nFinished\n");
  return 0;
}


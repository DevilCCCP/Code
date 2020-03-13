#include "TestBigModel.h"


bool TestBigModel::Prepare()
{
  mDb.reset(new Db());
  if (!mDb->OpenDefault() || !mDb->Connect()) {
    return false;
  }
  mTestBigtable.reset(new TestBigTable(mDb));
  return true;
}

bool TestBigModel::Do()
{
  QList<TestBigS> list;
  mTestBigtable->Select("", list);

  int result = 0;
  for (auto itr = list.begin(); itr != list.end(); itr++) {
    const TestBig* item = (*itr).data();
    for (int j = 0; j < 16; j++) {
      result += item->mData[j].size();
    }
  }

  printf("%i\n", result);
  return true;
}


bool TestBigModelIns::Prepare()
{
  mDb.reset(new Db());
  if (!mDb->OpenDefault() || !mDb->Connect()) {
    return false;
  }
  mTestBigtable.reset(new TestBigTable(mDb));

  for (int i = 0; i < 100; i++) {
    QString value;
    int length = 200 + (qrand() % 1200);
    for (int j = 0; j < length; j++) {
      value.append(QChar((char)('a' + (qrand() % 20))));
    }
    mTestData.append(value);
  }
  return true;
}

bool TestBigModelIns::Do()
{
  QList<TestBigS> items;
  for (int i = 0; i < mCount; i++) {
    TestBigS item(new TestBig());
    for (int j = 0; j < 16; j++) {
      item->mData[j] = mTestData[(qrand() % mTestData.size())];
    }
    items.append(item);
  }

  return mTestBigtable->InsertPack(items, 50);
}

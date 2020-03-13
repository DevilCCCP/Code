#pragma once

#include <QList>
#include <QMutex>
#include <QMutexLocker>

#include "Common.h"


template<typename TypeT>
class Pool<TypeT>
{
  QMutex        mMutex;
  QList<TypeT>  mFreePool;

public:
  void AddItem(const TypeT& item)
  {
    QMutexLocker lock(&mMutex);
    mFreePool.append(item);
  }

  bool Take(Container& container)
  {
    QMutexLocker lock(&mMutex);
    if (!mFreePool.isEmpty()) {
      container(this, mFreePool.takeFirst());
    }
    return false;
  }

private:
  void Return(const TypeT& item)
  {
    QMutexLocker lock(&mMutex);
    mFreePool.append(item);
  }

  friend class PoolHand;
};

template<typename TypeT>
class PoolHand {
  Pool<TypeT>* mParent;
  bool         mTaken;
  TypeT        mItem;

public:
  const TypeT& operator*() const { return Item; }

public:
  PoolHand(Pool<TypeT> _Parent = nullptr)
    : mParent(_Parent), mTaken(false)
  { }
  ~PoolHand()
  {
    if (mParent && mTaken) {
      mParent->Return(mItem);
    }
  }
};

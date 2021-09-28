#pragma once

#include <QSharedPointer>
#include <QWeakPointer>


class FatalException
{

public:
  FatalException()
  { }
};

#define DefineClassS(type) class type; typedef QSharedPointer<type> type##S; typedef QWeakPointer<type> type##W
#define DefineStructS(type) struct type; typedef QSharedPointer<type> type##S; typedef QWeakPointer<type> type##W

#define PROTECTED_SGET(TypeValue) \
  private: TypeValue##S m##TypeValue; \
  protected: const TypeValue##S & get##TypeValue() const { return m##TypeValue; } \
  private:

#define PROTECTED_GET(Type, Value) \
  private: Type m##Value; \
  protected: const Type & get##Value() const { return m##Value; } \
  private:

#define PROPERTY_GET(Type, Value) \
  private: Type m##Value; \
  public: const Type & get##Value() const { return m##Value; } \
  private:

#define PROPERTY_SGET(TypeValue) \
  private: TypeValue##S m##TypeValue; \
  public: const TypeValue##S & get##TypeValue() const { return m##TypeValue; } \
  private:

#define PROPERTY_GET_SET(Type, Value) \
  private: Type m##Value; \
  public: const Type & get##Value() const { return m##Value; } \
  public: void set##Value(const Type & value) { m##Value = value; } \
  protected: Type & ref##Value() { return m##Value; } \
  private:

#define PROPERTY_SGET_DEMAND(TypeValue) \
  private: mutable TypeValue##S m##TypeValue; \
  public: const TypeValue##S & get##TypeValue() const; \
  private:

#define PROPERTY_SGET_DEMAND_INST(BaseClass, TypeValue, ...) \
const TypeValue##S & BaseClass::get##TypeValue() const \
{ \
  if (!m##TypeValue) { \
    m##TypeValue.reset(new TypeValue(__VA_ARGS__)); \
  } \
  return m##TypeValue; \
}

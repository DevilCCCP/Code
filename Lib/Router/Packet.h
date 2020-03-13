#pragma once

#include <Lib/Include/Common.h>


enum PacketType {
  eHello,
  eGrant,
  eConnection,
  eDisconnection,
  eClientData,
  eServerData,
  ePacketIllegal,
};

class Packet
{
  PROPERTY_GET_SET(PacketType, Type)
  PROPERTY_GET_SET(int,        Id)
  PROPERTY_GET_SET(QByteArray, Data)
  ;
public:
  static int HeaderSize();

  bool HasHeader();
  int MinimumSize();
  void Clear();

  bool ReadHeader(QDataStream* stream);
  bool ReadData(QDataStream* stream);
  bool Write(QDataStream* stream) const;

public:
  Packet();
};

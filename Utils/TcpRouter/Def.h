#pragma once

#include <QByteArray>


const QByteArray kRecepientMagic = QByteArray::fromHex("13666613661366");
const QByteArray kRecepientGrant = QByteArray::fromHex("1366661366136600");

enum PacketType {
  eGrant = 0,
  eConnection,
  eDisconnection,
  eClientData,
  eServerData,
};

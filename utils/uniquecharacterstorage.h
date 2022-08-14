#ifndef UNIQUECHARACTERSTORAGE_H
#define UNIQUECHARACTERSTORAGE_H

#include <QObject>
#include <QQmlListProperty>
#include "game/character.h"
#include "game/level/grid.h"
#include "game/leveltask.h"

class StorageSlot;
class QJsonObject;

class UniqueCharacterStorage : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QMap<QString,QList<StorageSlot*>> levelToStorage MEMBER levelToStorage CONSTANT)

public:
  explicit UniqueCharacterStorage(QObject *parent = nullptr);

  Q_INVOKABLE int saveUniqueCharactersFromLevel(LevelTask* level);
  Q_INVOKABLE int loadUniqueCharactersToLevel(LevelTask* level);
  Q_INVOKABLE bool loadCharacterToCurrentLevel(QString characterSheet, int x, int y, int z);
  Q_INVOKABLE void log();

  void load(const QJsonObject&);
  void save(QJsonObject&);
private:
  bool loadCharacterIntoLevel(LevelTask* level, StorageSlot* characterSlot);
  bool loadCharacterIntoLevel(LevelTask* level, StorageSlot* characterSlot, Point position);


signals:

private:
  QMap<QString,QList<StorageSlot*>> levelToStorage;

};

// Contains character and relative information neccessary to spawn it
class StorageSlot : public QObject
{
  Q_OBJECT

  Q_PROPERTY(Character* storedCharacter MEMBER storedCharacter)
  Q_PROPERTY(long storedTimestampAtStorage MEMBER storedTimestampAtStorage)

public:
  explicit StorageSlot(QObject *parent = nullptr, Character* character = nullptr, long timestampAtStorage = 0);

  void load(const QJsonObject&);
  void save(QJsonObject&);

public:
  Character* storedCharacter;
  long storedTimestampAtStorage;
};

#endif // UNIQUECHARACTERSTORAGE_H

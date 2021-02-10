#ifndef  LEVELTASK_H
# define LEVELTASK_H

# include <QObject>
# include <QTimer>
# include "tilemap/tilemap.h"
# include "levelgrid.h"

# include "character.h"
# include "dynamicobject.h"

class CharacterDialog;

class LevelTask : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QString    name MEMBER name)
  Q_PROPERTY(bool       paused  MEMBER paused NOTIFY pausedChanged)
  Q_PROPERTY(TileMap*   tilemap READ getTileMap NOTIFY tilemapReady)
  Q_PROPERTY(LevelGrid* grid    MEMBER grid)
  Q_PROPERTY(DynamicObject* player MEMBER player)
public:
  explicit LevelTask(QObject *parent = nullptr);

  void load(const QString& levelName);

  const QString& getName() const { return name; }
  TileMap* getTileMap() const { return tilemap; }
  LevelGrid* getGrid() const { return grid; }

  Q_INVOKABLE void moveTo(int x, int y);
  Q_INVOKABLE bool moveCharacterTo(DynamicObject*, int x, int y);
  Q_INVOKABLE DynamicObject* getOccupantAt(int x, int y);
  Q_INVOKABLE void tileClicked(int x, int y);
  Q_INVOKABLE void interactOrderReceived(DynamicObject*, const QString& type);

  void registerDynamicObject(DynamicObject*);
  void unregisterDynamicObject(DynamicObject*);

  bool triggerCharacterMoveTo(DynamicObject* character, int x, int y);
  void forceCharacterPosition(DynamicObject* chracter, int x, int y);
  bool openInteractionMenu(DynamicObject*);
  void startPendingInteraction();

signals:
  void pausedChanged();
  void tilemapReady();
  void displayConsoleMessage(const QString&);
  void exitZoneEntered(TileZone*);
  void interactionRequired(DynamicObject* interactionTarget, QStringList interactionList);
  void startDialog(CharacterDialog* dialogController);

private slots:
  void onPauseChanged();
  void onClockTick();
  void onTaskTick();

  void onObjectMovementFinished(Sprite*);
  void onZoneEntered(DynamicObject*, TileZone*);
  void onZoneExited(DynamicObject*, TileZone*);

private:
  DynamicObject* player;
  QList<DynamicObject*> objects;

  QTimer     taskTick, clockTick;
  QString    name;
  TileMap*   tilemap = nullptr;
  LevelGrid* grid = nullptr;
  bool       paused = true;

  QPair<DynamicObject*, QString> pendingInteraction;
};

#endif // LEVELTASK_H

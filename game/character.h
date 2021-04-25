#ifndef  CHARACTER_H
# define CHARACTER_H

# include "globals.h"
# include "cmap/statmodel.h"
# include "diplomacy.hpp"
# include "characters/field_of_view.hpp"
# include "characters/buffs.h"

class ActionQueue;

class Character : public CharacterBuffs
{
  Q_OBJECT
  typedef CharacterBuffs ParentType;

  Q_PROPERTY(ActionQueue* actionQueue MEMBER actionQueue)
  Q_PROPERTY(int actionPoints MEMBER actionPoints NOTIFY actionPointsChanged)
public:
  explicit Character(QObject *parent = nullptr);

  void update(qint64) override;
  void load(const QJsonObject&) override;
  void save(QJsonObject&) const override;

  ActionQueue* getActionQueue() const { return actionQueue; }
  QString      getDialogName();
  unsigned int getXpValue() const;
  Q_INVOKABLE bool isMoving() const { return Sprite::isMoving() || currentPath.size() > 0; }
  bool         isSpriteMoving() const { return Sprite::isMoving(); }
  int          getZIndex() const override { return isAlive() ? 2 : 1; }
  int          getCoverValue() const override { return 10; }
  Q_INVOKABLE void moveAway(Character*);

  int getInteractionDistance() const override;
  bool isBlockingPath() const override { return isAlive(); }

  Q_INVOKABLE QJSValue getActions();
  Q_INVOKABLE bool isAlive() const { return getStatistics()->getHitPoints() > 0; }
  Q_INVOKABLE bool isUncounscious() const { return uncounscious; }

  Q_INVOKABLE void  takeDamage(int damage, Character* dealer);
  Q_INVOKABLE void  attackedBy(Character* dealer);
  Q_INVOKABLE bool  useActionPoints(int amount = 1, const QString& motive = "");
  inline int        getActionPoints() const { return actionPoints; }
  void              resetActionPoints();
  Q_INVOKABLE void  fall(int distance, const QString& direction);

signals:
  void actionPointsChanged();
  void requireJoinCombat();
  void died();
  void characterKill(Character* victim, Character* killer);

private slots:
  void onActionQueueCompleted();
  void afterDeathAnimation();

private:
  QVector<QPoint> getAvailableSurroundingCases() const;
  virtual QString getScriptPath() const override { return SCRIPTS_PATH + "pnjs"; }

  ActionQueue* actionQueue;
  bool sneakEnabled = false;
  bool uncounscious = false;
  int actionPoints = 0;
  QJSValue jsActionQueue;
  QVector<Buff*> buffs;
};

#endif // CHARACTER_H

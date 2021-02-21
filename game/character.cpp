#include "character.h"
#include "game.h"
#include "leveltask.h"
#include "characters/actionqueue.h"
#include <cmath>

Character::Character(QObject *parent) : StorageObject(parent)
{
  fieldOfView = new FieldOfView(*this);
  actionQueue = new ActionQueue(this);
  inventory->setUser(this);
  connect(inventory, &Inventory::unequippedItem, this, &Character::initializeEmptySlot);
  connect(actionQueue, &ActionQueue::queueCompleted, this, &Character::onActionQueueCompleted);
}

void Character::update(qint64 delta)
{
  DynamicObject::update(delta);
  fieldOfView->update(delta);
}

void Character::onActionQueueCompleted()
{
  if (script)
    script->call("onActionQueueCompleted");
}

void Character::takeDamage(int damage, Character* dealer)
{
  auto hp = getStatistics()->getHitPoints() - damage;

  getStatistics()->setHitPoints(hp);
  qDebug() << "took damage" << hp;
  if (hp <= 0)
  {
      qDebug() << "character dying";
    setAnimation("death");
    emit died();
  }
  else if (dealer != nullptr && !isAlly(dealer) && !isEnemy(dealer))
  {
    setAsEnemy(dealer);
    emit requireJoinCombat();
  }
}

QPoint Character::getInteractionPosition() const
{
  if (isAlive())
  {
    auto* player = Game::get()->getPlayer();
    auto* level  = Game::get()->getLevel();
    auto* grid   = level->getGrid();
    auto  center = getPosition();

    for (int x = -1 ; x <= 1 ; ++x)
    {
      if (x < 0) continue ;
      for (int y = -1 ; y <= 1 ; ++y)
      {
        QPoint position(center);
        DynamicObject* occupant;

        if (y < 0) continue ;
        position.rx() += x;
        position.ry() += y;
        occupant = grid->getOccupant(position.x(), position.y());
        if (!grid->isOccupied(position.x(), position.y()) || occupant == player)
          return position;
      }
    }
    qDebug() << "Character::getInteractionPosition: No path to reach character" << getObjectName();
  }
  return getPosition();
}

QString Character::getDialogName()
{
  return script ? script->property("dialog").toString() : "";
}

bool Character::isAlly(const Character* other) const
{
  return faction && faction->flag == other->getFactionFlag();
}

bool Character::isEnemy(const Character* other) const
{
  if (faction)
  {
    auto otherFlag = other->getFactionFlag();

    return otherFlag == 0 ? other->isEnemy(this) : (faction->enemyMask & otherFlag) != 0;
  }
  return (enemyFlag & other->getFactionFlag()) != 0;
}

bool Character::hasLineOfSight(const Character *) const
{
  return true;
}

void Character::setAsEnemy(Character* other)
{
  auto* diplomacy = Game::get()->getDiplomacy();
  auto* faction   = diplomacy->getFaction(other->getFactionFlag());

  if (other->getFactionName() != "") {
    if (faction)
      diplomacy->setAsEnemy(true, getFactionFlag(), other->getFactionFlag());
    else if (!faction)
      enemyFlag += other->getFactionFlag();
  }
  else if (faction)
    other->setAsEnemy(this);
  else
    qDebug() << "Character::setAsEnemy: both characters lack a faction:" << getObjectName() << "and" << other->getObjectName();
}

float Character::getDistance(const DynamicObject* target) const
{
  auto self  = getPosition();
  auto other = target->getPosition();
  auto a = self.x() - other.x();
  auto b = self.y() - other.y();

  return std::sqrt(static_cast<float>(a * a + b * b));
}

void Character::initializeFaction()
{
  auto* characterSheet = getStatistics();
  auto* diplomacy = Game::get()->getDiplomacy();

  if (characterSheet && diplomacy)
    faction = diplomacy->getFaction(characterSheet->getFaction());
}

bool Character::useActionPoints(int amount, const QString& actionType)
{
  auto* level = Game::get()->getLevel();

  if (level && level->isInCombat(this))
  {
    if (amount <= actionPoints)
    {
      actionPoints -= amount;
      emit actionPointsChanged();
      return true;
    }
    return false;
  }
  return true;
}

void Character::resetActionPoints()
{
  actionPoints = getStatistics()->get_actionPoints();
  emit actionPointsChanged();
}

void Character::updateInventorySlots()
{
  QMap<QString, QString> slotTypes({{"armor", "armor"},{"saddle","saddle"},{"use-1","any"},{"use-2","any"}});

  inventory->setSlots(slotTypes);
}

void Character::initializeEmptySlots()
{
  initializeEmptySlot("use-1");
  initializeEmptySlot("use-2");
}

void Character::initializeEmptySlot(const QString& slotName)
{
  if (inventory->getEquippedItem(slotName) == nullptr)
  {
    InventoryItem* item = new InventoryItem(this);

    item->setObjectName(getDefaultItemForSlot(slotName));
    item->setVirtual(true);
    inventory->equipItem(item, slotName);
  }
}

QString Character::getDefaultItemForSlot(const QString& name)
{
  if (script && script->hasMethod("getDefaultItem"))
    return script->call("getDefaultItem", QJSValueList() << name).toString();
  return "melee";
}

void Character::setScript(const QString& name)
{
  DynamicObject::setScript(name);
  updateInventorySlots();
  initializeEmptySlots();
}

void Character::load(const QJsonObject& data)
{
  QString objectName = data["objectName"].toString();

  isUnique = data["uniq"].toBool();
  enemyFlag = static_cast<unsigned int>(data["enemyFlag"].toInt(0));
  actionPoints = data["ap"].toInt();
  if (isUnique)
    statistics = Game::get()->getDataEngine()->makeStatModel(objectName);
  else
  {
    statistics = new StatModel(this);
    statistics->fromJson(data["stats"].toObject());
  }
  connect(statistics, &StatModel::factionChanged, this, &Character::initializeFaction);
  initializeFaction();
  StorageObject::load(data);
}

void Character::save(QJsonObject& data) const
{
  data["uniq"] = isUnique;
  data["enemyFlag"] = static_cast<int>(enemyFlag);
  data["ap"] = actionPoints;
  if (isUnique)
    Game::get()->getDataEngine()->saveStatModel(getObjectName(), statistics);
  else
  {
    QJsonObject statData;

    statistics->toJson(statData);
    data.insert("stats", statData);
  }
  StorageObject::save(data);
}


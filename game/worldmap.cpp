#include "worldmap.h"
#include "worldmapcity.h"
#include "game.h"
#include <QDebug>
#include <QSize>
#include <QJsonArray>

WorldMap::WorldMap(QObject* parent) : QObject(parent)
{
  updateTimer.setInterval(50);
  updateTimer.setSingleShot(false);
  connect(&updateTimer, &QTimer::timeout, this, &WorldMap::update);
  connect(this, &WorldMap::mapSizeChanged, this, &WorldMap::onMapSizeChanged);
  connect(this, &WorldMap::targetPositionChanged, this, &WorldMap::onTargetPositionChanged);
  connect(this, &WorldMap::currentPositionChanged, this, &WorldMap::onCurrentPositionChanged);
  timeManager = Game::get()->getTimeManager();
}

QJsonObject WorldMap::save() const
{
  QJsonObject data;
  QJsonArray discoveredJson;
  QJsonArray citiesJson;

  for (auto it = discovered.begin() ; it != discovered.end() ; ++it)
    discoveredJson << *it;
  for (auto it = cities.begin() ; it != cities.end() ; ++it)
  {
    QJsonObject cityJson;

    cityJson.insert("name",  (*it)->getName());
    cityJson.insert("level", (*it)->getLevel());
    cityJson.insert("x",     (*it)->getPosition().x());
    cityJson.insert("y",     (*it)->getPosition().y());
    cityJson.insert("size",  (*it)->getSize());
    citiesJson << cityJson;
  }
  if (!Game::get()->property("isGameEditor").toBool())
    data.insert("discovered", discoveredJson);
  data.insert("cities",  citiesJson);
  data.insert("playerX", currentPosition.x());
  data.insert("playerY", currentPosition.y());
  data.insert("width", mapSize.width());
  data.insert("height", mapSize.height());
  data.insert("caseWidth", caseSize.width());
  data.insert("caseHeight", caseSize.height());
  qDebug() << "Saving worldmap" << QJsonDocument(data).toJson();
  return data;
}

void WorldMap::onMapSizeChanged()
{
  caseCount = QSize(
    mapSize.width()  / std::max(caseSize.width(), 1),
    mapSize.height() / std::max(caseSize.height(), 1)
  );
  emit caseCountChanged();
}

void WorldMap::load(const QJsonObject& data)
{
  QJsonArray citiesJson = data["cities"].toArray();

  mapSize  = QSize(data["width"].toInt(),     data["height"].toInt());
  caseSize = QSize(data["caseWidth"].toInt(), data["caseHeight"].toInt());
  caseCount = QSize(
    mapSize.width()  / std::max(caseSize.width(), 1),
    mapSize.height() / std::max(caseSize.height(), 1)
  );
  if (!Game::get()->property("isGameEditor").toBool())
  {
    if (data["discovered"].isUndefined())
    {
      for (int i = caseCount.width() * caseCount.height() ; i > 0 ; --i)
        discovered << false;
    }
    else
    {
      QJsonArray discoveredJson = data["discovered"].toArray();

      discovered.reserve(caseCount.width() * caseCount.height());
      for (auto it = discoveredJson.begin() ; it != discoveredJson.end() ; ++it)
        discovered << it->toBool();
    }
  }

  for (auto it = citiesJson.begin() ; it != citiesJson.end() ; ++it)
  {
    QJsonObject cityJson = it->toObject();
    auto* city = new WorldMapCity(this);

    city->setName(cityJson["name"].toString());
    city->setLevel(cityJson["level"].toString());
    city->setPosition(QPoint(cityJson["x"].toInt(), cityJson["y"].toInt()));
    city->setSize(cityJson["size"].toInt());
    cities << city;
  }

  currentPosition.setX(data["playerX"].toInt(300));
  currentPosition.setY(data["playerY"].toInt(184));
  revealCaseAt(currentPosition);

  // Draftin citiez
  /*{
    auto* a = new WorldMapCity(this);
    a->setName("eltest5");
    a->setPosition(QPoint(300, 184));
    a->setSize(80);
    cities << a;
  }

  {
    auto* a = new WorldMapCity(this);
    a->setName("eltest");
    a->setPosition(QPoint(628, 794));
    a->setSize(100);
    cities << a;
  }*/

  emit mapSizeChanged();
  emit citiesChanged();
  emit zonesChanged();

}

static int axisMovement(int current, int final, int speed)
{
  int result = final;

  if      (current < final && final - current > speed) { result = current + speed; }
  else if (current > final && current - final > speed) { result = current - speed; }
  return std::abs(result - final) < speed ? final : result;
}

bool WorldMap::isVisible(int x, int y) const
{
  int offset = y * caseCount.width() + x;

  if (offset < discovered.size())
    return discovered[offset];
  return true;
}

float WorldMap::getCurrentMovementSpeed() const
{
  const WorldMapZone* zone = getCurrentZone();

  if (zone)
    return static_cast<float>(zone->getMovementSpeed());
  return 5.f;
}

void WorldMap::update()
{
  qDebug() << "WorldMap::update";
  float movementSpeed = getCurrentMovementSpeed();
  float movementSpeedX = movementSpeed;
  float movementSpeedY = movementSpeed;
  int   distX = std::max(currentPosition.x(), targetPosition.x()) - std::min(currentPosition.x(), targetPosition.x());
  int   distY = std::max(currentPosition.y(),  targetPosition.y()) - std::min(currentPosition.y(), targetPosition.y());

  if      (distX > distY) { movementSpeedY = movementSpeed * (static_cast<float>(distY) / static_cast<float>(distX)); }
  else if (distY > distX) { movementSpeedX = movementSpeed * (static_cast<float>(distX) / static_cast<float>(distY)); }
  currentPosition.setX(axisMovement(currentPosition.x(), targetPosition.x(), static_cast<int>(movementSpeedX)));
  currentPosition.setY(axisMovement(currentPosition.y(), targetPosition.y(), static_cast<int>(movementSpeedY)));
  emit currentPositionChanged();
  if (currentPosition == targetPosition)
    emit onTargetPositionReached();
  Game::get()->advanceTime(14);
}

void WorldMap::onCurrentPositionChanged()
{
  revealCaseAt(currentPosition);
}

void WorldMap::onTargetPositionChanged()
{
  qDebug() << "Starting update timer";
  updateTimer.start();
}

void WorldMap::onTargetPositionReached()
{
  qDebug() << "Stopping update timer";
  updateTimer.stop();
}

void WorldMap::getIntoCity(WorldMapCity* city)
{
  QString levelSource = city->getLevel();

  levelSource.resize(levelSource.length() - 5);
  qDebug() << "Rekuezted get into city" << city->getLevel() << "->" << levelSource;
  emit cityEntered(levelSource);
}

void WorldMap::getIntoWasteland(QPoint poz)
{
  qDebug() << "Get into wazteland" << poz;
}

void WorldMap::revealCaseAt(const QPoint position)
{
  int caseX = position.x() / std::max(caseSize.width(), 1);
  int caseY = position.y() / std::max(caseSize.height(), 1);
  unsigned int offset = static_cast<unsigned int>(caseY * caseCount.width() + caseX);

  if (offset < discovered.size() && !discovered[offset])
  {
    discovered[offset] = true;
    emit caseRevealed(caseX, caseY);
  }
}

WorldMapCity* WorldMap::createCity(const QString &name)
{
  WorldMapCity* city = new WorldMapCity(this);

  city->setName(name);
  cities << city;
  emit citiesChanged();
  return city;
}

WorldMapZone* WorldMap::createZone(const QString &name)
{
  WorldMapZone* zone = new WorldMapZone(this);

  zone->setProperty("name", name);
  zones << zone;
  emit zonesChanged();
  return zone;
}

void WorldMap::removeCity(WorldMapCity* city)
{
  auto index = cities.indexOf(city);

  if (index >= 0)
  {
    cities.removeAt(index);
    emit citiesChanged();
  }
  city->deleteLater();
}

void WorldMap::removeZone(WorldMapZone* zone)
{
  auto index = zones.indexOf(zone);

  if (index >= 0)
  {
    zones.removeAt(index);
    emit zonesChanged();
  }
  zone->deleteLater();
}

WorldMapZone* WorldMap::getCurrentZone() const
{
  int caseX = currentPosition.x() / caseSize.width();
  int caseY = currentPosition.y() / caseSize.height();

  for (WorldMapZone* zone : qAsConst(zones))
  {
    if (zone->containsCase(caseX, caseY))
      return zone;
  }
  return nullptr;
}

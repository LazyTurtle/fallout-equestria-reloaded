#include "dynamicobject.h"
#include "character.h"
#include "game.h"
#include "tilemap/tilezone.h"
#include <QJsonArray>

DynamicObject::DynamicObject(QObject *parent) : Sprite(parent)
{
  position = QPoint(-1,-1);
  taskManager = new TaskRunner(this);
  connect(this, &DynamicObject::controlZoneAdded,   this, &DynamicObject::controlZoneChanged);
  connect(this, &DynamicObject::controlZoneRemoved, this, &DynamicObject::controlZoneChanged);
  connect(this, &DynamicObject::blocksPathChanged, this, &DynamicObject::onBlocksPathChanged);
}

DynamicObject::~DynamicObject()
{
  if (script)
    delete script;
}

void DynamicObject::updateTasks(qint64 v)
{
  taskManager->update(v);
}

void DynamicObject::setScript(const QString& name)
{
  if (script)
    delete script;
  scriptName = name;
  script     = new ScriptController(getScriptPath() + '/' + name);
  taskManager->setScriptController(script);
  script->initialize(this);
  emit scriptNameChanged();
}

QStringList DynamicObject::getAvailableInteractions()
{
  if (script)
  {
    qDebug() << "Calling get available interactions on " << getObjectName();
    return script->call("getAvailableInteractions").toVariant().toStringList();
  }
  else
    qDebug() << "/!\\ Missing script on object " << getObjectName();
  return QStringList();
}

bool DynamicObject::triggerInteraction(Character* character, const QString &interactionType)
{
  static const QMap<QString, QString> callbackMap = {
    {"talk-to",   "onTalkTo"},
    {"use",       "onUse"},
    {"use-skill", "onUseSkill"},
    {"use-magic", "onUseMagic"},
    {"look",      "onLook"}
  };

  qDebug() << "DynamicObject::triggerInteraction";
  if (character && script)
  {
    const QString callback = callbackMap[interactionType];

    return script->call(callback, QJSValueList() << character->asJSValue()).toBool();
  }
  return false;
}

bool DynamicObject::triggerSkillUse(Character *user, const QString &skillName)
{
  QString methodName = skillName;

  methodName[0] = methodName[0].toUpper();
  methodName = "onUse" + methodName;
  qDebug() << "Trying to call" << methodName;
  if (user && script && script->hasMethod(methodName))
    return script->call(methodName, QJSValueList() << user->asJSValue()).toBool();
  else if (user == Game::get()->getPlayer())
    Game::get()->appendToConsole("You use " + skillName + " on " + getObjectName() + ". It does nothing.");
  return false;
}

QJSValue DynamicObject::scriptCall(const QString& method, const QString& message)
{
  if (script)
    return script->call(method, QJSValueList() << message);
  return QJSValue();
}

QJSValue DynamicObject::getScriptObject() const
{
  if (script)
    return script->getObject();
  return QJSValue();
}

QJSValue DynamicObject::asJSValue()
{
  if (script)
    return script->getModel();
  return Game::get()->getScriptEngine().newQObject(this);
}

TileZone* DynamicObject::addControlZone()
{
  if (controlZone == nullptr)
  {
    controlZone = new TileZone(this);
    emit controlZoneAdded(controlZone);
  }
  return controlZone;
}

void DynamicObject::removeControlZone()
{
  if (controlZone != nullptr)
  {
    delete controlZone;
    emit controlZoneRemoved(controlZone);
  }
  controlZone = nullptr;
}

void DynamicObject::setVisible(bool value)
{
  if (visible != value)
  {
    visible = value;
    emit visibilityChanged();
  }
}

void DynamicObject::onBlocksPathChanged()
{
  auto* level = Game::get()->getLevel();
  auto* grid  = level ? level->getGrid() : nullptr;

  if (grid)
  {
    if (blocksPath)
      grid->moveObject(this, position.x(), position.y());
    else
      grid->removeObject(this);
  }
}

void DynamicObject::load(const QJsonObject& data)
{
  objectName = data["objectName"].toString();
  position.setX(data["x"].toInt()); position.setY(data["y"].toInt());
  nextPosition.setX(data["nextX"].toInt()); nextPosition.setY(data["nextY"].toInt());
  interactionPosition.setX(data["intX"].toInt()); interactionPosition.setY(data["intY"].toInt());
  blocksPath = data["blocksPath"].toBool(true);
  emit blocksPathChanged();
  if (data["zone"].isArray())
  {
    controlZone = controlZone ? controlZone : new TileZone(this);
    for (QJsonValue posValue : data["zone"].toArray())
    {
      QJsonArray posArray(posValue.toArray());

      controlZone->addPosition(QPoint(posArray[0].toInt(), posArray[1].toInt()));
    }
    emit controlZoneChanged();
  }
  currentZone = data["currentZone"].toString();
  scriptName  = data["script"].toString();
  dataStore   = data["dataStore"].toObject();
  Sprite::load(data);
  setScript(scriptName);
  taskManager->load(data);
  emit positionChanged();
}

void DynamicObject::save(QJsonObject& data) const
{
  data["objectName"] = objectName;
  data["x"] = position.x(); data["y"] = position.y();
  data["nextX"] = nextPosition.x(); data["nextY"] = nextPosition.y();
  data["intX"] = interactionPosition.x(); data["intY"] = interactionPosition.y();
  data["blocksPath"] = blocksPath;
  if (controlZone)
  {
    QJsonArray zoneArray;

    for (QPoint position : controlZone->getPositions())
    {
      QJsonArray posArray;

      posArray << position.x() << position.y();
      zoneArray << posArray;
    }
    data["zone"] = zoneArray;
  }
  data["currentZone"] = currentZone;
  data["script"]      = scriptName;
  data["dataStore"]   = dataStore;
  Sprite::save(data);
  taskManager->save(data);
}

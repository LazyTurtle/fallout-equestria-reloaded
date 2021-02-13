#ifndef INVENTORYITEM_H
#define INVENTORYITEM_H

#include "dynamicobject.h"
#include <QJSValue>
#include <QObject>

class InventoryItem : public DynamicObject
{
  Q_OBJECT

  Q_PROPERTY(QString itemType READ getItemType NOTIFY objectNameChanged)
  Q_PROPERTY(int     weight   READ getWeight  NOTIFY weightChanged)
  Q_PROPERTY(int     quantity MEMBER quantity NOTIFY quantityChanged)
  Q_PROPERTY(int     value    READ getValue   NOTIFY valueChanged)
public:
  explicit InventoryItem(QObject* parent = nullptr);

  void load(const QJsonObject&);
  void save(QJsonObject&) const;

  Q_INVOKABLE void add(int quantity = 1);
  Q_INVOKABLE bool remove(int quantity = 1);
  Q_INVOKABLE QPoint getInteractionPosition() const { return getPosition(); }
  QStringList getAvailableInteractions();
  int         getWeight() const;
  int         getValue() const;
  int         getQuantity() const { return quantity; }
  QString     getItemType() const;
  bool        isGroupable(InventoryItem* = nullptr);

signals:
  void itemTypeChanged();
  void nameChanged();
  void weightChanged();
  void quantityChanged();
  void valueChanged();

private slots:
  void updateScript();
  void updateSprite();

private:
  int quantity;
};

#endif // INVENTORYITEM_H

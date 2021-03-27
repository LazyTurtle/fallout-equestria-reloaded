#ifndef  PRERENDERCOMPONENT_H
# define PRERENDERCOMPONENT_H

# include "zone.h"

class PreRenderComponent : public ZoneComponent
{
  Q_OBJECT
  typedef ZoneComponent ParentType;

  Q_PROPERTY(QString preRenderPath READ getPreRenderPath)
public:
  PreRenderComponent(QObject* parent = nullptr);

  void load();

protected:
  void    preRenderTilemap();
  QString getPreRenderPath() const;
};

#endif // PRERENDERCOMPONENT_H

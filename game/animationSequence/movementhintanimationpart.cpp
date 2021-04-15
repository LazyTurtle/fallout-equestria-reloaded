#include "movementhintanimationpart.h"
#include "game.h"

MovementHintAnimationPart::MovementHintAnimationPart(QPoint position)
{
  TileLayer* layer;
  Tile*      tile;

  sprite = new Sprite(this);
  level  = Game::get()->getLevel();
  layer = level->getTileMap()->getLayer("ground");
  tile  = layer->getTile(position.x(), position.y());
  sprite->setProperty("floating", false);
  sprite->setSpriteName("misc");
  sprite->setAnimation("movement-hint");
  qDebug() << "Sprite render pozition will be" << tile->getRenderPosition() << "correzponding to" << position;
  from = to = tile->getRenderPosition();
  connect(sprite, &Sprite::animationFinished, this, &SpriteAnimationPart::onAnimationFinished);
}

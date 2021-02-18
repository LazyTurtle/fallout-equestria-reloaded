import QtQuick 2.15
import "LevelRender.mjs" as LevelRender

Canvas {
  id: canvas
  anchors.fill: parent
  renderStrategy: Canvas.Cooperative
  renderTarget:   Canvas.Image
  onPaint: { if (controller) { controller.render(); } }

  property QtObject levelController;
  property point origin;
  property var controller;
  property var hoverTile: [];

  Component.onCompleted: {
    preloadImages();
  }

  onLevelControllerChanged: {
    preloadImages();
    initializeRenderer();
  }

  Timer {
    id: afterLoadCameraCenter; interval: 100; onTriggered: moveToObject(levelController.player);
  }

  Connections {
    target: levelController
    function onUpdated() { canvas.requestPaint(); }
    function onCameraFocusRequired(object) { moveToObject(object); }
  }

  onAvailableChanged: {
    initializeRenderer();
    afterLoadCameraCenter.running = true;
  }

  MouseArea {
    id: mouseArea
    anchors.fill: parent
    enabled: !levelController.paused
    acceptedButtons: Qt.LeftButton | Qt.RightButton
    hoverEnabled: true
    onClicked: {
      if (mouse.button === Qt.RightButton)
        levelController.swapMouseMode();
      else
        controller.onMouseClick(mouse, mouseX, mouseY);
    }
    onMouseXChanged: hoverTile = controller.getHoveredCase(mouseX, mouseY);
    onMouseYChanged: hoverTile = controller.getHoveredCase(mouseX, mouseY);
    cursorShape: Qt.BlankCursor
  }

  Image {
    property string cursorType: { ["move", "interaction", "target"][levelController.mouseMode] }
    property size  size: { [Qt.size(40, 15), Qt.size(50, 35), Qt.size(40, 40)][levelController.mouseMode] }
    property point offset: { levelController.mouseMode !== 1 ? Qt.point(size.width / 2, size.height / 2) : Qt.point(0, 0) }
    x: mouseArea.mouseX - offset.x
    y: mouseArea.mouseY - offset.y
    height: size.height
    width: size.width
    source: `qrc:/assets/ui/cursors/${cursorType}.png`
    visible: mouseArea.containsMouse
  }

  function initializeRenderer() {
    controller = new LevelRender.Controller(canvas, {
      level: levelController,
      tilemap: levelController.tilemap,
      pathPrefix: fileProtocol
    });
  }

  function preloadImages() {
    const images = [];

    for (var i = 0 ; i < levelController.tilemap.textureList.length ; ++i) {
      const texturePath = levelController.tilemap.textureList[i];
      if (images.indexOf(texturePath) === -1) {
        console.log("LevelCanvas: Preloading image", texturePath, "from Tiled resources");
        loadImage("../" + texturePath);
        images.push(texturePath);
      }
    }

    animationLibrary.getSources().forEach(function(texture) {
      console.log("LevelCanvas: Preloading texture", texture, "from animation library");
      loadImage("../assets/sprites/" + texture);
    });
  }

  function translate(x, y) {
    const context = getContext("2d");
    const limits = controller.getLimits();
    var   newOriginX = origin.x + x;
    var   newOriginY = origin.y + y;

    newOriginX = Math.min(newOriginX, limits.maxX);
    newOriginX = Math.max(newOriginX, limits.minX);
    newOriginY = Math.min(newOriginY, limits.maxY);
    newOriginY = Math.max(newOriginY, limits.minY);
    x = newOriginX - origin.x;
    y = newOriginY - origin.y;
    context.translate(x, y);
    origin.x += x;
    origin.y += y;
    requestPaint();
  }

  function moveToOffset(x, y) {
    translate(origin.x - x, origin.y - y);
  }

  function moveToCoordinates(x, y) {
    const position = controller.getPointFor(x, y);

    position.x -= canvas.width / 2
    position.y -= canvas.height / 2
    translate(-origin.x - position.x, -origin.y - position.y);

  }

  function moveToObject(object) {
    const position = object.getSpritePosition();

    position.x -= canvas.width / 2
    position.y -= canvas.height / 2
    translate(-origin.x - position.x, -origin.y - position.y);
  }

  Shortcut {
    sequence: "c"
    onActivated: moveToObject(levelController.player)
  }
}

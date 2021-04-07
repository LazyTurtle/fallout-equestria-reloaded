import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "qrc:/assets/ui" as UiStyle
import "../ui"
import "../game" as GameComponents
import "./level" as LevelEditorUi
import Game 1.0

Item {
  id: root
  property var gameController
  property alias currentLevelName: levelSelect.currentName
  property var objectList: []
  property QtObject selectedObject

  signal pickedTile(int tileX, int tileY)

  onCurrentLevelNameChanged: {
    selectedObject = null;
    levelEditorUiLoader.sourceComponent = null;
    gameController.goToLevel(currentLevelName.replace(".json", ""));
    if (currentLevelName.length > 0)
      levelEditorUiLoader.sourceComponent = levelEditorUi;
  }

  onSelectedObjectChanged: {
    canvas.editorObject = selectedObject;
    objectListComponent.currentName = selectedObject.objectName;
  }

  function updateObjectList() {
    var array = [];
    for (var i = 0 ; i < gameController.level.dynamicObjects.length ; ++i)
      array.push(gameController.level.dynamicObjects[i].objectName);
    objectList = array;
  }

  Connections {
    target: gameController.level
    function onDynamicObjectsChanged() {
      updateObjectList();
    }
    function onClickedOnObject(value) {
      objectSelectBox.currentIndex = objectList.indexOf(value.objectName);
    }
  }

  RowLayout {
    anchors.fill: parent

    EditorSelectPanel {
      id: levelSelect
      model: scriptController.getLevels()
      readOnly: true
    }

    Loader {
      id: levelEditorUiLoader
      Layout.fillHeight: true
      Layout.fillWidth: true
    }
  }

  Component {
    id: levelEditorUi
    RowLayout {
      Component.onCompleted: {
        updateObjectList();
        canvas.translate(-canvas.origin.x, -canvas.origin.y);
      }

      Pane {
        background: UiStyle.TerminalPane {}
        Layout.fillHeight: true
        Layout.fillWidth: true

        LevelEditorUi.EditorCanvas {
          id: canvas
          levelController: gameController.level
          renderRoofs: displayRoofCheckbox.checked
          renderWalls: displayWallsCheckbox.checked
          showHoverCoordinates: true
          editorObject: selectedObject

          editingZone: controlZoneEditor.editingZone
          onToggleZoneTile: controlZoneEditor.toggleTile(tileX, tileY)
          onPickedTile: root.pickedTile(tileX, tileY)
          onPickedObject: objectListComponent.currentName = dynamicObject.objectName;
        }

        GameComponents.ScreenEdges {
          enabled: characterInventory.visible == false
          onMoveTop:    { canvas.translate(0, scrollSpeed); }
          onMoveLeft:   { canvas.translate(scrollSpeed, 0); }
          onMoveRight:  { canvas.translate(-scrollSpeed, 0); }
          onMoveBottom: { canvas.translate(0, -scrollSpeed); }
        }
      }

      Pane {
        background: UiStyle.Pane {}
        Layout.preferredWidth: 400
        Layout.fillHeight: true
        Layout.bottomMargin: saveButton.height

        ColumnLayout {
          width: parent.width
          height: parent.height

          Row {
            CheckBox {
              id: displayRoofCheckbox
              text: "Display roofs"
              contentItem: Text {
                leftPadding: 45
                text: parent.text
                color: "white"
              }
            }

            CheckBox {
              id: displayWallsCheckbox
              checked: true
              text: "Display walls"
              contentItem: Text {
                leftPadding: 45
                text: parent.text
                color: "white"
              }
            }
          }

          EditorSelectPanel {
            id: objectListComponent
            model: objectList
            onNewClicked: dialogAddObject.open()
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: selectedObject == null
            onCurrentNameChanged: selectedObject = gameController.level.getObjectByName(currentName);
          }

          LevelEditorUi.ObjectEditorLoader {
            id: objectEditorComponent
            Layout.fillWidth: true
            Layout.fillHeight: true
            levelEditor: root
            visible: selectedObject != null
            onOpenInventory: {
              if (model.getObjectType() === "Character") {
                characterInventory.character = model;
                characterInventory.open();
              }
              else {
                lootEditor.inventory = model.inventory;
                lootEditor.open();
              }
            }
            onSaveTemplateClicked: saveTemplateDialog.open()
            onPreviousClicked: selectedObject = null;
          }

          ControlZoneEditor {
            id: controlZoneEditor
            selectedObject: root.selectedObject
            displayRoofs: displayRoofCheckbox
            displayWalls: displayWallsCheckbox
            Layout.fillWidth: true
          }
        }
      }
    }
  }

  UiStyle.CustomDialog {
    id: saveTemplateDialog
    title: "new template"
    modal: true
    anchors.centerIn: parent
    standardButtons: Dialog.Ok | Dialog.Cancel
    background: UiStyle.Pane {}
    GridLayout {
      CustomLabel { text: "Name" }
      CustomTextField {
        id: templateNameInput
        Layout.fillWidth: true
        Layout.preferredHeight: 40
      }
    }
    onAccepted: {
      gameObjectTemplates.save(templateNameInput.text, selectedObject);
    }
  }

  LevelEditorUi.CharacterInventoryEditor {
    id: characterInventory
    anchors.fill: parent
    anchors.margins: 50
    visible: false
  }

  LevelEditorUi.LootEditor {
    id: lootEditor
    anchors.fill: parent
    anchors.margins: 50
    visible: false
  }

  LevelEditorUi.AddObjectDialog {
    id: dialogAddObject
    gameController: root.gameController
    onObjectAdded: {
      console.log("Added object", newObject);
      objectSelectBox.currentIndex = gameController.level.dynamicObjects.indexOf(newObject);
    }
  }

  MenuButton {
    id: saveButton
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    text: "Save"
    onClicked: {
      gameController.save();
      gameController.getDataEngine().saveToFile("./assets/game.json");
    }
  }
}

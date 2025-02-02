import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../ui"
import "../assets/ui" as UiStyle

Pane {
  id: root
  property QtObject controller
  property QtObject selectedInventory
  property QtObject selectedObject

  background: UiStyle.Pane {}

  Connections {
    target: controller
    function onFinished() {
      if (root.StackView.status === StackView.Active)
        application.popView();
      if (gameManager.currentGame.level)
        gameManager.currentGame.level.paused = false;
    }
  }

  Action {
    id: takeAllAction
    onTriggered: {
      controller.takeAll();
    }
  }

  Action {
    id: closeAction
    onTriggered: {
      controller.finished();
    }
  }

  Action {
    id: takeAction
    enabled: selectedObject != null && selectedInventory == controller.inventory
    onTriggered: {
      if (selectedObject.quantity > 1)
        quantityInputDialog.open();
      else
      {
        controller.take(selectedObject, 1);
        selectedObject = null;
      }
    }
  }

  Action {
    id: dropAction
    enabled: selectedObject != null && selectedInventory == controller.character.inventory
    onTriggered: {
      if (selectedObject.quantity > 1)
        quantityInputDialog.open();
      else
      {
        controller.drop(selectedObject, 1);
        selectedObject = null;
      }
    }
  }

  ItemTransferDialog {
    id: quantityInputDialog
    anchors.centerIn: parent
    inventoryItem: selectedObject

    onPickedValue: {
      const removed = selectedObject.quantity == amount;

      if (selectedInventory === controller.character.inventory)
        controller.drop(selectedObject, amount);
      else
        controller.take(selectedObject, amount);
      if (removed)
        selectedObject = null;
    }
  }

  RowLayout {
    anchors { top: parent.top; left: parent.left; bottom: controls.top; right: parent.right }
    anchors.margins: 15

    InventoryItemsView {
      id: characterInventoryView
      dragZone: root
      inventory: controller.character.inventory
      selectedObject: root.selectedObject
      onItemSelected: {
        root.selectedInventory = controller.character.inventory;
        root.selectedObject    = selectedItem;
      }
      onItemDropped: takeAction.trigger()
      Layout.preferredWidth: parent.width / 3
      Layout.fillHeight: true
    }

    Pane {
      background: UiStyle.Pane {}
      Layout.preferredWidth: parent.width / 3
      Layout.maximumWidth: 300
      Layout.fillHeight: true

      Column {
        anchors.fill: parent
        anchors.margins: 10

        Pane {
          background: UiStyle.TerminalPane {}
          anchors.left: parent.left
          anchors.right: parent.right

          ColumnLayout {
            anchors.left: parent.left
            anchors.right: parent.right

            RowLayout {
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignCenter

              TerminalButton {
                visible: controller.availableTargets.length > 1
                text: '<'
                onClicked: controller.previousTarget()
              }

              TerminalLabel { text: controller.target.objectName }

              TerminalButton {
                visible: controller.availableTargets.length > 1
                text: '>'
                onClicked: controller.nextTarget()
              }
            }

            ObjectSpritePreview {
              dynamicObject: controller.target
              Layout.alignment: Qt.AlignCenter
            }
          }
        }

        Loader {
          anchors { left: parent.left; right: parent.right }
          sourceComponent: selectedObject ? itemPreviewComponent : null
        }

        MenuButton {
          text: i18n.t("Take")
          action: takeAction
          anchors.horizontalCenter: parent.horizontalCenter
        }
        MenuButton {
          text: i18n.t("Leave")
          action: dropAction
          anchors.horizontalCenter: parent.horizontalCenter
        }
      }
    }

    InventoryItemsView {
      id: targetInventoryView
      dragZone: root
      inventory: controller.inventory
      selectedObject: root.selectedObject
      onItemSelected: {
        root.selectedInventory = controller.inventory
        root.selectedObject    = selectedItem
      }
      onItemDropped: dropAction.trigger()

      Layout.preferredWidth: parent.width / 3
      Layout.fillHeight: true
    }
  }

  Row {
    id: controls
    anchors { bottom: parent.bottom; right: parent.right }

    MenuButton {
      text: i18n.t("Take all")
      action: takeAllAction
    }

    MenuButton {
      text: i18n.t("Close")
      action: closeAction
    }
  }

  Component {
    id: itemPreviewComponent
    InventoryItemPreview {
      model: selectedObject
    }
  }
}

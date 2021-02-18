import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "qrc:/ui"
import "qrc:/assets/ui" as UiStyle

Pane {
  property int itemIconWidth: 72
  property int itemIconHeight: 36
  property QtObject inventory
  property QtObject selectedObject
  id: root
  background: UiStyle.TerminalPane {}

  signal itemSelected(QtObject selectedItem)

  Flickable {
    clip: true
    contentHeight: characterInventoryItemsView.height
    anchors.fill: parent
    anchors.bottomMargin: 15

    GridLayout {
      id: characterInventoryItemsView
      width: parent.width
      columns: parent.width / root.itemIconWidth

      Repeater {
        model: inventory.items.length
        delegate: Rectangle {
          property QtObject inventoryItem: inventory.items[index]
          property int itemCount: inventoryItem.quantity
          property bool selected: selectedObject == inventoryItem

          width: Math.max(root.itemIconWidth, root.itemIconHeight)
          height: width
          border.width: root.selectedObject === inventoryItem ? 1 : 0
          border.color: "green"
          radius: 5
          color: "transparent"
          ItemIcon {
            anchors.centerIn: parent
            model: inventoryItem
            width: root.itemIconWidth
            height: root.itemIconHeight
          }
          Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 5
            text: itemCount > 1 ? `x${itemCount}` : ""
            font.family: application.consoleFontName
            font.pointSize: 8
            style: Text.Raised
            styleColor: "black"
            color: "white"
            width: root.itemIconWidth
            wrapMode: Text.WrapAnywhere
          }
          MouseArea {
            id: itemMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: root.itemSelected(inventoryItem);
          }
        }
      }
    }
  }
}

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "qrc:/assets/ui" as UiStyle
import "../ui"
import "./pipboy" as PipBoyUi

Pane {
  id: root
  property QtObject gameController
  property QtObject levelController
  property var      appNames:      ["Clock", "Quests", "Archives", "Automap"]
  property var      appComponents: [waitApplication, questsApplication, archiveApplication, automapApplication]
  property string   currentApp:    "Quests"

  background: UiStyle.Pane {}

  Component.onCompleted: {
    levelController.paused = true;
    contentApplication.sourceComponent = questsApplication
  }

  onCurrentAppChanged: {
    const index = appNames.indexOf(currentApp);
    contentApplication.sourceComponent = appComponents[index];
  }

  Connections {
    target: gameController.player

    function onDied() {
      application.popView();
    }
  }

  Action {
    id: nextAppAction
    shortcut: Shortcut { sequence: "PgDown"; onActivated: nextAppAction.trigger() }
    onTriggered: {
      const index = appNames.indexOf(currentApp);
      currentApp = appNames[index + 1 >= appNames.length ? 0 : index + 1];
    }
  }

  Action {
    id: previousAppAction
    shortcut: Shortcut { sequence: "PgUp"; onActivated: previousAppAction.trigger() }
    onTriggered: {
      const index = appNames.indexOf(currentApp);
      currentApp = appNames[index - 1 < 0 ? appNames.length - 1 : index - 1];
    }
  }

  Connections {
    target: gamepad

    function onLeftTriggerClicked() { previousAppAction.trigger(); }
    function onRightTriggerClicked() { nextAppAction.trigger(); }
  }

  RowLayout {
    anchors.fill: parent

    // Left pane
    ColumnLayout {
      Layout.preferredWidth: 400
      Layout.fillHeight: true

      PipBoyUi.TimeDisplay {
        timeManager: gameController.timeManager
        Layout.alignment: Qt.AlignTop | Qt.AlignCenter
      }

      Item {
        Layout.fillHeight: true
      }

      Pane {
        background: UiStyle.Pane {}
        Layout.alignment: Qt.AlignBottom | Qt.AlignCenter
        Column {
          Repeater {
            model: appNames
            delegate: MenuButton {
              text: i18n.t(`pipboy.${appNames[index]}`)
              textColor: appNames[index] === currentApp ? "yellow" : "white"
              onClicked: currentApp = appNames[index]
            }
          }

          MenuButton {
            text: i18n.t("Close")
            onClicked: {
              levelController.paused = false;
              application.popView();
            }
          }
        }
      }
    }

    // Content Pane
    Pane {
      background: UiStyle.TerminalPane {}
      Layout.fillWidth: true
      Layout.fillHeight: true

      Loader {
        id: contentApplication
        anchors.fill: parent
        anchors.margins: 10
        clip: true
      }
    }

    // Pages
    Component {
      id: waitApplication
      PipBoyUi.TimeApplication {
        gameController: root.gameController
      }
    }

    Component {
      id: questsApplication
      PipBoyUi.QuestsApplication {
        questManager: root.gameController.getQuestManager()
      }
    }

    Component {
      id: archiveApplication
      Text {
        text: "Nothing here yet !"
      }
    }

    Component {
      id: automapApplication
      PipBoyUi.AutomapApplication {
        levelController: root.levelController
      }
    }
  }
}

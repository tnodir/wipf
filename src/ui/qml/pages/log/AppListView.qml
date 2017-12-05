import QtQuick 2.9
import QtQuick.Controls 2.2
import com.fortfirewall 1.0

ListView {
    id: appListView

    spacing: 10

    property string emptyText

    highlightRangeMode: ListView.ApplyRange
    highlightResizeDuration: 0
    highlightMoveDuration: 200

    highlight: Item {
        Rectangle {
            anchors.fill: parent
            anchors.margins: -7
            radius: 2
            border.width: 3
            border.color: palette.highlight
            color: "transparent"
        }
    }

    delegate: Row {
        id: appItem
        width: appListView.width
        spacing: 6

        readonly property string appPath: display

        // TODO: Use SHGetFileInfo() to get app's display name and icon
        Image {
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 1
            source: "qrc:/images/application.png"
        }
        Label {
            font.pixelSize: 20
            elide: Text.ElideRight
            text: appItem.appPath ? fileUtil.fileName(appItem.appPath)
                                  : emptyText
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            const index = appListView.indexAt(mouse.x, mouse.y);
            if (index >= 0) {
                appListView.currentIndex = index;
            }
        }
    }
}

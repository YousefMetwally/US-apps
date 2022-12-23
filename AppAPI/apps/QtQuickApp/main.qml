// Qt imports
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// Import contents of internal and common qrc files
import "."
import test2d 1.0

/* Context properties set before loading QML:
  - property QtQuickApp application; ///< Top-level class of application
*/

Rectangle {
    id: root

    signal exitClicked()
    signal playStopClicked()
    signal loadClicked()

    width: 1; height: 1 ///< Size will be resized by parent anyway
    color: "limegreen"      ///< Background color

    Image {
        anchors.top: parent.top
        sourceSize.width: parent.width
        sourceSize.height: parent.height - 30
        horizontalAlignment: Image.AlignHCenter
        verticalAlignment: Image.AlignVCenter
        id: imageview
        source: "image://image2dprovider/" + application.image2dprovider.sourceImageFrame
    }

    Row {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        layoutDirection: Qt.RightToLeft
        height: 30
        Button {
            text: qsTr("Exit")
            onClicked: root.exitClicked()
        }
        Button {
            text: qsTr("Stop")
            onClicked: {
                root.playStopClicked()
                if(text == "Play")
                    text = "Stop"
                else
                    text = "Play"
            }
        }
        Button {
            text: qsTr("Load")
            objectName: "loadBtn"
            visible: false
            onClicked: root.loadClicked()
        }
    }
}

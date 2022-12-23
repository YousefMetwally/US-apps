import QtQuick 2.15
import QtQuick.Window 2.15 // needed by host
import QmlTestApp 1.0

Rectangle {
    anchors.fill: parent
    color: edsGray1300

    // To support high DPI displays, a scale factor (sf) is calculated by
    // defining a FontMetric and dividing the height in pixels of this metric
    // by a reference height in pixels. The reference pixel height (55.84375)
    // is here obtained on a MacBook Pro (16-inch, 2019).
    FontMetrics {
        id: scale_font
        font.family: "Arial"
        font.pointSize: 50.0
    }
    property real sf: (scale_font.height/55.84375)

    property real defaultMargin: 10*sf
    property real buttonHorizontalMargin: 25*sf
    property real defaultRadius: 2.5*sf

    property real defaultFontPointSize: 16.0
    property string defaultFontFamily: "GE Inspira Sans"

    FontMetrics {
        id: default_font
        font.family: defaultFontFamily
        font.pointSize: defaultFontPointSize
    }
    property real buttonHeight: default_font.height*2.45

    property color edsGray100: "#fafbfd"
    property color edsGray1300: "#202324"
    property color edsBlue400: "#037aa6"
    property color edsBlue500: "#02607e"

    ImageItem {
        objectName: "imageitemobject"
        anchors.fill: parent
        anchors.margins: defaultMargin/2.0
    }

    Rectangle {
        id: change_display_button
        objectName: "changedisplaymodeobject"
        anchors.left: parent.left
        anchors.leftMargin: defaultMargin
        anchors.top: parent.top
        anchors.topMargin: defaultMargin
        width: button_text.contentWidth + buttonHorizontalMargin*2
        height: buttonHeight
        color: edsBlue400
        radius: defaultRadius

        Text {
            id: button_text
            anchors.centerIn: parent
            font.family: defaultFontFamily
            font.pointSize: defaultFontPointSize
            font.bold: true
            color: edsGray100
            text: qsTr("Resize")
        }

        signal changeDisplayModeSignal

        MouseArea {
            anchors.fill: parent

            onPressed: {
                change_display_button.color = edsBlue500;
            }
            onClicked: {
                change_display_button.changeDisplayModeSignal()
            }
            onReleased: {
                change_display_button.color = edsBlue400;
            }
        }
    }

    Rectangle {
        id: quit_plugin_button
        objectName: "quitpluginbuttonobject"
        anchors.right: parent.right
        anchors.rightMargin: defaultMargin
        anchors.top: parent.top
        anchors.topMargin: defaultMargin
        width: button_text_quit_plugin.contentWidth + buttonHorizontalMargin*2
        height: buttonHeight
        color: edsBlue400
        radius: defaultRadius

        Text {
            id: button_text_quit_plugin
            anchors.centerIn: parent
            font.family: defaultFontFamily
            font.pointSize: defaultFontPointSize
            font.bold: true
            color: edsGray100
            text: qsTr("Quit")
        }

        signal quitPluginSignal

        MouseArea {
            anchors.fill: parent

            onPressed: {
                quit_plugin_button.color = edsBlue500;
            }
            onClicked: {
                quit_plugin_button.quitPluginSignal()
            }
            onReleased: {
                quit_plugin_button.color = edsBlue400;
            }
        }
    }
}

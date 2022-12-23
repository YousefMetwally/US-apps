// Qt imports
import QtQuick 2.15
import QtQuick.Window 2.15
import QtWebEngine 1.10

Rectangle {
    width: 0
    height: 0
    visible: true

    WebEngineView {
        objectName: "webView"
        anchors.fill: parent

        settings.screenCaptureEnabled: true

        onFeaturePermissionRequested: {
            console.log("FeaturePermission "+feature+" granted for "+securityOrigin)
            grantFeaturePermission(securityOrigin, feature, true);
        }

        function openURL(_url) {
            url = _url
        }
    }
}

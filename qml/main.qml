import QtQuick 2.11
import QtQuick.Window 2.11
import StrobeTuner 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Strobe Tuner")

    AudioStrobe {
        id: audioStrobe
    }

    Strobe {
        id: strobeView
        anchors.fill: parent
        diskAngle: audioStrobe.angle
    }
}

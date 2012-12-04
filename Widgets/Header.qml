import QtQuick 2.0
import Ubuntu.Components 0.1

Item {
    id: header
    property string text
    height: childrenRect.height
    clip: true

    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
    }
  
    Rectangle {
        id: topDivider
        height: units.dp(2)
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        color: "#f37505"
    }

    Item {
        id: headerLabel
        height: units.gu(8)
        anchors {
            top: topDivider.top
            left: parent.left
            right: parent.right
        }

        Label {
            text: header.text
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: units.gu(2)
            }
            fontSize: "x-large"
            color: "#333333"
            opacity: 0.4
        }
    }

    Image {
        id: divider
        anchors {
            top: headerLabel.bottom
            left: parent.left
            right: parent.right
        }
        source: "../assets/section_divider.png"
    }
}

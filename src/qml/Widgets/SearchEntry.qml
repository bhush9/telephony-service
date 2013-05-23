/*
 * Copyright 2010-2013 Canonical Ltd.
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Ubuntu.Components 0.1

FocusScope {
    id: searchEntry

    property alias text : searchInput.text
    property string searchQuery
    property string hint: ""
    property bool active: false
    property alias forceCursorVisible: searchInput.forceCursorVisible
    property alias anyKeypressGivesFocus: searchInput.anyKeypressGivesFocus

    property string leftIconSource
    property alias leftIconVisible: leftIcon.visible
    property string rightIconSource
    property alias rightIconVisible: rightIcon.visible
    property string backgroundSource: "../assets/input_field_background.sci"


    signal activateFirstResult
    signal leftIconClicked
    signal rightIconClicked

    // FIXME: check why it is not working.
    //Accessible.name: "Search Entry"

    /* Delete search when set in-active */
    onActiveChanged: if (!active) searchInput.text = ""

    /* Keys forwarded to the search entry are forwarded to the text input. */
    Keys.forwardTo: [searchInput]

    height: units.gu(4)
    width: units.gu(13)

    BorderImage {
        anchors.fill: parent
        source: searchEntry.backgroundSource
        smooth: false
    }

    Item {
        anchors.fill: parent
        anchors.leftMargin: units.gu(0.5)
        anchors.rightMargin: units.gu(0.5)
        anchors.topMargin: units.gu(0.5)
        anchors.bottomMargin: units.gu(0.5)

        Image {
            id: leftIcon

            anchors.left: parent.left
            anchors.leftMargin: units.gu(0.5)
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            sourceSize.height: height

            smooth: true
            source: leftIconSource
            fillMode: Image.PreserveAspectFit

            MouseArea {
                // FIXME: check why it is not working.
                //Accessible.name: "Left Icon"
                //Accessible.role: Accessible.PushButton
                anchors.fill: parent

                onClicked: {
                    searchInput.forceActiveFocus()
                    leftIconClicked()
                }
            }
        }

        Image {
            id: rightIcon

            anchors.right: parent.right
            anchors.rightMargin: units.gu(0.5)
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            sourceSize.height: height

            source: rightIconSource
            fillMode: Image.PreserveAspectFit
            smooth: true

            MouseArea {
                // FIXME: check why it is not working.
                //Accessible.name: "Right Icon"
                //Accessible.role: Accessible.PushButton
                anchors.fill: parent

                onClicked: {
                    searchInput.forceActiveFocus()
                    rightIconClicked()
                }
            }
        }

        TextInput {
            id: searchInput
            property bool forceCursorVisible: false
            property bool anyKeypressGivesFocus: false

            // FIXME: check why it is not working.
            //Accessible.name: searchInstructions.text
            //Accessible.role: Accessible.EditableText

            anchors.left: leftIcon.source != "" ? leftIcon.right : parent.left
            anchors.leftMargin: units.gu(0.5)
            anchors.right: rightIcon.source != "" ? rightIcon.left : parent.right
            anchors.rightMargin: units.gu(0.5)
            anchors.verticalCenter: parent.verticalCenter
            horizontalAlignment: Text.AlignLeft

            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            font.pixelSize: FontUtils.sizeToPixels("large")
            focus: true
            selectByMouse: true
            cursorDelegate: cursor
            selectionColor: "gray"

            onTextChanged: liveSearchTimeout.restart()

            Timer {
                id: liveSearchTimeout
                interval: 200
                onTriggered: searchQuery = searchInput.text
            }

            Keys.onPressed: {
                if (anyKeypressGivesFocus) {
                    forceActiveFocus();
                }
                if (event.key == Qt.Key_Return || event.key == Qt.Key_Enter) {
                    activateFirstResult()
                    event.accepted = true;
                }
                if (event.key == Qt.Key_Escape) {
                    if (searchInput.text != "") {
                        searchInput.text = ""
                        event.accepted = true;
                    }
                }
            }

            Component {
                id: cursor

                Rectangle {
                    property bool timerShowCursor: true

                    id: customCursor
                    color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.topMargin: units.dp(2)
                    anchors.bottomMargin: units.dp(2)
                    width: units.dp(1)
                    visible: (customCursor.parent.forceCursorVisible || parent.activeFocus) && timerShowCursor
                    Timer {
                        interval: 800; running: (customCursor.parent.forceCursorVisible || customCursor.parent.activeFocus); repeat: true
                        onTriggered: {
                            interval = interval == 800 ? 400 : 800
                            customCursor.timerShowCursor = !customCursor.timerShowCursor
                        }
                    }
                }
            }

            Label {
                id: searchInstructions

                anchors.left: parent.left
                anchors.right: parent.right
                elide: Text.ElideRight
                anchors.verticalCenter: parent.verticalCenter
                LayoutMirroring.enabled: false
                //horizontalAlignment: Utils.isRightToLeft() ? Text.AlignRight : Text.AlignLeft

                color: Qt.rgba(0.7, 0.7, 0.7, 1.0)
                fontSize: "small"
                text: {
                    if(searchInput.text || searchInput.inputMethodComposing)
                        return ""
                    else
                        return searchEntry.hint
                }
            }
        }
    }
    /*
    CursorShapeArea {
        anchors.fill: parent
        cursorShape: Qt.IBeamCursor
    }
    */
}
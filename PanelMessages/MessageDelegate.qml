import QtQuick 1.1
import "../Widgets"
import "../dateUtils.js" as DateUtils
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

ListItem.Base {
    id: messageDelegate
    height: 60

    CustomListItemBase {
        id: baseInfo

        anchors.fill: parent
        iconSource: (avatar != "") ? avatar : placeholderIconSource
        placeholderIconSource: "../assets/avatar_contacts_list.png"
        text: contactAlias
        subtext: message
        textBold: unreadCount > 0
        selected: messageDelegate.selected

        onClicked: messageDelegate.clicked()
    }

    TextCustom {
        id: subsublabel

        anchors.baseline: parent.bottom
        anchors.baselineOffset: -baseInfo.padding + 4
        anchors.right: parent.right
        anchors.rightMargin: baseInfo.padding - 2
        horizontalAlignment: Text.AlignRight
        fontSize: "x-small"

        color: baseInfo.__textColor
        style: Text.Raised
        styleColor: "white"
        opacity: messageDelegate.enabled ? 1.0 : 0.5
        text: DateUtils.formatLogDate(timestamp)

    }
}

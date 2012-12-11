import QtQuick 2.0
import "../Widgets" as LocalWidgets
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

Item {
    id: messageList

    property alias model: listView.model

    ListView {
        id: listView
        anchors.fill: parent
        clip: true
        // We are not using sections right now, re-enable that in the future in case the
        // design changes.
        //section.property: "date"
        //section.delegate: ListItem.Divider { }

        delegate: CommunicationDelegate {
            item: (model && model.item) ? model.item : null
            title: (model && model.contactAlias) ? model.contactAlias : ""
            //subtitle: "(TODO: show phone type)"
            text: (model && model.item && model.item.message) ? model.item.message : ""
            timestamp: (model && model.timestamp) ? model.timestamp : null
            avatar: (model && model.contactAvatar) ? model.contactAvatar : ""
            itemIcon:  {
                switch (itemType) {
                case "message":
                    "../assets/messages.png";
                    break;
                case "call":
                    if (item.missed) {
                        "../assets/missed-call.png"
                    } else if (item.incoming) {
                        "../assets/incoming-call.png"
                    } else {
                        "../assets/outgoing-call.png"
                    }
                    break;
                case "group":
                    "../assets/tab_icon_contacts_inactive.png";
                    break;
                default:
                    "";
                    break;
                }
            }

            selected: isSelected()

            function isSelected() {
                if (!model || !model.groupingProperty) {
                    return false;
                }

                if (!telephony.view || !telephony.view.filterProperty) {
                    return false;
                }

                return (telephony.view.filterProperty == model.groupingProperty) &&
                        (telephony.view.filterValue == model.item[model.groupingProperty]);
            }

            onClicked: {
                telephony.showCommunication(model.groupingProperty, model.item[groupingProperty], "", model.contactId, true);
            }
        }
    }

    Scrollbar {
        flickableItem: listView
        align: Qt.AlignTrailing
        __interactive: false
    }
}

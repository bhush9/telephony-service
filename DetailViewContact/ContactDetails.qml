import QtQuick 2.0
import TelephonyApp 0.1
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem
import "../Widgets" as LocalWidgets
import "../"
import "DetailTypeUtilities.js" as DetailTypes

LocalWidgets.TelephonyPage {
    id: contactDetails

    property bool editable: false
    property alias contact: contactWatcher.contact
    property variant contactId: (contact) ? contact.id : null
    property bool added: false

    ContactWatcher {
        id: contactWatcher
    }

    onContactChanged: editable = false

    title: "Contact Details"
    width: units.gu(50)
    height: units.gu(75)

    ListModel {
        id: editButtons
        ListElement {
            label: "Delete"
            name: "delete"
            icon: "../assets/delete.png"
        }

        ListElement {
            label: "Cancel"
            name: "cancel"
            icon: "../assets/cancel.png"
        }
        ListElement {
            label: "Save"
            name: "save"
            icon: "../assets/save.png"
        }
    }

    ListModel {
        id: newContactButtons

        ListElement {
            label: "Cancel"
            name: "cancel"
            icon: "../assets/cancel.png"
        }
        ListElement {
            label: "Save"
            name: "save"
            icon: "../assets/save.png"
        }
    }

    ListModel {
        id: standardButtons

        ListElement {
            label: "Edit"
            name: "edit"
            icon: "../assets/edit.png"
        }
    }

    chromeButtons: bottomButtons()

    onChromeButtonClicked: {
        switch (buttonName) {
        case "delete":
            // FIXME: show a dialog asking for confirmation
            contactModel.removeContact(contact);
            telephony.resetView();
            break;
        case "cancel":
            if (added) {
                telephony.resetView();
            } else {
                contact.revertChanges();
                editable = false;
            }
            break;
        case "save":
            contactDetails.save();
            break;
        case "edit":
            editable = true;
            break;

        }
    }

    function bottomButtons() {
        if (editable) {
            if (added) {
                return newContactButtons;
            } else {
                return editButtons;
            }
        } else {
            return standardButtons;
        }
    }

    function createNewContact() {
        contact = Qt.createQmlObject("import TelephonyApp 0.1; ContactEntry {}", contactModel);
        editable = true;
        added = true;

        for (var i = 0; i < detailsList.children.length; i++) {
            var child = detailsList.children[i];
            if (child.detailTypeInfo && child.detailTypeInfo.createOnNew) {
                child.appendNewItem();
            }
        }

        header.focus = true;
    }

    function save() {
        /* We ask each detail delegate to save all edits to the underlying
           model object. The other way to do it would be to change editable
           to false and catch onEditableChanged in the delegates and save there.
           However that other way doesn't work since we can't guarantee that all
           delegates have received the signal before we call contact.save() here.
        */
        header.save();

        var addedDetails = [];
        for (var i = 0; i < detailsList.children.length; i++) {
            var saver = detailsList.children[i].save;
            if (saver && saver instanceof Function) {
                var newDetails = saver();
                for (var k = 0; k < newDetails.length; k++)
                    addedDetails.push(newDetails[k]);
            }
        }

        for (i = 0; i < addedDetails.length; i++) {
            console.log("Add detail: " + contact.addDetail(addedDetails[i]));
        }

        if (contact.modified || added)
            contactModel.saveContact(contact);

        editable = false;
        added = false;
    }

    Connections {
        target: contactModel
        onContactSaved: {
            // once the contact gets saved after editing, we reload it in the view
            // because for added contacts, we need the newly created ContactEntry instead of the one
            // we were using before.
            contactWatcher.contact = null;
            // empty contactId because if it remains same, contact watcher wont search
            // for a new contact
            contactWatcher.contactId = ""
            contactWatcher.contactId = contactId;
        }

        onContactRemoved: {
            if (contactId == contactDetails.contactId) {
                contactDetails.contact = null;
                telephony.resetView();
            }
        }
    }

    ContactDetailsHeader {
        id: header
        contact: contactDetails.contact
        editable: contactDetails.editable
        focus: true
    }

    ListItem.ThinDivider {
        id: bottomDividerLine
        anchors.bottom: header.bottom
    }

    Flickable {
        id: scrollArea

        anchors.top: header.bottom
        anchors.bottom: editFooter.top
        anchors.left: parent.left
        anchors.right: parent.right
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        contentHeight: detailsList.height +
                       (contactDetails.editable ? newDetailChooser.height + newDetailChooser.menuHeight + units.gu(1) : 0)

        Column {
            id: detailsList

            anchors.left: parent.left
            anchors.right: parent.right

            Repeater {
                model: (contact) ? DetailTypes.supportedTypes : []

                delegate: ContactDetailsSection {
                    anchors.left: (parent) ? parent.left : undefined
                    anchors.right: (parent) ? parent.right : undefined

                    detailTypeInfo: modelData
                    editable: contactDetails.editable
                    onDetailAdded: focus = true

                    model: (contact) ? contact[modelData.items] : []
                    delegate: Loader {
                        anchors.left: (parent) ? parent.left : undefined
                        anchors.right: (parent) ? parent.right : undefined

                        source: detailTypeInfo.delegateSource

                        Binding { target: item; property: "detail"; value: modelData }
                        Binding { target: item; property: "detailTypeInfo"; value: detailTypeInfo }
                        Binding { target: item; property: "editable"; value: contactDetails.editable }

                        Connections {
                            target: item
                            ignoreUnknownSignals: true

                            onDeleteClicked: contact.removeDetail(modelData)
                            onActionClicked: {
                                switch(modelData.type) {
                                case ContactDetail.PhoneNumber:
                                    var filterProperty = "contactId";
                                    var filterValue = contact.id;
                                    var phoneNumber = modelData.number;
                                    telephony.showCommunication(filterProperty, filterValue, phoneNumber, contact.id, true);
                                    break;
                                case ContactDetail.EmailAddress:
                                    Qt.openUrlExternally("mailto:" + modelData.emailAddress);
                                    break;
                                default:
                                    break;
                                }
                            }
                            onClicked: {
                                switch (modelData.type) {
                                case ContactDetail.PhoneNumber:
                                    telephony.callNumber(modelData.number);
                                    break;
                                case ContactDetail.EmailAddress:
                                    Qt.openUrlExternally("mailto:" + modelData.emailAddress);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        ContactDetailTypeChooser {
            id: newDetailChooser

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: detailsList.bottom
            anchors.leftMargin: units.dp(1)
            anchors.rightMargin: units.dp(1)

            opacity: (editable) ? 1.0 : 0.0
            contact: (editable) ? contactDetails.contact : null
            height: (editable) ? units.gu(4) : 0

            onSelected: {
                for (var i = 0; i < detailsList.children.length; i++) {
                    var child = detailsList.children[i];
                    if (child.detailTypeInfo.name == detailType.name) {
                        child.appendNewItem();
                        return;
                    }
                }
            }

            onOpenedChanged: if (opened) scrollArea.contentY = scrollArea.contentHeight
        }
    }

    Scrollbar {
        flickableItem: scrollArea
        align: Qt.AlignTrailing
        __interactive: false
    }

    Item {
        id: editFooter

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        visible: !telephony.singlePane
        height: visible ? units.gu(5) : 0

        Rectangle {
            anchors.fill: parent
            color: "white"
            opacity: 0.5
        }

        Rectangle {
            id: separator

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: units.dp(1)
            color: "white"
        }

        Item {
            id: footerButtons

            anchors.top: separator.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            Button {
                id: deleteButton

                height: units.gu(3)
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: units.gu(1)
                text: "Delete"
                opacity: (editable && !added) ? 1.0 : 0.0

                onClicked: {
                    // FIXME: show a dialog asking for confirmation
                    contactModel.removeContact(contact);
                    telephony.resetView();
                }
            }

            Button {
                id: cancelButton

                height: units.gu(3)
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: editSaveButton.left
                anchors.rightMargin: units.gu(1)
                text: "Cancel"
                opacity: (editable) ? 1.0 : 0.0
                onClicked: {
                    if (added) {
                        telephony.resetView();
                    } else {
                        contact.revertChanges();
                        editable = false;
                    }
                }
            }

            Button {
                id: editSaveButton
                objectName: "editSaveButton"

                height: units.gu(3)
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: units.gu(1)
                color: editable ? "#dd4f22" : "#e3e5e8"
                text: (editable) ? "Save" : "Edit"
                enabled: !editable || header.contactNameValid
                onClicked: {
                    if (!editable) editable = true;
                    else {
                        contactDetails.save();
                    }
                }
            }
        }
    }
}

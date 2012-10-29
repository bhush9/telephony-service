import QtQuick 2.0
import Ubuntu.Components 0.1

// FIXME: BUG WHEN SEARCHING

FocusScope {
    id: dropDownSearch
    property alias searchQuery: entry.searchQuery
    property alias model: searchView.model
    property alias delegate: searchView.delegate
    property alias count: searchView.count
    property int listMaximumHeight: units.gu(50)
    property alias text: entry.text
    property int currentIndex: -1
    property alias hint: entry.hint

    property alias leftIconSource: entry.leftIconSource
    property alias leftIconVisible: entry.leftIconVisible
    property alias rightIconSource: entry.rightIconSource
    property alias rightIconVisible: entry.rightIconVisible

    signal leftIconClicked
    signal rightIconClicked
    signal activateFirstResult

    signal itemSelected(variant item)
    state: "idle"
    height: entry.height

    SearchEntry {
        id: entry
        anchors.left: parent.left
        anchors.right: parent.right
        onTextChanged: text != "" ? dropDownSearch.state = "searching" : dropDownSearch.state = "idle"
        onLeftIconClicked: parent.leftIconClicked()
        onRightIconClicked: parent.rightIconClicked()
        onActivateFirstResult: parent.activateFirstResult()
        backgroundSource: dropDownSearch.state == "searching" && searchView.count > 0 ? "../assets/input_field_autofill_top.sci" : "../assets/input_field_background.sci"
        focus: true
    }

    ListView {
        id: searchView
        anchors.top: entry.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        // FIXME: Does not uses contentHeight here
        height: visible ? (contentHeight >= listMaximumHeight ? listMaximumHeight : contentHeight) : 0
        Behavior on height {StandardAnimation {}}
        clip: true
    }

    states: [
        State {
            name: "searching"
            PropertyChanges { target: searchView; visible: true }
        },
        State {
            name: "idle"
            PropertyChanges { target: searchView; visible: false }
        }
    ]
}

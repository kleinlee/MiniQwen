import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import AAAAA

Rectangle {
    width: 500
    height: 800
    color: "#000000FF"

    property string inConversationWith : "huahua"

    Rectangle {
        id: rect0
        anchors.fill: parent
        ColumnLayout {
            anchors.fill: parent

            ListView {
                id: listView
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: pane.leftPadding + messageField.leftPadding
                displayMarginBeginning: 40
                displayMarginEnd: 40
                verticalLayoutDirection: ListView.BottomToTop
                spacing: 12
                model: SqlConversationModel {
                    recipient: inConversationWith
                }
                delegate: Column {
                    anchors.right: sentByMe ? listView.contentItem.right : undefined
                    spacing: 6

                    readonly property bool sentByMe: model.recipient !== "Me"

                    Row {
                        id: messageRow
                        spacing: 6
                        anchors.right: sentByMe ? parent.right : undefined

                        Image {
                            id: avatar
                            width: 50
                            height: 50
                            source: !sentByMe ? "images/" + model.author.replace(" ", "_") + ".jpg" : ""
                        }

                        Rectangle {
                            width: Math.min(messageText.implicitWidth + 24, listView.width - avatar.width - messageRow.spacing)
                            height: messageText.implicitHeight + 24
                            radius: 10
                            color: sentByMe ? "lightgrey" : "steelblue"

                            TextEdit  {
                                id: messageText
                                text: model.message
                                font.pixelSize: 14
                                color: sentByMe ? "black" : "white"
                                anchors.fill: parent
                                anchors.margins: 12
                                wrapMode: Text.WordWrap
                                selectByMouse: true
                                readOnly: true
                            }
                        }
                    }

                    Label {
                        id: timestampText
                        text: Qt.formatDateTime(model.timestamp, "d MMM hh:mm")
                        color: "lightgrey"
                        anchors.right: sentByMe ? parent.right : undefined
                    }
                }

                ScrollBar.vertical: ScrollBar {}
            }

            Pane {
                id: pane
                Layout.fillWidth: true

                RowLayout {
                    width: parent.width

                    TextArea {
                        id: messageField
                        Layout.fillWidth: true
                        placeholderText: qsTr("Compose message")
                        wrapMode: TextArea.Wrap
                        Keys.onPressed: (event)=> {
                            if (event.key === Qt.Key_Return) {
                                listView.model.sendMessage(inConversationWith, messageField.text);
                                messageField.text = "";
                            }
                        }
                        onTextChanged: {
                            if (length > 30) remove(30, length);
                        }
                    }

                    Button {
                        id: sendButton
                        text: qsTr("发送")
                        enabled: messageField.length > 0
                        onClicked: {
                            listView.model.sendMessage(inConversationWith, messageField.text);
                            messageField.text = "";
                        }
                    }
                    Button {
                        id: clearButton
                        text: qsTr("清空历史")
                        onClicked: {
                            listView.model.removeAllMessage();
                        }
                    }
                }
            }
        }
    }
}

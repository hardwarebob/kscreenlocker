/*
SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2023 Nate Graham <nate@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls as QQC2

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kquickcontrolsaddons 2.0
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.private.sessions 2.0

Item {
    id: lockScreen

    property alias capsLockOn: unlockUI.capsLockOn
    property bool locked: false

    signal unlockRequested()

    Rectangle {
        width: parent.width
        height: parent.height
        color: "#1d99f3"
    }

    SessionManagement {
        id: sessionManagment
    }

    Rectangle {
        visible: typeof remoteSessions !== "undefined" && remoteSessions.hasActiveSessions
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: Kirigami.Units.largeSpacing * 2
        z: 10
        radius: Kirigami.Units.cornerRadius
        color: Qt.rgba(0, 0, 0, 0.7)
        width: remoteBannerRow.implicitWidth + Kirigami.Units.largeSpacing * 2
        height: remoteBannerRow.implicitHeight + Kirigami.Units.largeSpacing

        Row {
            id: remoteBannerRow
            anchors.centerIn: parent
            spacing: Kirigami.Units.largeSpacing

            QQC2.Label {
                anchors.verticalCenter: parent.verticalCenter
                color: "white"
                text: {
                    if (typeof remoteSessions === "undefined" || !remoteSessions.hasActiveSessions) {
                        return "";
                    }
                    const s = remoteSessions.activeSessions[0];
                    return i18nd("kscreenlocker_greet", "Active remote session: %1 (%2)", s.provider, s.clientName);
                }
            }

            QQC2.CheckBox {
                anchors.verticalCenter: parent.verticalCenter
                checked: typeof remoteSessions !== "undefined" && remoteSessions.disconnectOnUnlock
                text: i18nd("kscreenlocker_greet", "Disconnect on unlock")
                onToggled: {
                    if (typeof remoteSessions !== "undefined") {
                        remoteSessions.disconnectOnUnlock = checked;
                    }
                }
            }
        }
    }

    KSvg.FrameSvgItem {
        id: dialog

        visible: lockScreen.locked
        anchors.centerIn: parent
        width: (mainStack.currentItem?.implicitWidth ?? 0) + margins.left + margins.right
        height: (mainStack.currentItem?.implicitHeight ?? 0) + margins.top + margins.bottom
        imagePath: "widgets/background"

        Behavior on height {
            enabled: mainStack.currentItem != null
            NumberAnimation {
                duration: Kirigami.Units.longDuration
            }
        }
        Behavior on width {
            enabled: mainStack.currentItem != null
            NumberAnimation {
                duration: Kirigami.Units.longDuration
            }
        }

        QQC2.StackView {
            id: mainStack

            clip: true
            anchors {
                fill: parent
                leftMargin: dialog.margins.left
                topMargin: dialog.margins.top
                rightMargin: dialog.margins.right
                bottomMargin: dialog.margins.bottom
            }
            initialItem: unlockUI
        }
    }

    Greeter {
        id: unlockUI

        switchUserEnabled: sessionManagment.canSwitchUser

        visible: opacity > 0
        opacity: mainStack.currentItem == unlockUI
        Behavior on opacity {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
            }
        }

        onSwitchUserClicked: sessionManagment.switchUser()
    }

    function returnToLogin() {
        mainStack.pop();
        unlockUI.resetFocus();
    }
}

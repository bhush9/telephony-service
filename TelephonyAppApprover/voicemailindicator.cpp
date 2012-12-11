/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "voicemailindicator.h"
#include "telepathyhelper.h"
#include "messagingmenu.h"
#include <QDebug>
#include <QDBusReply>

#define CANONICAL_IFACE_TELEPHONY "com.canonical.Telephony"

VoiceMailIndicator::VoiceMailIndicator(QObject *parent)
: QObject(parent),
  mConnection(QDBusConnection::sessionBus())
{
    if(!checkConnected()) {
        connect(TelepathyHelper::instance(), SIGNAL(accountReady()), SLOT(onAccountReady()));
        connect(TelepathyHelper::instance(), SIGNAL(connectionChanged()), SLOT(onAccountReady()));
    } else {
        onAccountReady();
    }
}

void VoiceMailIndicator::showVoicemailOnApp()
{
    QDBusInterface telephonyApp("com.canonical.TelephonyApp",
                                "/com/canonical/TelephonyApp",
                                "com.canonical.TelephonyApp");
    telephonyApp.call("ShowVoicemail");
}

bool VoiceMailIndicator::checkConnected()
{
    return TelepathyHelper::instance()->account() && TelepathyHelper::instance()->account()->connection();
}

void VoiceMailIndicator::onAccountReady()
{
    if (!checkConnected()) {
        return;
    }

    Tp::ConnectionPtr conn(TelepathyHelper::instance()->account()->connection());
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    mConnection.connect(busName, objectPath, CANONICAL_IFACE_TELEPHONY, QLatin1String("VoicemailCountChanged"),
                        this, SLOT(onVoicemailCountChanged(int)));

    mConnection.connect(busName, objectPath, CANONICAL_IFACE_TELEPHONY, QLatin1String("VoicemailIndicatorChanged"),
                        this, SLOT(onVoicemailIndicatorChanged(bool)));

    onVoicemailCountChanged(voicemailCount());
}

void VoiceMailIndicator::onVoicemailIndicatorChanged(bool active)
{
    if (active) {
        MessagingMenu::instance()->showVoicemailEntry(voicemailCount());
    } else {
        MessagingMenu::instance()->hideVoicemailEntry();
    }
}

bool VoiceMailIndicator::voicemailIndicatorVisible()
{
    if (!checkConnected()) {
        return false;
    }

    Tp::ConnectionPtr conn(TelepathyHelper::instance()->account()->connection());
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    QDBusInterface connIface(busName, objectPath, CANONICAL_IFACE_TELEPHONY);
    QDBusReply<bool> reply = connIface.call("VoicemailIndicator");
    if (reply.isValid()) {
        return reply.value();
    }
    return false;
}

int VoiceMailIndicator::voicemailCount()
{
    if (!checkConnected()) {
        return 0;
    }

    Tp::ConnectionPtr conn(TelepathyHelper::instance()->account()->connection());
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    QDBusInterface connIface(busName, objectPath, CANONICAL_IFACE_TELEPHONY);
    QDBusReply<int> reply = connIface.call("VoicemailCount");
    if (reply.isValid()) {
        return reply.value();
    }
    return false;
}

void VoiceMailIndicator::onVoicemailCountChanged(int count)
{
    onVoicemailIndicatorChanged(voicemailIndicatorVisible());
}

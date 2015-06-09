/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of history-service.
 *
 * history-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * history-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "connection.h"
#include "mockconnectiondbus.h"
#include "mockconnectionadaptor.h"

Q_DECLARE_METATYPE(QList< QVariantMap >)

MockConnectionDBus::MockConnectionDBus(MockConnection *parent) :
    QObject(parent), mAdaptor(0), mConnection(parent)
{
    connect(mConnection,
            SIGNAL(messageRead(QString)),
            SIGNAL(MessageRead(QString)));
    connect(mConnection,
            SIGNAL(messageSent(QString,QVariantMap)),
            SIGNAL(MessageSent(QString,QVariantMap)));
    connect(mConnection,
            SIGNAL(callReceived(QString)),
            SIGNAL(CallReceived(QString)));
    connect(mConnection,
            SIGNAL(callEnded(QString)),
            SIGNAL(CallEnded(QString)));
    connect(mConnection,
            SIGNAL(callStateChanged(QString,QString,QString)),
            SIGNAL(CallStateChanged(QString,QString,QString)));
    connect(mConnection,
            SIGNAL(conferenceCreated(QString)),
            SIGNAL(ConferenceCreated(QString)));
    connect(mConnection,
            SIGNAL(channelMerged(QString)),
            SIGNAL(ChannelMerged(QString)));
    connect(mConnection,
            SIGNAL(channelSplitted(QString)),
            SIGNAL(ChannelSplitted(QString)));
    connect(mConnection,
            SIGNAL(ussdInitiateCalled(QString)),
            SIGNAL(USSDInitiateCalled(QString)));
    connect(mConnection,
            SIGNAL(ussdRespondCalled(QString)),
            SIGNAL(USSDRespondCalled(QString)));
    connect(mConnection,
            SIGNAL(ussdCancelCalled()),
            SIGNAL(USSDCancelCalled()));
    connect(mConnection,
            SIGNAL(disconnected()),
            SIGNAL(Disconnected()));
    connect(mConnection,
            SIGNAL(destroyed()),
            SIGNAL(Destroyed()));
    qDBusRegisterMetaType<QList<QVariantMap> >();
    mObjectPath = "/com/canonical/MockConnection/" + mConnection->protocolName();
    connectToBus();
}

MockConnectionDBus::~MockConnectionDBus()
{
    QDBusConnection::sessionBus().unregisterObject(mObjectPath, QDBusConnection::UnregisterTree);
}

bool MockConnectionDBus::connectToBus()
{
    bool ok = QDBusConnection::sessionBus().registerService("com.canonical.MockConnection");
    if (!ok) {
        return false;
    }

    if (!mAdaptor) {
        mAdaptor = new MockConnectionAdaptor(this);
    }

    return QDBusConnection::sessionBus().registerObject(mObjectPath, this);
}

void MockConnectionDBus::PlaceIncomingMessage(const QString &message, const QVariantMap &properties)
{
    qDebug() << __PRETTY_FUNCTION__ << message << properties;
    mConnection->placeIncomingMessage(message, properties);
}

QString MockConnectionDBus::PlaceCall(const QVariantMap &properties)
{
    qDebug() << __PRETTY_FUNCTION__ << properties;
    return mConnection->placeCall(properties);
}

void MockConnectionDBus::HangupCall(const QString &callerId)
{
    qDebug() << __PRETTY_FUNCTION__ << callerId;
    mConnection->hangupCall(callerId);
}

void MockConnectionDBus::SetCallState(const QString &phoneNumber, const QString &state)
{
    qDebug() << __PRETTY_FUNCTION__ << phoneNumber << state;
    mConnection->setCallState(phoneNumber, state);
}

void MockConnectionDBus::SetOnline(bool online)
{
    qDebug() << __PRETTY_FUNCTION__ << online;
    mConnection->setOnline(online);
}

void MockConnectionDBus::SetPresence(const QString &status, const QString &statusMessage)
{
    qDebug() << __PRETTY_FUNCTION__ << status << statusMessage;
    Tp::DBusError error;
    mConnection->setPresence(status, statusMessage, &error);
}

void MockConnectionDBus::SetVoicemailIndicator(bool active)
{
    qDebug() << __PRETTY_FUNCTION__ << active;
    mConnection->setVoicemailIndicator(active);
}

void MockConnectionDBus::SetVoicemailNumber(const QString &number)
{
    qDebug() << __PRETTY_FUNCTION__ << number;
    mConnection->setVoicemailNumber(number);
}

void MockConnectionDBus::SetVoicemailCount(int count)
{
    qDebug() << __PRETTY_FUNCTION__ << count;
    mConnection->setVoicemailCount(count);
}

void MockConnectionDBus::SetEmergencyNumbers(const QStringList &numbers)
{
    qDebug() << __PRETTY_FUNCTION__ << numbers;
    mConnection->setEmergencyNumbers(numbers);
}

QString MockConnectionDBus::Serial()
{
    qDebug() << __PRETTY_FUNCTION__ << mConnection->serial();
    return mConnection->serial();
}

void MockConnectionDBus::TriggerUSSDNotificationReceived(const QString &message)
{
    qDebug() << __PRETTY_FUNCTION__ << message;
    mConnection->supplementaryServicesIface->NotificationReceived(message);
}

void MockConnectionDBus::TriggerUSSDRequestReceived(const QString &message)
{
    qDebug() << __PRETTY_FUNCTION__ << message;
    mConnection->supplementaryServicesIface->RequestReceived(message);
}

void MockConnectionDBus::TriggerUSSDInitiateUSSDComplete(const QString &ussdResp)
{
    qDebug() << __PRETTY_FUNCTION__ << ussdResp;
    mConnection->supplementaryServicesIface->InitiateUSSDComplete(ussdResp);
}

void MockConnectionDBus::TriggerUSSDRespondComplete(bool success, const QString &ussdResp)
{
    qDebug() << __PRETTY_FUNCTION__ << ussdResp;
    mConnection->supplementaryServicesIface->RespondComplete(success, ussdResp);
}

void MockConnectionDBus::TriggerUSSDBarringComplete(const QString &ssOp, const QString &cbService, const QVariantMap &cbMap)
{
    qDebug() << __PRETTY_FUNCTION__ << ssOp << cbService << cbMap;
    mConnection->supplementaryServicesIface->BarringComplete(ssOp, cbService, cbMap);
}

void MockConnectionDBus::TriggerUSSDForwardingComplete(const QString &ssOp, const QString &cfService, const QVariantMap &cfMap)
{
    qDebug() << __PRETTY_FUNCTION__ << ssOp << cfService << cfMap;
    mConnection->supplementaryServicesIface->ForwardingComplete(ssOp, cfService, cfMap);
}

void MockConnectionDBus::TriggerUSSDWaitingComplete(const QString &ssOp, const QVariantMap &cwMap)
{
    qDebug() << __PRETTY_FUNCTION__ << ssOp << cwMap;
    mConnection->supplementaryServicesIface->WaitingComplete(ssOp, cwMap);
}

void MockConnectionDBus::TriggerUSSDCallingLinePresentationComplete(const QString &ssOp, const QString &status)
{
    qDebug() << __PRETTY_FUNCTION__ << ssOp << status;
    mConnection->supplementaryServicesIface->CallingLinePresentationComplete(ssOp, status);
}

void MockConnectionDBus::TriggerUSSDConnectedLinePresentationComplete(const QString &ssOp, const QString &status)
{
    qDebug() << __PRETTY_FUNCTION__ << ssOp << status;
    mConnection->supplementaryServicesIface->ConnectedLinePresentationComplete(ssOp, status);
}

void MockConnectionDBus::TriggerUSSDCallingLineRestrictionComplete(const QString &ssOp, const QString &status)
{
    qDebug() << __PRETTY_FUNCTION__ << ssOp << status;
    mConnection->supplementaryServicesIface->CallingLineRestrictionComplete(ssOp, status);
}

void MockConnectionDBus::TriggerUSSDConnectedLineRestrictionComplete(const QString &ssOp, const QString &status)
{
    qDebug() << __PRETTY_FUNCTION__ << ssOp << status;
    mConnection->supplementaryServicesIface->ConnectedLineRestrictionComplete(ssOp, status);
}

void MockConnectionDBus::TriggerUSSDInitiateFailed()
{
    qDebug() << __PRETTY_FUNCTION__;
    mConnection->supplementaryServicesIface->InitiateFailed();
}

void MockConnectionDBus::TriggerUSSDStateChanged(const QString &state)
{
    qDebug() << __PRETTY_FUNCTION__ << state;
    mConnection->supplementaryServicesIface->StateChanged(state);
}

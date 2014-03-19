/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "callmanager.h"
#include "callentry.h"
#include "telepathyhelper.h"

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>
#include <QDBusInterface>

typedef QMap<QString, QVariant> dbusQMap;
Q_DECLARE_METATYPE(dbusQMap)

CallManager *CallManager::instance()
{
    static CallManager *self = new CallManager();
    return self;
}

CallManager::CallManager(QObject *parent)
: QObject(parent), mNeedsUpdate(false)
{
    connect(TelepathyHelper::instance(), SIGNAL(connectedChanged()), SLOT(onConnectedChanged()));
    connect(TelepathyHelper::instance(), SIGNAL(channelObserverUnregistered()), SLOT(onChannelObserverUnregistered()));
}

void CallManager::onChannelObserverUnregistered()
{
    // do not clear the manager right now, wait until the observer is re-registered
    // to avoid flickering in the UI
    mNeedsUpdate = true;
}

void CallManager::startCall(const QString &phoneNumber, const QString &accountId)
{
    Tp::AccountPtr account;
    if (accountId.isNull()) {
        account = TelepathyHelper::instance()->accounts()[0];
    } else {
        account = TelepathyHelper::instance()->accountForId(accountId);
    }

    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->call("StartCall", phoneNumber, account->uniqueIdentifier());
}

void CallManager::onConnectedChanged()
{
    if (!TelepathyHelper::instance()->connected()) {
        mVoicemailNumber = QString();
        Q_EMIT voicemailNumberChanged();
        return;
    }

    // FIXME: needs to handle voicemail numbers from multiple accounts
    Tp::ConnectionPtr conn(TelepathyHelper::instance()->accounts()[0]->connection());
    QString busName = conn->busName();
    QString objectPath = conn->objectPath();
    QDBusInterface connIface(busName, objectPath, CANONICAL_TELEPHONY_VOICEMAIL_IFACE);
    QDBusReply<QString> replyNumber = connIface.call("VoicemailNumber");
    if (replyNumber.isValid()) {
        mVoicemailNumber = replyNumber.value();
        Q_EMIT voicemailNumberChanged();
    }
}

CallEntry *CallManager::foregroundCall() const
{
    CallEntry *call = 0;

    // if we have only one call, return it as being always in foreground
    // even if it is held
    if (mCallEntries.count() == 1) {
        call = mCallEntries.first();
    } else {
        Q_FOREACH(CallEntry *entry, mCallEntries) {
            if (entry->isActive() && !entry->isHeld()) {
                call = entry;
                break;
            }
        }
    }

    if (!call) {
        return 0;
    }

    // incoming but not yet answered calls cant be considered foreground calls
    if (call->ringing()) {
        return 0;
    }

    return call;
}

CallEntry *CallManager::backgroundCall() const
{
    // if we have only one call, assume there is no call in background
    // even if the foreground call is held
    if (mCallEntries.count() == 1) {
        return 0;
    }

    Q_FOREACH(CallEntry *entry, mCallEntries) {
        if (entry->isHeld()) {
            return entry;
        }
    }

    return 0;
}

bool CallManager::hasCalls() const
{
    // check if the callmanager already has active calls
    if (activeCallsCount() > 0) {
        return true;
    }

    // if that's not the case, query the teleophony-service-handler for the availability of calls
    // this is done only to get the live call view on clients as soon as possible, even before the
    // telepathy observer is configured
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    QDBusReply<bool> reply = phoneAppHandler->call("HasCalls");
    if (reply.isValid()) {
        return reply.value();
    }

    return false;
}

bool CallManager::hasBackgroundCall() const
{
    return activeCallsCount() > 1;
}

void CallManager::onCallChannelAvailable(Tp::CallChannelPtr channel)
{
    // if this is the first call after re-registering the observer, clear the data
    if (mNeedsUpdate) {
        Q_FOREACH(CallEntry *entry, mCallEntries) {
            entry->deleteLater();
        }
        mCallEntries.clear();
        mNeedsUpdate = false;
    }

    CallEntry *entry = new CallEntry(channel, this);
    if (entry->phoneNumber() == getVoicemailNumber()) {
        entry->setVoicemail(true);
    }

    mCallEntries.append(entry);
    connect(entry,
            SIGNAL(callEnded()),
            SLOT(onCallEnded()));
    connect(entry,
            SIGNAL(heldChanged()),
            SIGNAL(foregroundCallChanged()));
    connect(entry,
            SIGNAL(activeChanged()),
            SIGNAL(foregroundCallChanged()));
    connect(entry,
            SIGNAL(heldChanged()),
            SIGNAL(backgroundCallChanged()));
    connect(entry,
            SIGNAL(activeChanged()),
            SIGNAL(hasBackgroundCallChanged()));

    // FIXME: check which of those signals we really need to emit here
    Q_EMIT hasCallsChanged();
    Q_EMIT hasBackgroundCallChanged();
    Q_EMIT foregroundCallChanged();
    Q_EMIT backgroundCallChanged();
}

void CallManager::onCallEnded()
{
    // FIXME: handle multiple calls
    CallEntry *entry = qobject_cast<CallEntry*>(sender());
    if (!entry) {
        return;
    }

    // at this point the entry should be removed
    mCallEntries.removeAll(entry);

    notifyEndedCall(entry->channel());
    Q_EMIT hasCallsChanged();
    Q_EMIT hasBackgroundCallChanged();
    Q_EMIT foregroundCallChanged();
    Q_EMIT backgroundCallChanged();
    entry->deleteLater();
}

QString CallManager::getVoicemailNumber()
{
    return mVoicemailNumber;
}


void CallManager::notifyEndedCall(const Tp::CallChannelPtr &channel)
{
    Tp::Contacts contacts = channel->remoteMembers();
    Tp::AccountPtr account = TelepathyHelper::instance()->accountForConnection(channel->connection());
    if (contacts.isEmpty() || !account) {
        qWarning() << "Call channel had no remote contacts:" << channel;
        return;
    }

    QString phoneNumber;
    // FIXME: handle conference call
    Q_FOREACH(const Tp::ContactPtr &contact, contacts) {
        phoneNumber = contact->id();
        break;
    }

    // fill the call info
    QDateTime timestamp = channel->property("timestamp").toDateTime();
    bool incoming = channel->initiatorContact() != account->connection()->selfContact();
    QTime duration(0, 0, 0);
    bool missed = incoming && channel->callStateReason().reason == Tp::CallStateChangeReasonNoAnswer;

    if (!missed) {
        QDateTime activeTime = channel->property("activeTimestamp").toDateTime();
        duration = duration.addSecs(activeTime.secsTo(QDateTime::currentDateTime()));
    }

    // and finally add the entry
    // just mark it as new if it is missed
    Q_EMIT callEnded(phoneNumber, incoming, timestamp, duration, missed, missed);
}

int CallManager::activeCallsCount() const
{
    int count = 0;
    Q_FOREACH(const CallEntry *entry, mCallEntries) {
        if (entry->isActive() || entry->dialing()) {
            count++;
        }
    }

    return count;
}

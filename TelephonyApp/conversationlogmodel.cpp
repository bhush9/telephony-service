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

#include "conversationlogmodel.h"
#include <TelepathyLoggerQt4/Event>
#include <TelepathyLoggerQt4/TextEvent>
#include <TelepathyLoggerQt4/Entity>

QVariant ConversationLogEntry::data(int role) const
{
    switch (role) {
    case ConversationLogModel::Message:
        return message;
    default:
        return LogEntry::data(role);
    }
}

ConversationLogModel::ConversationLogModel(QContactManager *manager, QObject *parent) :
    AbstractLoggerModel(manager, parent)
{
    // set the role names
    QHash<int, QByteArray> roles = roleNames();
    roles[Message] = "message";
    setRoleNames(roles);

    fetchLog(Tpl::EventTypeMaskText);
}

void ConversationLogModel::onMessageReceived(const QString &number, const QString &message)
{
    updateLatestMessage(number, message, true);
}

void ConversationLogModel::onMessageSent(const QString &number, const QString &message)
{
    updateLatestMessage(number, message, false);
}

LogEntry *ConversationLogModel::createEntry(const Tpl::EventPtr &event)
{
    ConversationLogEntry *entry = new ConversationLogEntry();
    Tpl::TextEventPtr textEvent = event.dynamicCast<Tpl::TextEvent>();

    if (!textEvent) {
        qWarning() << "The event" << event << "is not a Tpl::TextEvent!";
    }

    entry->message = textEvent->message();
    return entry;
}

void ConversationLogModel::handleDates(const Tpl::EntityPtr &entity, const Tpl::QDateList &dates)
{
    if (!dates.count()) {
        return;
    }
    QDate newestDate = dates.first();

    // search for the newest available date
    Q_FOREACH(const QDate &date, dates) {
        if (date > newestDate) {
            newestDate = date;
        }
    }

    requestEventsForDates(entity, Tpl::QDateList() << newestDate);
}

void ConversationLogModel::handleEvents(const Tpl::EventPtrList &events)
{
    if (!events.count()) {
        return;
    }

    Tpl::EventPtr newestEvent = events.first();

    // search for the newest message
    Q_FOREACH(const Tpl::EventPtr &event, events) {
        if (event->timestamp() > newestEvent->timestamp()) {
            newestEvent = event;
        }
    }

    appendEvents(Tpl::EventPtrList() << newestEvent);
}

void ConversationLogModel::updateLatestMessage(const QString &number, const QString &message, bool incoming)
{
    int count = mLogEntries.count();
    for(int i = 0; i < mLogEntries.count(); ++i) {
        ConversationLogEntry *entry = dynamic_cast<ConversationLogEntry*>(mLogEntries[i]);
        if (!entry) {
            continue;
        }

        if (entry->phoneNumber == number) {
            entry->timestamp = QDateTime::currentDateTime();
            entry->message = message;
            entry->incoming = incoming;
            emit dataChanged(index(i,0), index(i,0));
            return;
        }
    }

    // if we reach this point, there is a new conversation, so create the item
    ConversationLogEntry *entry = new ConversationLogEntry();
    entry->timestamp = QDateTime::currentDateTime();
    entry->incoming = incoming;
    entry->message = message;
    entry->phoneNumber = number;

    QContact contact = contactForNumber(number);
    if (!contact.isEmpty()) {
        fillContactInfo(entry, contact);
    }

    appendEntry(entry);
}
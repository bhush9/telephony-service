/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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

#ifndef TELEPATHYHELPER_H
#define TELEPATHYHELPER_H

#include <QObject>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Contact>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ConnectionManager>
#include <TelepathyQt/Types>
#include "channelobserver.h"

#define CANONICAL_TELEPHONY_VOICEMAIL_IFACE "com.canonical.Telephony.Voicemail"
#define CANONICAL_TELEPHONY_SPEAKER_IFACE "com.canonical.Telephony.Speaker"

class TelepathyHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QStringList accountIds READ accountIds NOTIFY accountIdsChanged)

public:
    ~TelepathyHelper();

    static TelepathyHelper *instance();
    QList<Tp::AccountPtr> accounts() const;
    ChannelObserver *channelObserver() const;
    QDBusInterface *handlerInterface();

    bool connected() const;
    QStringList accountIds() const;

    void registerClient(Tp::AbstractClient *client, QString name);

    Tp::AccountPtr accountForConnection(const Tp::ConnectionPtr &connection) const;
    Tp::AccountPtr accountForId(const QString &accountId) const;

    bool isAccountConnected(const Tp::AccountPtr &account) const;

Q_SIGNALS:
    void channelObserverCreated(ChannelObserver *observer);
    void channelObserverUnregistered();
    void accountReady();
    void connectionChanged();
    void connectedChanged();
    void accountIdsChanged();

public Q_SLOTS:
    Q_INVOKABLE void registerChannelObserver(const QString &observerName = QString::null);
    Q_INVOKABLE void unregisterChannelObserver();

protected:
    QStringList supportedProtocols() const;
    void initializeAccount(const Tp::AccountPtr &account);
    void ensureAccountEnabled(const Tp::AccountPtr &account);
    void ensureAccountConnected(const Tp::AccountPtr &account);
    void watchSelfContactPresence(const Tp::AccountPtr &account);

private Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void updateConnectedStatus();

private:
    explicit TelepathyHelper(QObject *parent = 0);
    Tp::AccountManagerPtr mAccountManager;
    Tp::Features mAccountManagerFeatures;
    Tp::Features mAccountFeatures;
    Tp::Features mContactFeatures;
    Tp::Features mConnectionFeatures;
    Tp::ClientRegistrarPtr mClientRegistrar;
    QList<Tp::AccountPtr> mAccounts;
    ChannelObserver *mChannelObserver;
    bool mFirstTime;
    bool mConnected;
    QDBusInterface *mHandlerInterface;
};

#endif // TELEPATHYHELPER_H

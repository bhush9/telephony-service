/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include "contactutils.h"
#include "telepathytest.h"
#include "messagingmenu.h"
#include "messagingmenumock.h"
#include "telepathyhelper.h"
#include "mockcontroller.h"

class MessagingMenuTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void testCallNotificationAdded();
    void testCallNotificationRemoved();
    void testTextMessagesNotificationAdded();
    void testTextMessagesNotificationFromOwnNumber();
private:
    Tp::AccountPtr mOfonoAccount;
    Tp::AccountPtr mMultimediaAccount;
    MockController *mOfonoMockController;
    MockController *mMultimediaMockController;
};

void MessagingMenuTest::initTestCase()
{
    initialize();

    // just trigger the creation of the mock singleton
    MessagingMenuMock::instance();

    // use the memory contact backend
    ContactUtils::sharedManager("memory");

    QSignalSpy accountAddedSpy(TelepathyHelper::instance(), SIGNAL(accountAdded(AccountEntry*)));
    mOfonoAccount = addAccount("mock", "ofono", "theOfonoAccount");
    QTRY_COMPARE(accountAddedSpy.count(), 1);
    accountAddedSpy.clear();
    mOfonoMockController = new MockController("ofono", this);

    mMultimediaAccount = addAccount("mock", "multimedia", "theMultimediaAccount");
    QTRY_COMPARE(accountAddedSpy.count(), 1);

    mMultimediaMockController = new MockController("multimedia", this);
}

void MessagingMenuTest::cleanupTestCase()
{
    doCleanup();
}

void MessagingMenuTest::cleanup()
{
    // just to prevent the doCleanup() to run on every test
}

void MessagingMenuTest::testCallNotificationAdded()
{
    QString caller("12345");
    QSignalSpy messageAddedSpy(MessagingMenuMock::instance(), SIGNAL(messageAdded(QString,QString,QString,bool)));
    MessagingMenu::instance()->addCall(caller, mOfonoAccount->uniqueIdentifier(), QDateTime::currentDateTime());
    QTRY_COMPARE(messageAddedSpy.count(), 1);
    QCOMPARE(messageAddedSpy.first()[1].toString(), caller);
}

void MessagingMenuTest::testCallNotificationRemoved()
{
    QString caller("2345678");
    QSignalSpy messageRemovedSpy(MessagingMenuMock::instance(), SIGNAL(messageRemoved(QString,QString)));
    MessagingMenu::instance()->addCall(caller, mOfonoAccount->uniqueIdentifier(), QDateTime::currentDateTime());
    MessagingMenu::instance()->removeCall(caller, mOfonoAccount->uniqueIdentifier());
    QCOMPARE(messageRemovedSpy.count(), 1);
    QCOMPARE(messageRemovedSpy.first()[1].toString(), caller);
}

void MessagingMenuTest::testTextMessagesNotificationAdded()
{
    QDBusInterface notificationsMock("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QSignalSpy notificationSpy(&notificationsMock, SIGNAL(MockNotificationReceived(QString, uint, QString, QString, QString, QStringList, QVariantMap, int)));

    QVariantMap properties;
    properties["Sender"] = "12345";
    properties["Recipients"] = (QStringList() << "12345");
    QStringList messages;
    messages << "Hi there" << "How are you" << "Always look on the bright side of life";
    Q_FOREACH(const QString &message, messages) {
        mMultimediaMockController->PlaceIncomingMessage(message, properties);
    }

    Q_FOREACH(const QString &message, messages) {
        mOfonoMockController->PlaceIncomingMessage(message, properties);
    }

    TRY_COMPARE(notificationSpy.count(), 6);
}

void MessagingMenuTest::testTextMessagesNotificationFromOwnNumber()
{
    QDBusInterface notificationsMock("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QSignalSpy notificationSpy(&notificationsMock, SIGNAL(MockNotificationReceived(QString, uint, QString, QString, QString, QStringList, QVariantMap, int)));

    QVariantMap properties;
    properties["Sender"] = "11112222";
    properties["Recipients"] = (QStringList() << "11112222");
    QStringList messages;
    messages << "Hi there" << "How are you" << "Always look on the bright side of life";

    Q_FOREACH(const QString &message, messages) {
        mOfonoMockController->PlaceIncomingMessage(message, properties);
    }
    TRY_COMPARE(notificationSpy.count(), 3);

    notificationSpy.clear();

    Q_FOREACH(const QString &message, messages) {
        mMultimediaMockController->PlaceIncomingMessage(message, properties);
    }

    // we need to make sure no notifications were displayed, using timers is always a bad idea,
    // but in this case there is no other easy way to test it.
    QTest::qWait(2000);
    QCOMPARE(notificationSpy.count(), 0);
}


QTEST_MAIN(MessagingMenuTest)
#include "MessagingMenuTest.moc"

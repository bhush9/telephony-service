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

#ifndef CONVERSATIONPROXYMODEL_H
#define CONVERSATIONPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "messagelogmodel.h"

class ConversationGroup {
public:
    ConversationGroup() : displayedRow(-1), newItem(false) {}
    QMap<QString, int> eventCount;
    QDateTime latestTime;
    int displayedRow;
    bool newItem;
};

class ConversationProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(bool ascending
               READ ascending
               WRITE setAscending
               NOTIFY ascendingChanged)
    Q_PROPERTY(QObject *conversationModel
               READ conversationModel
               WRITE setConversationModel
               NOTIFY conversationModelChanged)
    Q_PROPERTY (QString searchString
                READ searchString
                WRITE setSearchString
                NOTIFY searchStringChanged)
    Q_PROPERTY(QString filterValue
               READ filterValue
               WRITE setFilterValue
               NOTIFY filterValueChanged)
    Q_PROPERTY(QString filterProperty
               READ filterProperty
               WRITE setFilterProperty
               NOTIFY filterPropertyChanged)
    Q_PROPERTY(bool grouped
               READ grouped
               WRITE setGrouped
               NOTIFY groupedChanged)
    Q_PROPERTY(bool showLatestFromGroup
               READ showLatestFromGroup
               WRITE setShowLatestFromGroup
               NOTIFY showLatestFromGroupChanged)

    Q_ENUMS(ModelRoles)

public:
    enum CustomRoles {
        EventsRole = ConversationFeedModel::CustomRole
    };

    explicit ConversationProxyModel(QObject *parent = 0);

    QString filterValue() const;
    void setFilterValue(const QString &value);

    QString filterProperty() const;
    void setFilterProperty(const QString &value);

    bool ascending() const;
    void setAscending(bool value);

    QObject *conversationModel() const;
    void setConversationModel(QObject *value);

    QString searchString() const;
    void setSearchString(QString value);

    bool grouped() const;
    void setGrouped(bool value);

    bool showLatestFromGroup() const;
    void setShowLatestFromGroup(bool value);

    void updateSorting();

    virtual QVariant data(const QModelIndex &index, int role) const;

Q_SIGNALS:
    void ascendingChanged();
    void conversationModelChanged();
    void searchStringChanged();
    void filterValueChanged();
    void filterPropertyChanged();
    void groupedChanged();
    void showLatestFromGroupChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    ConversationGroup groupForSourceIndex(const QModelIndex &sourceIndex) const;

private Q_SLOTS:
    void processGrouping();

private:
    bool mAscending;
    QString mSearchString;
    QString mFilterValue;
    QString mFilterProperty;
    bool mGrouped;
    bool mShowLatestFromGroup;

    QMap<QString, QMap<QString, ConversationGroup> > mGroupedEntries;
};

#endif // CONVERSATIONPROXYMODEL_H
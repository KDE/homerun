/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "tabmodel.h"

// KDE
#include <KConfigGroup>
#include <KDebug>
#include <KLocale>
#include <KStandardDirs>

// Qt
#include <QFile>

// Local
#include <sourcemodel.h>
#include <sourceregistry.h>

using namespace Homerun;

static const char *TAB_GROUP_PREFIX = "Tab";

static const char *GENERAL_GROUP = "General";
static const char *GENERAL_TABS_KEY = "tabs";

class Tab
{
public:
    KConfigGroup m_group;

    QString m_name;
    QString m_iconName;
    SourceModel *m_sourceModel;

    Tab()
    : m_sourceModel(0)
    {
    }

    ~Tab()
    {
        delete m_sourceModel;
    }

    bool setName(const QString &value)
    {
        if (m_name == value) {
            return false;
        }
        m_name = value;
        saveName();
        m_group.sync();
        return true;
    }

    bool setIconName(const QString &value)
    {
        if (m_iconName == value) {
            return false;
        }
        m_iconName = value;
        saveIconName();
        m_group.sync();
        return true;
    }

    void saveName()
    {
        m_group.writeEntry("name", m_name);
    }

    void saveIconName()
    {
        m_group.writeEntry("icon", m_iconName);
    }

    void save()
    {
        // In case a deleted group is reused
        m_group.writeEntry("deleted", false);
        saveName();
        saveIconName();
        m_group.sync();
    }

    static Tab *createFromGroup(const KConfigGroup &group, TabModel *tabModel)
    {
        Tab *tab = new Tab;

        // (read "name" as QByteArray because i18n() wants a char* as argument)
        QByteArray name = group.readEntry("name", QByteArray());
        if (!name.isEmpty()) {
            // Only translate if not empty. If name is empty i18n(name)
            // returns I18N_EMPTY_MESSAGE.
            // Translation context must be the same as in internal/i18nconfig.cpp
            tab->m_name = i18nc("Tab title", name);
        }

        tab->m_group = group;
        tab->m_sourceModel = new SourceModel(tabModel->sourceRegistry(), group, tabModel);
        tab->m_iconName = group.readEntry("icon");
        return tab;
    }

    void remove()
    {
        m_group.deleteGroup();
        m_group.writeEntry("deleted", true);
        m_group.sync();
    }
};


TabModel::TabModel(QObject *parent)
: QAbstractListModel(parent)
, m_sourceRegistry(0)
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");
    roles.insert(SourceModelRole, "sourceModel");

    setRoleNames(roles);
}

TabModel::~TabModel()
{
    qDeleteAll(m_tabList);
}

QStringList TabModel::tabGroupList() const
{
    KConfigGroup group (m_config, GENERAL_GROUP);
    return group.readEntry(GENERAL_TABS_KEY, QStringList());
}

void TabModel::setConfig(const KSharedConfig::Ptr &ptr)
{
    beginResetModel();
    m_config = ptr;
    qDeleteAll(m_tabList);
    m_tabList.clear();
    QStringList list = tabGroupList();
    Q_FOREACH(const QString &groupName, list) {
        KConfigGroup group = m_config->group(groupName);
        Tab *tab = Tab::createFromGroup(group, this);
        if (tab) {
            m_tabList << tab;
        }
    }
    endResetModel();
    configFileNameChanged(m_config->name());
}

static void copyGroup(const KConfigGroup &dst_, const KConfigGroup &src)
{
    KConfigGroup dst = dst_;
    QMap<QString, QString> entryMap = src.entryMap();
    QSet<QString> keys = dst.keyList().toSet() | src.keyList().toSet();
    Q_FOREACH(const QString &key, keys) {
        auto it = entryMap.find(key);
        if (it == entryMap.end()) {
            dst.deleteEntry(key);
        } else {
            dst.writeEntry(key, src.readEntry(key));
        }
    }

    QStringList srcGroupList = src.groupList();
    Q_FOREACH(const QString &groupName, dst.groupList()) {
        if (!srcGroupList.contains(groupName)) {
            dst.deleteGroup(groupName);
        }
    }
    Q_FOREACH(const QString &groupName, srcGroupList) {
        copyGroup(dst.group(groupName), src.group(groupName));
    }
}

void TabModel::resetConfig()
{
    KConfigGroup generalGroup = m_config->group(GENERAL_GROUP);
    generalGroup.revertToDefault(GENERAL_TABS_KEY);

    QStringList tabs = generalGroup.readEntry(GENERAL_TABS_KEY, QStringList());

    // FIXME: Should load all config files in cascade
    QStringList fileNames = KGlobal::dirs()->findAllResources("config", m_config->name());
    KConfig systemConfig(fileNames.last());

    Q_FOREACH(const QString &groupName, m_config->groupList()) {
        if (!groupName.startsWith(TAB_GROUP_PREFIX)) {
            continue;
        }
        if (tabs.contains(groupName)) {
            copyGroup(m_config->group(groupName), systemConfig.group(groupName));
        } else {
            m_config->deleteGroup(groupName);
        }
    }
    m_config->sync();

    setConfig(m_config);
}

QString TabModel::configFileName() const
{
    return m_config ? m_config->name() : QString();
}

void TabModel::setConfigFileName(const QString &name)
{
    if (name == configFileName()) {
        return;
    }
    setConfig(KSharedConfig::openConfig(name));
}

AbstractSourceRegistry *TabModel::sourceRegistry() const
{
    return m_sourceRegistry;
}

void TabModel::setSourceRegistry(AbstractSourceRegistry *registry)
{
    if (m_sourceRegistry != registry) {
        m_sourceRegistry = registry;
        sourceRegistryChanged();
    }
}

int TabModel::rowCount(const QModelIndex &parent) const
{
    if (m_config.isNull()) {
        return 0;
    }
    if (parent.isValid()) {
        return 0;
    }
    return m_tabList.count();
}

QVariant TabModel::data(const QModelIndex &index, int role) const
{
    Tab *tab = m_tabList.value(index.row());
    if (!tab) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
        return tab->m_name;
    case Qt::DecorationRole:
        return tab->m_iconName;
    case SourceModelRole:
        return qVariantFromValue(static_cast<QObject *>(tab->m_sourceModel));
    default:
        kWarning() << "Unhandled role" << role;
        return QVariant();
    }
}

void TabModel::setDataForRow(int row, const QByteArray &roleName, const QVariant &value)
{
    Tab *tab = m_tabList.value(row);
    if (!tab) {
        kWarning() << "Invalid row number" << row;
        return;
    }

    if (roleName == "display") {
        if (!tab->setName(value.toString())) {
            return;
        }
    } else if (roleName == "decoration") {
        if (!tab->setIconName(value.toString())) {
            return;
        }
    } else {
        kWarning() << "Don't know how to handle role" << roleName;
        return;
    }

    QModelIndex idx = index(row, 0);
    dataChanged(idx, idx);
}

void TabModel::appendRow()
{
    QStringList list = tabGroupList();
    int lastId;
    if (list.isEmpty()) {
        lastId = -1;
    } else {
        bool ok;
        lastId = list.last().mid(3).toInt(&ok);
        if (!ok) {
            kWarning() << "Cannot extract a valid lastId from" << list.last();
            return;
        }
    }

    KConfigGroup tabGroup = m_config->group(QLatin1String(TAB_GROUP_PREFIX) + QString::number(lastId + 1));
    Tab *tab = Tab::createFromGroup(tabGroup, this);

    int count = m_tabList.count();
    beginInsertRows(QModelIndex(), count, count);
    m_tabList.append(tab);
    endInsertRows();

    tab->save();
    writeGeneralTabsEntry();
}

#define CHECK_ROW(row) \
    if (row < 0 || row >= m_tabList.count()) { \
        kWarning() << "Invalid row number" << row; \
        return; \
    }

void TabModel::removeRow(int row)
{
    CHECK_ROW(row)
    beginRemoveRows(QModelIndex(), row, row);
    Tab *tab = m_tabList.takeAt(row);
    Q_ASSERT(tab);
    tab->remove();
    delete tab;
    writeGeneralTabsEntry();
    endRemoveRows();
}

void TabModel::moveRow(int from, int to)
{
    CHECK_ROW(from)
    CHECK_ROW(to)
    if (from == to) {
        kWarning() << "Cannot move row to itself";
        return;
    }
    // See beginMoveRows() doc for an explanation on modelTo
    int modelTo = to + (to > from ? 1 : 0);
    beginMoveRows(QModelIndex(), from, from, QModelIndex(), modelTo);
    m_tabList.move(from, to);
    writeGeneralTabsEntry();
    endMoveRows();
}

void TabModel::writeGeneralTabsEntry()
{
    QStringList lst;
    Q_FOREACH(const Tab *tab, m_tabList) {
        lst << tab->m_group.name();
    }
    KConfigGroup group(m_config, GENERAL_GROUP);
    group.writeEntry(GENERAL_TABS_KEY, lst);
    m_config->sync();
}

#include "tabmodel.moc"

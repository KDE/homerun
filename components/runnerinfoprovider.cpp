/*
 * Copyright 2012 Aurélien Gâteau <agateau@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
// Self
#include <runnerinfoprovider.h>

// Local

// KDE
#include <KDebug>
#include <Plasma/RunnerManager>

// Qt

RunnerInfoProvider::RunnerInfoProvider(QObject *parent)
: QObject(parent)
{
}

RunnerInfoProvider::~RunnerInfoProvider()
{
}

QString RunnerInfoProvider::runnerId() const
{
    return m_runnerId;
}

void RunnerInfoProvider::setRunnerId(const QString &runnerId)
{
    if (runnerId == m_runnerId) {
        return;
    }
    m_info = KPluginInfo();
    KPluginInfo::List lst = Plasma::RunnerManager::listRunnerInfo();
    Q_FOREACH(const KPluginInfo &info, lst) {
        if (info.pluginName() == runnerId) {
            m_info = info;
            break;
        }
    }
    runnerIdChanged();
    nameChanged();
    iconChanged();
}

QString RunnerInfoProvider::name() const
{
    return m_info.isValid() ?  m_info.name() : QString();
}

QString RunnerInfoProvider::icon() const
{
    return m_info.isValid() ?  m_info.icon() : QString();
}

#include <runnerinfoprovider.moc>

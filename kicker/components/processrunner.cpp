/*
 *   Copyright 2013 Eike Hein <hein@kde.org>
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "processrunner.h"

#include <KProcess>

ProcessRunner::ProcessRunner(QObject *parent) : QObject(parent)
, m_triggerAction(0)
{
}

ProcessRunner::~ProcessRunner()
{
}

QString ProcessRunner::processName() const
{
    return m_processName;
}

void ProcessRunner::setProcessName(const QString& name)
{
    if (name != m_processName) {
        m_processName = name;
    }
}

QAction *ProcessRunner::triggerAction() const
{
    return m_triggerAction;
}

void ProcessRunner::setTriggerAction(QAction *action)
{
    if (action != m_triggerAction)
    {
        if (m_triggerAction) {
            disconnect(m_triggerAction, SIGNAL(triggered()), this, SLOT(execute()));
        }

        m_triggerAction = action;

        connect(m_triggerAction, SIGNAL(triggered()), this, SLOT(execute()));
    }
}

void ProcessRunner::execute(const QString& name)
{
    if (!name.isEmpty()) {
        KProcess::execute(name);
    } else if (!m_processName.isEmpty()) {
        KProcess::execute(m_processName);
    }
}

#include "processrunner.moc"

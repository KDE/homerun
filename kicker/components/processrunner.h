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

#ifndef PROCESSRUNNER_H
#define PROCESSRUNNER_H

#include <QAction>

class ProcessRunner : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString processName READ processName WRITE setProcessName)
    Q_PROPERTY(QAction* triggerAction READ triggerAction WRITE setTriggerAction)

    public:
        ProcessRunner(QObject *parent = 0);
        ~ProcessRunner();

        QString processName() const;
        void setProcessName(const QString &name);

        QAction *triggerAction() const;
        void setTriggerAction(QAction *action);

        Q_INVOKABLE void execute(const QString &name = QString());

    private:
        QString m_processName;
        QAction *m_triggerAction;
};

#endif

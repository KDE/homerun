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

#include "toolboxtoggle.h"

#include <QGraphicsWidget>

ToolBoxToggle::ToolBoxToggle(QObject *parent) : QObject(parent)
, m_toolBox(0)
, m_visible(true)
{
}

ToolBoxToggle::~ToolBoxToggle()
{
}

QObject* ToolBoxToggle::toolBox() const
{
    return m_toolBox;
}

void ToolBoxToggle::setToolBox(QObject* toolBox)
{
    if (m_toolBox != toolBox) {
        m_toolBox = static_cast<QGraphicsWidget *>(toolBox);
        emit toolBoxChanged();

        setVisible(!m_visible);
    }
}

bool ToolBoxToggle::visible() const
{
    return m_visible;
}

void ToolBoxToggle::setVisible(bool visible)
{
    if (!m_toolBox) {
        return;
    }

    if (m_visible != visible) {
        QGraphicsWidget *containment = static_cast<QGraphicsWidget *>(m_toolBox->parentItem());

        if (!containment) {
            return;
        }

        m_visible = visible;

        QGraphicsWidget *childWidget = 0;

        foreach(QGraphicsItem *childItem, containment->childItems()) {
            childWidget = static_cast<QGraphicsWidget *>(childItem);

            if (childWidget->inherits("InternalToolBox")) {
                childWidget->setVisible(m_visible);
            }
        }

        emit visibleChanged();
    }
}

#include "toolboxtoggle.moc"

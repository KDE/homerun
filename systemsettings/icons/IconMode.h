/***************************************************************************
 *   Copyright (C) 2009 by Ben Cooksley <bcooksley@kde.org>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#ifndef ICONMODE_H
#define ICONMODE_H

#include "BaseMode.h"

class ModuleView;
class KAboutData;
class QModelIndex;
class QAbstractItemView;

class IconMode : public BaseMode
{
    Q_OBJECT

public:
    IconMode(QObject * parent, const QVariantList& );
    ~IconMode();
    QWidget * mainWidget();
    void initEvent();
    void giveFocus();
    void leaveModuleView();
    KAboutData * aboutData();
    ModuleView * moduleView() const;

protected:
    QList<QAbstractItemView*> views() const;

public Q_SLOTS:
    void searchChanged( const QString& text );

private Q_SLOTS:
    void changeModule( const QModelIndex& activeModule );
    void moduleLoaded();
    void backToOverview();
    void initWidget();

private:
    class Private;
    Private *const d;
};

#endif

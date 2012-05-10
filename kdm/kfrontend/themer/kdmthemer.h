/*
 *  Copyright (C) 2003 by Unai Garro <ugarro@users.sourceforge.net>
 *  Copyright (C) 2004 by Enrico Ros <rosenric@dei.unipd.it>
 *  Copyright (C) 2004 by Stephan Kulow <coolo@kde.org>
 *  Copyright (C) 2004 by Oswald Buddenhagen <ossi@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KDMTHEMER_H
#define KDMTHEMER_H

#include <QMap>
#include <QObject>

class KdmItem;

class QDomNode;
class QPainter;
class QRect;

/**
* @author Unai Garro
*/



/*
* The themer widget. Whatever drawn here is just themed
* according to a XML file set by the user.
*/


class KdmThemer : public QObject {
    Q_OBJECT

public:
    /*
     * Construct and destruct the interface
     */

    KdmThemer(const QString &path,
              const QMap<QString, bool> &types, QWidget *w);
    ~KdmThemer();

    bool isOK() { return rootItem != 0; }

    const QString &baseDir() const { return basedir; }

    KdmItem *findNode(const QString &) const;

    // must be called by parent widget
    void widgetEvent(QEvent *e);

    void setWidget(QWidget *w);
    QWidget *widget() { return m_widget; }

    void setTypeVisible(const QString &t, bool show);
    bool typeVisible(const QString &t) { return m_showTypes.value(t, false); }

    void paintBackground(QPainter *p, const QRect &rect, bool primaryScreen);

public Q_SLOTS:
    void slotNeedPlacement();

Q_SIGNALS:
    void activated(const QString &id);

private:
    QMap<QString, bool> m_showTypes;

    // defines the directory the theme is in
    QString basedir;

    /*
     * Stores the root of the theme
     */
    KdmItem *rootItem;

    bool m_geometryOutdated;
    bool m_geometryInvalid;

    QWidget *m_widget;

    // methods

    /*
     * Parses the XML file looking for the
     * item list and adds those to the themer
     */
    void generateItems(KdmItem *parent, const QDomNode &node);
    void generateLayouts(KdmItem *parent, const QDomNode &node);

    void showStructure();

private Q_SLOTS:
    void update(int x, int y, int w, int h);
    void slotNeedPlugging();

};


#endif

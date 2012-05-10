/*
    Copyright 2008 Aaron Seigo <aseigo@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef BRANDINGBUTTON_H
#define BRANDINGBUTTON_H

#include <QtGui/QToolButton>

namespace Plasma
{
class Svg;
} // namespace Plasma

namespace Kickoff
{

class BrandingButton : public QToolButton
{
    Q_OBJECT

public:
    BrandingButton(QWidget *parent = 0);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);

protected Q_SLOTS:
    void checkBranding();
    void openHomepage();

private:
    Plasma::Svg *m_svg;
    QSize m_size;
    bool m_doingBranding;
};


} // namespace KickOff

#endif


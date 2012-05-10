/*
 *  Copyright (C) 2003 by Unai Garro <ugarro@users.sourceforge.net>
 *  Copyright (C) 2004 by Enrico Ros <rosenric@dei.unipd.it>
 *  Copyright (C) 2004 by Stephan Kulow <coolo@kde.org>
 *  Copyright (C) 2004,2006 by Oswald Buddenhagen <ossi@kde.org>
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

#include "parse.h"

#include <kdm_greet.h>

#include <KGlobalSettings>
#include <KStandardDirs>

#include <QComboBox>
#include <QDomElement>
#include <QFrame>
#include <QLineEdit>
#include <QStack>
#include <QStyleFactory>
#include <QWidget>

void
parseSize(const QString &s, DataPoint &pt)
{
    if (s.isEmpty())
        return;

    int p;
    if (s == "box") { // box value
        pt.type = DTbox;
        pt.val = 0;
    } else if (s == "scale") {
        pt.type = DTscale;
        pt.val = 0;
    } else if ((p = s.indexOf('%')) >= 0) { // percent value
        pt.type = DTpercent;
        QString sCopy = s;
        sCopy.remove(p, 1);
        pt.levels = 0;
        while ((p = sCopy.indexOf('^')) >= 0) {
            sCopy.remove(p, 1);
            pt.levels++;
        }
        sCopy.replace(',', '.');
        pt.val = (int)sCopy.toDouble();
    } else { // int value
        pt.type = DTpixel;
        QString sCopy = s;
        if (sCopy.at(0) == '-') {
            sCopy.remove(0, 1);
            pt.type = DTnpixel;
        }
        sCopy.replace(',', '.');
        pt.val = (int)sCopy.toDouble();
    }
}


static QString
getword(QString &rs)
{
    int splitAt = rs.lastIndexOf(' ') + 1;
    QString s(rs.mid(splitAt));
    rs.truncate(splitAt - 1);
    return s;
}

void
parseFont(const QString &is, FontType &ft)
{
    if (is.isNull())
        return;
    QString rs(is.simplified());
    if ((ft.present = !rs.isEmpty())) {
        QString s(getword(rs));
        bool ok;
        if (s.endsWith("px")) {
            int ps = s.left(s.length() - 2).toInt(&ok);
            if (ok) {
                ft.font.setPixelSize(ps);
                s = getword(rs);
            }
        } else {
            double ps = s.toDouble(&ok);
            if (ok) {
                ft.font.setPointSizeF(ps);
                s = getword(rs);
            }
        }
        forever {
            QString ss(s.toLower());
            if (ss == "oblique")
                ft.font.setStyle(QFont::StyleOblique);
            else if (ss == "italic")
                ft.font.setStyle(QFont::StyleItalic);
            else if (ss == "ultra-light")
                ft.font.setWeight(13);
            else if (ss == "light")
                ft.font.setWeight(QFont::Light);
            else if (ss == "medium")
                ft.font.setWeight(50);
            else if (ss == "semi-bold")
                ft.font.setWeight(QFont::DemiBold);
            else if (ss == "bold")
                ft.font.setWeight(QFont::Bold);
            else if (ss == "ultra-bold")
                ft.font.setWeight(QFont::Black);
            else if (ss == "heavy")
                ft.font.setWeight(99);
            else if (ss == "ultra-condensed")
                ft.font.setStretch(QFont::UltraCondensed);
            else if (ss == "extra-condensed")
                ft.font.setStretch(QFont::ExtraCondensed);
            else if (ss == "condensed")
                ft.font.setStretch(QFont::Condensed);
            else if (ss == "semi-condensed")
                ft.font.setStretch(QFont::SemiCondensed);
            else if (ss == "semi-expanded")
                ft.font.setStretch(QFont::SemiExpanded);
            else if (ss == "expanded")
                ft.font.setStretch(QFont::Expanded);
            else if (ss == "extra-expanded")
                ft.font.setStretch(QFont::ExtraExpanded);
            else if (ss == "ultra-expanded")
                ft.font.setStretch(QFont::UltraExpanded);
            else if (ss == "normal" || // no-op
                     ss == "small-caps" || // this and following ignored
                     ss == "not-rotated" || ss == "south" || ss == "upside-down" ||
                     ss == "north" ||
                     ss == "rotated-left" || ss == "east" ||
                     ss == "rotated-right" || ss == "west")
                ;
            else
                break;
            s = getword(rs);
        }
        if (!rs.isEmpty())
            rs.append(' ').append(s);
        else
            rs = s;
        QStringList ffs = rs.split(QRegExp(" ?, ?"), QString::SkipEmptyParts);
        if (!ffs.isEmpty()) {
            foreach (const QString& ff, ffs) {
                ft.font.setFamily(ff);
                if (ft.font.exactMatch())
                    return;
            }
            ft.font.setFamily(ffs.first());
        }
    }
    // don't inherit settings from parent widgets
    ft.font.setFamily(ft.font.family());
    if (ft.font.pixelSize() > 0)
        ft.font.setPixelSize(ft.font.pixelSize());
    else
        ft.font.setPointSize(ft.font.pointSize());
    ft.font.setStyle(ft.font.style());
    ft.font.setStretch(ft.font.stretch());
    ft.font.setWeight(ft.font.weight());
}

void
parseFont(const QDomElement &el, FontType &ft)
{
    parseFont(el.attribute("font", QString()), ft);
}


bool
parseColor(const QString &s, const QString &a, QColor &color)
{
    bool ok = false;
    if (s.startsWith('#')) {
        uint hexColor = s.mid(1).toUInt(&ok, 16);
        if (ok) {
            if (s.length() == 9)
                color.setRgba(hexColor);
            else
                color.setRgb(hexColor);
        } else {
            return false;
        }
    } else if (!s.isNull() && s.isEmpty()) {
        color = QColor();
        ok = true;
    }
    if (!a.isEmpty()) {
        float fltAlpha = a.toFloat(&ok);
        if (ok)
            color.setAlphaF(fltAlpha);
    }
    return ok;
}

void
parseColor(const QDomElement &el, QColor &color)
{
    parseColor(el.attribute("color"), el.attribute("alpha"), color);
}


static void
parsePalEnt(const QDomElement &el, const QString &pfx, const QString &core,
            QPalette &pal, QPalette::ColorGroup cg, QPalette::ColorRole cr)
{
    QColor col = pal.color(cg, cr);
    if (parseColor(el.attribute(pfx + core + "-color"),
                   el.attribute(pfx + core + "-alpha"), col))
        pal.setColor(cg, cr, col);
}

static void
parsePalEnt(const QDomElement &el, const QString &core,
            QPalette &pal, QPalette::ColorRole cr)
{
    parsePalEnt(el, "all-", core, pal, QPalette::Active, cr);
    parsePalEnt(el, "all-", core, pal, QPalette::Inactive, cr);
    parsePalEnt(el, "all-", core, pal, QPalette::Disabled, cr);
    parsePalEnt(el, QString(), core, pal, QPalette::Active, cr);
    parsePalEnt(el, QString(), core, pal, QPalette::Inactive, cr);
    parsePalEnt(el, "active-", core, pal, QPalette::Active, cr);
    parsePalEnt(el, "inactive-", core, pal, QPalette::Inactive, cr);
    parsePalEnt(el, "disabled-", core, pal, QPalette::Disabled, cr);
}

void
parseStyle(const QDomElement &el, StyleType &style)
{
    parseFont(el, style.font);
    parseFont(el.attribute("edit-font", QString()), style.editfont);
    QString colorscheme = el.attribute("colorscheme", QString());
    if (!colorscheme.isNull()) {
        colorscheme = KStandardDirs::locate("data", "color-schemes/" + colorscheme + ".colors");
        if (!colorscheme.isEmpty()) {
            KSharedConfigPtr config = KSharedConfig::openConfig(colorscheme, KConfig::SimpleConfig);
            style.palette = KGlobalSettings::createApplicationPalette(config);
        }
    }
    parsePalEnt(el, "window", style.palette, QPalette::Window);
    parsePalEnt(el, "window-text", style.palette, QPalette::WindowText);
    parsePalEnt(el, "base", style.palette, QPalette::Base);
    parsePalEnt(el, "alternate-base", style.palette, QPalette::AlternateBase);
    parsePalEnt(el, "text", style.palette, QPalette::Text);
    parsePalEnt(el, "highlight", style.palette, QPalette::Highlight);
    parsePalEnt(el, "highlighted-text", style.palette, QPalette::HighlightedText);
    parsePalEnt(el, "button", style.palette, QPalette::Button);
    parsePalEnt(el, "button-text", style.palette, QPalette::ButtonText);
    parsePalEnt(el, "bright-text", style.palette, QPalette::BrightText);
    QString frame = el.attribute("frame", QString());
    if (!frame.isNull())
        style.frame = frame == "true";
    QString guistyle = el.attribute("guistyle", QString());
    if (!guistyle.isNull())
        style.guistyle = QStyleFactory::create(guistyle);
}

void
setWidgetAttribs(QWidget *widget, const StyleType &style, bool frame)
{
    widget->setFont(
        (style.editfont.present &&
         (qobject_cast<QLineEdit *>(widget) ||
          qobject_cast<QComboBox *>(widget) ||
          widget->objectName() == "edit")) ?
            style.editfont.font : style.font.font);
    
    if (!frame) {
        if (QFrame *frm = qobject_cast<QFrame *>(widget)) {
            if ((widget->windowFlags() & Qt::WindowType_Mask) == Qt::Widget)
                frm->setFrameStyle(QFrame::NoFrame);
        } else if (QLineEdit *le = qobject_cast<QLineEdit *>(widget)) {
            le->setFrame(false);
        } else if (QComboBox *cb = qobject_cast<QComboBox *>(widget)) {
            cb->setFrame(false);
        }
    }

    foreach (QObject *child, widget->children())
        if (QWidget *cw = qobject_cast<QWidget *>(child))
            setWidgetAttribs(cw, style, frame ||
                (widget->testAttribute(Qt::WA_OpaquePaintEvent) ||
                 (widget->autoFillBackground() &&
                  !widget->testAttribute(Qt::WA_NoSystemBackground) &&
                  widget->palette().brush(widget->backgroundRole()).isOpaque())));
}

static QString prefix;
static QStack<QString> prefixes;

#define dbgs ((debugLevel & DEBUG_THEMING) ? QDebug(QtDebugMsg) : kDebugDevNull())

QDebug
enter(const char *fct)
{
    prefixes.push(prefix);
    prefix.replace('-', ' ').append(" |-");
    if (prefixes.top().isEmpty())
        return dbgs << fct;
    return dbgs << (qPrintable(prefixes.top()) + 1) << fct;
}

QDebug
debug()
{
    if (prefix.isEmpty())
        return dbgs;
    return dbgs << (qPrintable(prefix) + 1);
}

QDebug
leave()
{
    prefix[prefix.length() - 2] = '\\';
    QString nprefix(prefix);
    prefix = prefixes.pop();
    return dbgs << (qPrintable(nprefix) + 1);
}

/* Plastik KWin window decoration
  Copyright (C) 2003 Sandro Giessl <ceebx@users.sourceforge.net>

  based on the window decoration "Web":
  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
 */

#include <QButtonGroup>
#include <QCheckBox>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QWhatsThis>

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>

#include "config.h"

PlastikConfig::PlastikConfig(KConfig* config, QWidget* parent)
    : QObject(parent), m_config(0), m_dialog(0)
{
    Q_UNUSED( config );
    // create the configuration object
    m_config = new KConfig("kwinplastikrc");
    KConfigGroup cg(m_config, "General");
    KGlobal::locale()->insertCatalog("kwin_clients");

    // create and show the configuration dialog
    m_dialog = new PlastikConfigDialog(parent);
    m_dialog->show();

    // load the configuration
    load(cg);

    // setup the connections
    connect(m_dialog->titleAlign, SIGNAL(clicked(int)),
            this, SIGNAL(changed()));
    connect(m_dialog->animateButtons, SIGNAL(toggled(bool)),
            this, SIGNAL(changed()));
    connect(m_dialog->menuClose, SIGNAL(toggled(bool)),
            this, SIGNAL(changed()));
    connect(m_dialog->titleShadow, SIGNAL(toggled(bool)),
            this, SIGNAL(changed()));
    connect(m_dialog->coloredBorder, SIGNAL(toggled(bool)),
            this, SIGNAL(changed()));
}

PlastikConfig::~PlastikConfig()
{
    delete m_dialog;
    delete m_config;
}

void PlastikConfig::load(const KConfigGroup&)
{
    KConfigGroup cg(m_config, "General");


    QString value = cg.readEntry("TitleAlignment", "AlignLeft");
    QRadioButton *button = m_dialog->titleAlign->findChild<QRadioButton*>(value.toLatin1());
    if (button) button->setChecked(true);
    bool animateButtons = cg.readEntry("AnimateButtons", true);
    m_dialog->animateButtons->setChecked(animateButtons);
    bool menuClose = cg.readEntry("CloseOnMenuDoubleClick", true);
    m_dialog->menuClose->setChecked(menuClose);
    bool titleShadow = cg.readEntry("TitleShadow", true);
    m_dialog->titleShadow->setChecked(titleShadow);
    bool coloredBorder = cg.readEntry("ColoredBorder", true);
    m_dialog->coloredBorder->setChecked(coloredBorder);
}

void PlastikConfig::save(KConfigGroup&)
{
    KConfigGroup cg(m_config, "General");

    QList<QRadioButton *> buttons = m_dialog->titleAlign->findChildren<QRadioButton *>();
    for(QList<QRadioButton *>::ConstIterator it = buttons.constBegin(); it != buttons.constEnd(); ++it)
    {
        if((*it)->isChecked())
            cg.writeEntry("TitleAlignment", QString((*it)->objectName()));
    }

    cg.writeEntry("AnimateButtons", m_dialog->animateButtons->isChecked() );
    cg.writeEntry("CloseOnMenuDoubleClick", m_dialog->menuClose->isChecked() );
    cg.writeEntry("TitleShadow", m_dialog->titleShadow->isChecked() );
    cg.writeEntry("ColoredBorder", m_dialog->coloredBorder->isChecked() );
    m_config->sync();
}

void PlastikConfig::defaults()
{
    QRadioButton *button = m_dialog->titleAlign->findChild<QRadioButton*>("AlignLeft");
    if (button) button->setChecked(true);
    m_dialog->animateButtons->setChecked(true);
    m_dialog->menuClose->setChecked(false);
    m_dialog->titleShadow->setChecked(true);
    m_dialog->coloredBorder->setChecked(true);
}

//////////////////////////////////////////////////////////////////////////////
// Plugin Stuff                                                             //
//////////////////////////////////////////////////////////////////////////////

extern "C"
{
    KDE_EXPORT QObject* allocate_config(KConfig* config, QWidget* parent) {
        return (new PlastikConfig(config, parent));
    }
}

#include "config.moc"

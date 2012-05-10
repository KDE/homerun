/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Christian Nitschkowski <christian.nitschkowski@kdemail.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "diminactive_config.h"

#include <kwineffects.h>

#include <klocale.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <kaction.h>
#include <KShortcutsEditor>

#include <QWidget>
#include <QVBoxLayout>

namespace KWin
{

KWIN_EFFECT_CONFIG_FACTORY

DimInactiveEffectConfigForm::DimInactiveEffectConfigForm(QWidget* parent) : QWidget(parent)
{
    setupUi(this);
}

DimInactiveEffectConfig::DimInactiveEffectConfig(QWidget* parent, const QVariantList& args) :
    KCModule(EffectFactory::componentData(), parent, args)
{
    m_ui = new DimInactiveEffectConfigForm(this);

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(m_ui);

    connect(m_ui->spinStrength, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(m_ui->checkPanel, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(m_ui->checkDesktop, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(m_ui->checkKeepAbove, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(m_ui->checkGroup, SIGNAL(toggled(bool)), this, SLOT(changed()));

    load();
}

void DimInactiveEffectConfig::load()
{
    KCModule::load();

    KConfigGroup conf = EffectsHandler::effectConfig("DimInactive");

    int strength = conf.readEntry("Strength", 25);
    bool panel = conf.readEntry("DimPanels", false);
    bool desktop = conf.readEntry("DimDesktop", false);
    bool keepabove = conf.readEntry("DimKeepAbove", false);
    bool group = conf.readEntry("DimByGroup", true);
    m_ui->spinStrength->setValue(strength);
    m_ui->checkPanel->setChecked(panel);
    m_ui->checkDesktop->setChecked(desktop);
    m_ui->checkKeepAbove->setChecked(keepabove);
    m_ui->checkGroup->setChecked(group);

    emit changed(false);
}

void DimInactiveEffectConfig::save()
{
    KConfigGroup conf = EffectsHandler::effectConfig("DimInactive");

    conf.writeEntry("Strength", m_ui->spinStrength->value());
    conf.writeEntry("DimPanels", m_ui->checkPanel->isChecked());
    conf.writeEntry("DimDesktop", m_ui->checkDesktop->isChecked());
    conf.writeEntry("DimKeepAbove", m_ui->checkKeepAbove->isChecked());
    conf.writeEntry("DimByGroup", m_ui->checkGroup->isChecked());

    conf.sync();

    KCModule::save();
    emit changed(false);
    EffectsHandler::sendReloadMessage("diminactive");
}

void DimInactiveEffectConfig::defaults()
{
    m_ui->spinStrength->setValue(25);
    m_ui->checkPanel->setChecked(false);
    m_ui->checkDesktop->setChecked(false);
    m_ui->checkKeepAbove->setChecked(false);
    m_ui->checkGroup->setChecked(true);
    emit changed(true);
}


} // namespace

#include "diminactive_config.moc"

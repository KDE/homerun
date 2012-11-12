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
#include <dirconfigurationwidget.h>

// Local
#include <ui_dirconfigurationwidget.h>

// KDE
#include <KGlobalSettings>
#include <KUrlRequester>

// Qt
#include <QDir>
#include <QVBoxLayout>

using namespace Homerun;

DirConfigurationWidget::DirConfigurationWidget(const KConfigGroup &group)
: SourceConfigurationWidget(group)
, m_ui(new Ui_DirConfigurationWidget)
{
    m_ui->setupUi(this);

    QFont helpFont = KGlobalSettings::smallestReadableFont();
    m_ui->titleHelpLabel->setFont(helpFont);

    KUrl url = group.readPathEntry("rootUrl", QDir::homePath());
    if (url.isValid()) {
        m_ui->urlRequester->setUrl(url);
    }

    m_ui->titleLineEdit->setText(group.readEntry("rootName", QString()));
}

DirConfigurationWidget::~DirConfigurationWidget()
{
    delete m_ui;
}

void DirConfigurationWidget::save()
{
    KUrl url = m_ui->urlRequester->url().url();
    QString title = m_ui->titleLineEdit->text();

    configGroup().writePathEntry("rootUrl", url.url());
    configGroup().writeEntry("rootName", title);
}

#include <dirconfigurationwidget.moc>

/*
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
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

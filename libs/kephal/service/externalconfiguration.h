/*
 *   Copyright 2008 Aike J Sommer <dev@aikesommer.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef KEPHAL_EXTERNALCONFIGURATION_H
#define KEPHAL_EXTERNALCONFIGURATION_H

#include "backendconfigurations.h"


namespace Kephal {

    /**
     * Configuration which is hardwired to name 'external'.
     * Is this a hardwired permanent configuration object? If so where is 'single'?
     */
    class ExternalConfiguration : public BackendConfiguration {
        Q_OBJECT
        public:
            ExternalConfiguration(BackendConfigurations * parent);

            /**
             * @reimp Kephal::Configuration
             */
            QString name() const;
            /**
             * @reimp Kephal::Configuration
             */
            bool isModifiable() const;
            /**
             * @reimp Kephal::Configuration
             */
            bool isActivated() const;
            /**
             * @reimp Kephal::Configuration
             */
            QMap<int, QPoint> layout() const;
            /**
             * @reimp Kephal::Configuration
             */
            int primaryScreen() const;
        public Q_SLOTS:
            void activate();
        Q_SIGNALS:
            void activateExternal();
        private:
            BackendConfigurations * m_parent;
    };

}

#endif


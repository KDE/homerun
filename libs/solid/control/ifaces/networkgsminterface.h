/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#ifndef SOLID_IFACES_GSMNETWORKINTERFACE_H
#define SOLID_IFACES_GSMNETWORKINTERFACE_H

#include "../solid_control_export.h"

#include <QtCore/QStringList>

#include "../networkgsminterface.h"
#include "networkserialinterface.h"


namespace Solid
{
namespace Control
{
namespace Ifaces
{
    /**
     * Represents a wireless network interface
     */
    class SOLIDCONTROLIFACES_EXPORT GsmNetworkInterface : virtual public SerialNetworkInterface
    {
    public:
        /**
         * Destroys a GsmNetworkInterface object
         */
        virtual ~GsmNetworkInterface();

        virtual Solid::Control::ModemGsmCardInterface * getModemCardIface() = 0;

        virtual void setModemCardIface(Solid::Control::ModemGsmCardInterface * iface) = 0;

        virtual Solid::Control::ModemGsmNetworkInterface * getModemNetworkIface() = 0;

        virtual void setModemNetworkIface(Solid::Control::ModemGsmNetworkInterface * iface) = 0;

    protected:
    //Q_SIGNALS:
        /**
         * This signal is emitted when the network name of this network changes
         *
         * @param networkName the new network name
         */
        void networkNameChanged(const QString & networkName);
    };
} //Ifaces
} //Control
} //Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::GsmNetworkInterface, "org.kde.Solid.Control.Ifaces.GsmNetworkInterface/0.1")

#endif //SOLID_IFACES_GSMNETWORKINTERFACE_H


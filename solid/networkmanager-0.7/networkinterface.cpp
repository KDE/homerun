/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy 
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "networkinterface.h"
#include "networkinterface_p.h"

#include <arpa/inet.h>

#include <KDebug>

#include "dbus/nm-ip4-configinterface.h"
#include "manager.h"
#include "networkmanagerdefinitions.h"

// lifted from libnm-glib/nm-device.h, remove when our NM packages have this version
#define NM_DEVICE_UDI "udi"
#define NM_DEVICE_INTERFACE "interface"
#define NM_DEVICE_DRIVER "driver"
#define NM_DEVICE_CAPABILITIES "capabilities"
#define NM_DEVICE_MANAGED "managed"
#define NM_DEVICE_IP4_CONFIG "ip4-config"
#define NM_DEVICE_STATE "state"
#define NM_DEVICE_VENDOR "vendor"
#define NM_DEVICE_PRODUCT "product"

NMNetworkInterfacePrivate::NMNetworkInterfacePrivate( const QString & path, QObject * owner ) : deviceIface(NMNetworkManager::DBUS_SERVICE, path, QDBusConnection::systemBus()), uni(path), designSpeed(0), manager(0)/*, propHelper(owner)*/
{
    Q_UNUSED(owner);
    //isLinkUp = deviceIface.isLinkUp();
    driver = deviceIface.driver();
    interfaceName = deviceIface.interface();
    ipV4Address = deviceIface.ip4Address();    
    managed = deviceIface.managed();
    udi = deviceIface.udi();
    firmwareMissing = deviceIface.firmwareMissing();

    //TODO set active connections based on active connection list on the manager; find out if
    //signal needed
    //activeConnection = deviceIface.activeConnection();
    //propHelper.registerProperty(NM_DEVICE_UDI, PropertySignalPair("uni",0));
}

NMNetworkInterfacePrivate::~NMNetworkInterfacePrivate()
{

}

NMNetworkInterface::NMNetworkInterface(const QString & path, NMNetworkManager * manager, QObject * parent) : QObject(parent), d_ptr(new NMNetworkInterfacePrivate(path, this))
{
    Q_D(NMNetworkInterface);
    init();
    d->manager = manager;
}

NMNetworkInterface::NMNetworkInterface(NMNetworkInterfacePrivate & dd, NMNetworkManager * manager, QObject * parent) : QObject(parent), d_ptr(&dd)
{
    qDBusRegisterMetaType<UIntList>();
    qDBusRegisterMetaType<UIntListList>();
    Q_D(NMNetworkInterface);
    init();
    d->manager = manager;
}

void NMNetworkInterface::init()
{
    Q_D(NMNetworkInterface);
    d->capabilities = convertCapabilities(d->deviceIface.capabilities());
    d->connectionState = convertState(d->deviceIface.state());

    connect(&d->deviceIface, SIGNAL(StateChanged(uint,uint,uint)), this, SLOT(stateChanged(uint,uint,uint)));
}

NMNetworkInterface::~NMNetworkInterface()
{
    delete d_ptr;
}

QString NMNetworkInterface::uni() const
{
    Q_D(const NMNetworkInterface);
    return d->uni;
}

void NMNetworkInterface::setUni(const QVariant & uni)
{
    Q_D(NMNetworkInterface);
    d->uni = uni.toString();
}

QString NMNetworkInterface::interfaceName() const
{
    Q_D(const NMNetworkInterface);
    return d->interfaceName;
}

void NMNetworkInterface::setInterfaceName(const QVariant & name)
{
    Q_D(NMNetworkInterface);
    d->interfaceName = name.toString();
}

QString NMNetworkInterface::ipInterfaceName() const
{
    Q_D(const NMNetworkInterface);
    return d->deviceIface.ipInterface();
}

QString NMNetworkInterface::driver() const
{
    Q_D(const NMNetworkInterface);
    return d->driver;
}

bool NMNetworkInterface::firmwareMissing() const
{
    Q_D(const NMNetworkInterface);
    return d->firmwareMissing;
}

void NMNetworkInterface::setDriver(const QVariant & driver)
{
    Q_D(NMNetworkInterface);
    d->driver = driver.toString();
}

QString NMNetworkInterface::udi() const
{
    Q_D(const NMNetworkInterface);
    return d->udi;
}

int NMNetworkInterface::ipV4Address() const
{
    Q_D(const NMNetworkInterface);
    return d->ipV4Address;
}

Solid::Control::IPv4Config NMNetworkInterface::ipV4Config() const
{
    Q_D(const NMNetworkInterface);
    if (d->connectionState != Solid::Control::NetworkInterface::Activated) {
        return Solid::Control::IPv4Config();
    } else {
        // ask the daemon for the details
        QDBusObjectPath ipV4ConfigPath = d->deviceIface.ip4Config();
        OrgFreedesktopNetworkManagerIP4ConfigInterface iface(NMNetworkManager::DBUS_SERVICE, ipV4ConfigPath.path(), QDBusConnection::systemBus());
        if (iface.isValid()) {
            //convert ipaddresses into object
            UIntListList addresses = iface.addresses();
            QList<Solid::Control::IPv4Address> addressObjects;
            foreach (UIntList addressList, addresses) {
                if ( addressList.count() == 3 ) {
                    Solid::Control::IPv4Address addr(htonl(addressList[0]), addressList[1], htonl(addressList[2]));
                    addressObjects.append(addr);
                }
            }
            //convert routes into objects
            UIntListList routes = iface.routes();
            QList<Solid::Control::IPv4Route> routeObjects;
            foreach (UIntList routeList, routes) {
                if ( routeList.count() == 4 ) {
                    Solid::Control::IPv4Route addr(routeList[0], routeList[1], routeList[2], routeList[3]);
                    routeObjects.append(addr);
                }
            }
            // nameservers' IP addresses are always in network byte order
            return Solid::Control::IPv4Config(addressObjects,
                iface.nameservers(), iface.domains(),
                routeObjects);
        } else {
            return Solid::Control::IPv4Config();
        }
    }
}

bool NMNetworkInterface::isActive() const
{
    Q_D(const NMNetworkInterface);
    return !(d->connectionState == Solid::Control::NetworkInterface::Unavailable
            || d->connectionState == Solid::Control::NetworkInterface::Disconnected
            || d->connectionState == Solid::Control::NetworkInterface::Failed );
}

bool NMNetworkInterface::managed() const
{
    Q_D(const NMNetworkInterface);
    return d->managed;
}

void NMNetworkInterface::disconnectInterface()
{
    Q_D(NMNetworkInterface);
    d->deviceIface.Disconnect();
}

void NMNetworkInterface::setManaged(const QVariant & driver)
{
    Q_D(NMNetworkInterface);
    d->driver = driver.toBool();
}

Solid::Control::NetworkInterface::ConnectionState NMNetworkInterface::connectionState() const
{
    Q_D(const NMNetworkInterface);
    return d->connectionState;
}

void NMNetworkInterface::setConnectionState(const QVariant & state)
{
    Q_D(NMNetworkInterface);
    d->connectionState = convertState(state.toUInt());
}

int NMNetworkInterface::designSpeed() const
{
    Q_D(const NMNetworkInterface);
    return d->designSpeed;
}
/*
bool NMNetworkInterface::isLinkUp() const
{
    Q_D(const NMNetworkInterface);
    return d->isLinkUp;
}
*/
Solid::Control::NetworkInterface::Capabilities NMNetworkInterface::capabilities() const
{
    Q_D(const NMNetworkInterface);
    return d->capabilities;
}

QVariant NMNetworkInterface::capabilitiesV() const
{
    Q_D(const NMNetworkInterface);
    return QVariant(d->capabilities);
}

void NMNetworkInterface::setCapabilitiesV(const QVariant & caps)
{
    Q_D(NMNetworkInterface);
    d->capabilities = convertCapabilities(caps.toUInt());
}

Solid::Control::NetworkInterface::Capabilities NMNetworkInterface::convertCapabilities(uint theirCaps)
{
    Solid::Control::NetworkInterface::Capabilities ourCaps
        = (Solid::Control::NetworkInterface::Capabilities) theirCaps;
    return ourCaps;
}

Solid::Control::NetworkInterface::ConnectionState NMNetworkInterface::convertState(uint theirState)
{
    Solid::Control::NetworkInterface::ConnectionState ourState = (Solid::Control::NetworkInterface::ConnectionState)theirState;
    return ourState;
}

void NMNetworkInterface::stateChanged(uint new_state, uint old_state, uint reason)
{
    Q_D(NMNetworkInterface);
    d->connectionState = convertState(new_state);
    emit connectionStateChanged(d->connectionState, convertState(old_state), reason);
}

#include "networkinterface.moc"


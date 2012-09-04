#!/usr/bin/python
# http://wiki.bluez.org/wiki/HOWTO/DiscoveringDevices
import dbus
import dbus.glib
import gobject

def rem_dev_found_signal(address, cls, rssi):
        print 'Signal: RemoteDeviceFound(%s, %s, %s)' % (address, cls, rssi)

def rem_dev_name_signal(address, name):
        print 'Signal: RemoteNameUpdated(%s, %s)' % (address, name)

bus = dbus.SystemBus();

bus.add_signal_receiver(rem_dev_found_signal, 'RemoteDeviceFound', 'org.bluez.Adapter', 'org.bluez', '/org/bluez/hci0')
bus.add_signal_receiver(rem_dev_name_signal, 'RemoteNameUpdated', 'org.bluez.Adapter', 'org.bluez', '/org/bluez/hci0')

obj = bus.get_object('org.bluez', '/org/bluez/hci0')
adapter = dbus.Interface(obj, 'org.bluez.Adapter')

adapter.DiscoverDevices()

gobject.threads_init()
dbus.glib.init_threads()
main_loop = gobject.MainLoop()
main_loop.run()


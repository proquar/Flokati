Flokati
=======

Flokati is a collaborative ad-hoc automation system consisting of a
(relatively) low-speed rf-part based on AVR-microcontrollers and Nordic's
NRF-ICs, an OpenWrt-based WiFi-Gateway and an Android-App as a frontend.

For convenience the Android-App and OpenWrt-package for ar71xx are provided
in binary too.


History / Motivation / Overview
===============================

The original idea was to build a system of battery-powered devices for off-
the-grid outdoor parties, it should be:
* absolutely easy to set up and to use
* fast and responsive
* used simultaneously by a few people (and be robust at that)
* relatively cheap to build

Overall it already works very well, it still has some shortcomings, but I am
working on those.


Flokati-Parts
-------------

*Devices*

	> Currently there are only some spotlights and a mirror ball motor that I
	built around the Atmega88 and NRF24L01+.
	Devices have a unique 64bit address and can have multiple controllable
	elements (e.g. brightness, speed).
	Devices use Nordic's proprietary 2.4GHz NRF-technology (which was chosen
	because it is really cheap, much cheaper than e.g. 802.15.4).

*Gateway*

	> There needs to be a bridge between the IP-world and the proprietary
	rf-world. Here it is an OpenWrt-based WiFi access point (a battery powered
	TP-Link MR11U in my case) that also acts as a DCHP-server and hosts the
	device database.

*Frontend*

	> At the moment there's only the Android-App, which discovers new devices,
	requests device metadata from the device database and allows low-level
	control of discovered devices.


Architecture
------------

On the IP-side Flokati makes heavy use of IP-multicast. The bridge and the
Android devices all join a multicast group, and changes on one Android device
are immediately visible on the others. All actions on either side (NRF and IP)
are as fast as possible broadcast to the other side.

Controllable devices broadcast notify-messages with their ID and current
settings regularly, which are received by the frontend. The frontend then looks
up metadata (like name and controllable sinks) for that device from a database
which is also distributed in a multicast group. (Though the database currently
runs on the OpenWrt gateway and is not yet distributed)

So it is also possible to have controllable devices on the IP-side, so there
are currently no implementations for this. I will also add a mechanism for
remote configuration of some devices in the near future.


Directories
===========

*/avr*

	> These are some small devices I have built, there's a spotlight, which is
	able to PWM-dim or strobe a led. A mirror ball motor with controllable
	speed and direction. And the serial<->NRF gateway that is attached to the
	OpenWrt router.
	I will add hardware schematics and board layouts later.

*/Flokati-android*

	> This is the Android App as an Eclipse project. It's tiny and without any
	additional libraries. 

*/json-db*

	> Device metadata currently used by the database server on the OpenWrt
	router. Temporary solution. Should be put into /etc/flokati/db.

*/linux/flokati*

	> This is the OpenWrt package, consisting of two server-executables (bridge
	and database) and an init-script. The bridge can indicate it's status with
	a LED, that can be configured in the init-script.

*/flokati/python*

	> A few python tools, that I used during development. They do the same as the
	C-servers for OpenWrt.
#!/bin/sh /etc/rc.common
 
START=99 
STOP=99

SER2NET_HOST="127.0.0.1"
SER2NET_PORT="9098"
SERIAL_DEVICE="/dev/ttyUSB0"
LED_FILE="/sys/class/leds/tp-link:green:3g/brightness"
DB_DIR="/etc/flokati/db"


start() {
	setserial -a /dev/ttyUSB0 divisor 48 spd_cust
	
	ser2net -P /tmp/run/ser2net.flokati.pid -C "$SER2NET_HOST,$SER2NET_PORT:raw:0:$SERIAL_DEVICE:38400,NONE,1STOPBIT,8DATABITS,-LOCAL"
	start-stop-daemon -S -p /tmp/run/flokati.bridge.pid -m -b -x flokati-bridge -- $SER2NET_HOST $SER2NET_PORT $LED_FILE
	start-stop-daemon -S -p /tmp/run/flokati.db.pid -m -b -x flokati-db -- $DB_DIR
}

boot() {
	route add -net 224.0.0.0 netmask 240.0.0.0 dev wlan0
	start
}

stop() {
	start-stop-daemon -K -s SIGINT -p /tmp/run/flokati.bridge.pid
	start-stop-daemon -K -p /tmp/run/ser2net.flokati.pid
	start-stop-daemon -K -s SIGINT -p /tmp/run/flokati.db.pid
}

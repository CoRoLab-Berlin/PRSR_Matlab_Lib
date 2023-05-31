# PRSR_Matlab_Lib

IMU only

# Umgang mit 'sudo nano ...'

Zum Speichern nach dem Bearbeiten `STRG+X --> Y --> Enter` drücken

# Setup

## Host

(1) Matlab (>= R2023a) installieren (Empfehlung: mit allen Packages).

(2) Matlab AddOns installieren:
- `MATLAB Support Package for Raspberry Pi Hardware`
- `Simulink Support Package for Raspberry Pi Hardware`

(3) Raspberry Pi einrichten und per Ethernet direkt an Host verbinden (siehe **Raspberry Pi** Setup).

(4) Projekt von GitLab laden, `setup.m` ausführen und `LIB/raspberry_pi_interface.slx` öffnen.

(5) In Ubuntu statische IPv4 einrichten (Host ggf. über WLAN mit dem Internet verbinden, falls möglich und benötigt):

`Settings` --> `Network` --> `Wired` (Zahnrad) --> `IPv4` --> auf `Manuel` setzen und Folgendes eintragen:

    Address: 192.168.1.2
    Netmask: 255.255.255.0

(6) In Simulink unter dem Reiter

`Hardware` --> `Hardware Settings in Simulink` --> `Target hardware resources` --> `Board Parameters` --> `Edit`

Folgendes eintragen:
    
    Device Address: 192.168.1.1
    Username: pi
    Password: pi

(WARNUNG: Nur Tastatur benutzen falls klicken nicht möglich aufgrund Matlab Bug)
(Unter `Build Options` findet man noch die Option `Run on Boot`)

(7) Build & Monitor sollte nun komplett funktionieren.

## Raspberry Pi

(1) Mit `Raspberry Pi Imager` Raspberry OS (Bullseye 32-bit with no desktop) installieren.

Download: https://www.raspberrypi.com/software/

--> Advanced Options (Zahnrad Symbol) Folgendes einstellen: 

    Hostname: pi
    Enable SSH
    Username: pi
    Password: pi
    Locale settings:    Time zone: Europe/Berlin
                        Keyboard layout: de

(2) Auto-Login einrichten:

    sudo systemctl edit getty@tty1.service

Folgende Zeilen hinzufügen:

    [Service]
    ExecStart=
    ExecStart=-/sbin/agetty --noissue --autologin myusername %I $TERM
    Type=idle

(3) Passwordless sudo einrichten:

    sudo nano /etc/sudoers

`@includedir /etc/sudoers.d` auskommentieren und Folgendes unten hinzufügen:

    pi ALL=(ALL:ALL) NOPASSWD:ALL

(4) i2cdetect installieren und Upgrade durchführen:

    sudo apt-get update; sudo apt-get install i2c-tools -y; sudo apt-get upgrade -y

(5) SPI & I2C in `Interface Options` aktivieren:

    sudo raspi-config

(7) I2C Baudrate erhöhen:

    sudo nano /boot/config.txt` 

Die Zeile `dtparam=i2c_baudrate=400000` hinzufügen, damit es wie folgt aussieht:

    # Uncomment some or all of these to enable the optional hardware interfaces
    dtparam=i2c_arm=on
    dtparam=i2c_baudrate=400000
    #dtparam=i2s=on
    dtparam=spi=on

(8) Static IP einrichten:

    sudo nano /etc/dhcpcd.conf

Und folgende Zeilen wie folgt abändern:

    #Example static IP configuration:
    interface eth0
    static ip_address=192.168.1.1/24
    #static ip6_address=fd51:42f8:caae:d92e::ff/64
    static routers=192.168.1.2
    #static domain_name_servers=192.168.0.1 8.8.8.8 fd51:42f8:caae:d92e::1

(9) `sudo reboot`


**Optional** --- WLAN manuell aktivieren:

(1) Softblock des WiFi's aufheben, falls beim Flashen des Images auf die SD nicht explizit aktiviert:
    
    sudo rfkill unblock wifi; sudo rfkill unblock all

(2) Mit `sudo nano /etc/network/interfaces` Folgendes hinzufügen:
    
    allow-hotplug wlan0
    iface wlan0 inet manual
    wpa-conf /etc/wpa_supplicant/wpa_supplicant.conf

(3) Mit `sudo nano /etc/wpa_supplicant/wpa_supplicant.conf` Folgendes hinzufügen:

    network={
	    ssid="WLAN Name"
	    psk="WLAN Password"
	    key_mgmt=WPA-PSK
    }

(4) `sudo reboot`


# Weiteres

Raspberry Pi Matlab Support Package Dokumentation:
https://de.mathworks.com/help/supportpkg/raspberrypi/setup-and-configuration.html

Workaround zwecks "Klicken in Matlab nicht möglich" - Fehler (nicht empfehlenswert):
https://wiki.ubuntuusers.de/Xfce_Installation/
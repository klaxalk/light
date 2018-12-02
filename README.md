Light - A program to control backlights (and other hardware lights) in GNU/Linux
==================================================

*Copyright (C) 2012 - 2018*

*Author: Fredrik Haikarainen*

*Contributor & Maintainer: Joachim Nilsson*

*This is free software, see the source for copying conditions.  There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE*


- [Introduction](#introduction)
- [Examples](#examples)
- [Usage](#usage)
  - [Command options](#command-options)
  - [Extra options](#extra-options)
- [Installation](#installation)
  - [Arch Linux](#arch-linux)
  - [Fedora](#fedora)
  - [Manual](#manual)
  - [Permissions](#permissions)
- [Origin & References](#origin--references)


Introduction
------------

[Light][] is a program to control backlights and other lights under GNU/Linux:

* Works where other software has proven unreliable (xbacklight etc.)
* Works even in a fully CLI-environment, i.e. it does not rely on X
* Provides functionality to automatically control backlights with the highest precision available
* Extra features, like setting a minimum brightness value for controllers, or saving/restoring the value for poweroffs/boots.

See the following sections for the detailed descriptions of all available commands, options and how to access different controllers.

Light is available in many GNU/Linux distributions already.


Examples
--------

Get the current backlight brightness in percent

    light -G

or

     light

Increase backlight brightness by 5 percent

    light -A 5

Set the minimum cap to 2 in raw value on the sysfs/backlight/acpi_video0 device:

    light -Nrs "sysfs/backlight/acpi_video0" 2

List available devices

    light -L

Activate the Num. Lock keyboard LED, here `sysfs/leds/input3::numlock` is used, but this varies
between different systems:

    light -Srs "sysfs/leds/input3::numlock" 1


Usage
-----

Usage follows the following pattern, where options are optional and the neccesity of value depends on the options used
    
    light [options] <value>

### Command options

You may only specify one command flag at a time. These flags decide what the program will ultimately end up doing.

*  `-H` Show help and exit
*  `-V` Show program version and exit
*  `-L` List available devices
*  `-A` Increase brightness by value (value needed!)
*  `-U` Decrease brightness by value (value needed!)
*  `-S` Set brightness to value (value needed!)
*  `-G` Get brightness
*  `-N` Set minimum brightness to value (value needed!)
*  `-P` Get minimum brightness
*  `-I` Save the current brightness
*  `-O` Restore the previously saved brightness

Without any extra options, the command will operate on the device called `sysfs/backlight/auto`, which works as it's own device however it proxies the backlight device that has the highest controller resolution (read: highest precision). Values are interpreted and printed as percentage between 0.0 - 100.0.

**Note:** If something goes wrong, you can find out by maxing out the verbosity flag by passing `-v 3` to the options. This will activate the logging of warnings, errors and notices. Light will never print these by default, as it is designed to primarily interface with other applications and not humanbeings directly.

### Extra options

These can be mixed, combined and matched after convenience. 

* `-r` Raw mode, values (printed and interpreted from commandline) will be treated as integers in the controllers native range, instead of in percent.
* `-v <verbosity>` Specifies the verbosity level. 0 is default and prints nothing. 1 prints only errors, 2 prints only errors and warnings, and 3 prints both errors, warnings and notices.
* `-s <devicepath>` Specifies which device to work on. List available devices with the -L command. Full path is needed.


Installation
------------

### Arch Linux

The latest stable release is available in official repos, install with:

    pacman -S light

Additionally, the latest development branch (master) is available on AUR: [light-git][]

### Fedora

Fedora already has light packaged in main repos, so just run:

    dnf install light

and you're good to go.

### Debian/Ubuntu

Pre-built .deb files, for the latest Ubuntu release, can be downloaded
from the [GitHub][Light] releases page.  If you want to build your own
there is native support available in the GIT sources.  Clone and follow
the development branch guidelines below followed by:

    make deb

### Manual

If you download a stable release, these are the commands that will get you up and running:

    tar xf light-x.yy.tar.gz
    cd light-x.yy/
    ./configure && make
    sudo make install

However the latest development branch requires some extras. Clone the repository and run the `autogen.sh` script.  This requires that `automake` and `autoconf` is installed on your system.

    ./autogen.sh
    ./configure && make
    sudo make install

The `configure` script and `Makefile.in` files are not part of GIT because they are generated at release time with `make release`.


### Permissions

Optionally, instead of the classic SUID root mode of operation, udev rules can be set up to manage the kernel sysfs permissions.  Use the configure script to enable this mode of operation:

    ./configure --with-udev && make
    sudo make install

This installs the `90-backlight.rules` into `/usr/lib/udev/rules.d/`.
If your udev rules are located elsewhere, use `--with-udev=PATH`.

**Note:** make sure that your user is part of the `video` group, otherwise you will not get access to the devices.

**Note:** in this mode `light` runs unpriviliged, so the `/etc/light`
directory (for cached settings) is not used, instead the per-user
specific `~/.cache/light` is used.


[Light]:     https://github.com/haikarainen/light/
[light-git]: https://aur.archlinux.org/packages/light-git

Light - A Program to Control Backlight Controllers
==================================================

- [Introduction](#introduction)
- [Examples](#examples)
- [Usage](#usage)
  - [Operation modes](#operation-modes)
  - [Value modes](#value-modes)
  - [Target](#target)
  - [Field](#field)
  - [Controller modes](#controller-modes)
- [Installation](#installation)
  - [Arch Linux](#arch-linux)
  - [Manual](#manual)
  - [Permissions](#permissions)
- [Origin & References](#origin--references)


Introduction
------------

[Light][] is a program to control backlight controllers under GNU/Linux:

* Works, in particular when other software, e.g. xbacklight, does not
* Does not rely on X
* Automatically detects the best controller
* Possibility to set a minimum brightness value

Let's get started with a few examples, for details, see below for the
full description of the different commands, options and how to access
different controllers.


Examples
--------

Get the current brightness in percent

    light -G

or

     light

Increase brightness by 5 percent

    light -A 5

Set the minimum cap to 2 in raw value on the acpi_video0 controller:

    light -cr -s acpi_video0 -S 2

Try to set the brightness to 0 after that, it will be changed to the
minimum 2:

    light -r -s acpi_video0 -S 0

Find keyboard controllers:

    light -k -L

Activate `ScrollLock` LED, here `input15` is used, but this varies
between different systems:

    light -k -s "input15::scrolllock" -S 100

Usually, LEDs only take 0 or 1 in raw value (i.e. for off/on), so you
can instead write:

    light -kr -s "input15::scrolllock" -S 1

Verify by reading back the max brightness, you should get a value of 1:

    light -kr -m -s "input15::scrolllock


Usage
-----

### Commands

* `-G`: Get (read) brightness/data from controllers/files
* `-S VAL`: Set (write)brightness/data to controllers/files
* `-A VAL`: Like `-S`, but adds the given value
* `-U VAL`: Like `-S`, but subtracts the given value
* `-O`: Save the current brightness for later use (usually used on shutdown)
* `-I`: Restore the previously saved brightness (usually used on boot)
* `-L`: List available controllers, see below `-k` option as well

Without any options (below) the commands operate on the brightness of an
automatically selected controller.  Values are given in percent, unless
the below `r` option is also given.

**Note:** like most UNIX applications, light only gives output on
  errors.  If something goes wrong try the verbosity option `-v VAL`:

* 0: No debug output
* 1: Errors
* 2: Errors, warnings
* 3: Errors, warnings, notices

### Options

Values may be given, or presented, in percent or raw mode.  Raw mode is
the format specific to the controller.  The default is in percent, but
raw mode may be required for precise control, or when the steps are very
few, e.g. for most keyboard backlight controllers.

* `-p`: Percent, default
* `-r`: Raw mode

By default the screen is the active target for all commands, use `-k` to
select the keyboard instead.  In either case you may need to select a
different controller, see below.

* `-l`: Act on screen backlight, default
* `-k`: Act on keyboard backlight and LEDs

By default commands act on the brightness property, which is read+write.
The maximum brightness is a read-only property.  The minimum brightness
cap is a feature implemented to protect against setting brightness too
low, since some controllers make the screen go pitch black at 0%.  For
controllers like that it is recommended to set this value.

* `-b`: Current brightness of selected controller, default
* `-m`: Max. brightness of selected controller
* `-c`: Min. brightness (cap) of selected controller (recommend raw mode)

Controller is automatically done to select the controller with maximum
precision.  It can however also be done manually and we recommend the
`-L` and `-Lk` commands to list available controllers:

* `-a`: Automatic controller selection
* `-s ARG`: Manual controller selection

**Note:** Without the `-s` flag on _every_ command light will default
  to automatic controller selection.


Installation
------------

### Arch Linux

If you run Arch Linux, there exists 2 packages;

* [light-git][] - For the absolutely latest version
* [light-tag][] - For the latest tagged release

We recommend you go with light-git as you might miss important features
and bugfixes if you do not.


### Manual

We recommended downloading a versioned tarball from the relases page on
GitHub.  Download and untar the archive:

    tar xf light-x.yy.tar.gz
    cd light-x.yy/
    ./configure && make
    sudo make install

However, should you want to try the latest GitHub source you first need
to clone the repository and run the `autogen.sh` script.  This requires
`automake` and `autoconf` to be installed on your system.

    ./autogen.sh
    ./configure && make
    sudo make install

The `configure` script and `Makefile.in` files are not part of GIT
because they are generated at release time with `make release`.


### Permissions

Optionally, instead of the classic SUID root mode of operation, udev
rules can be set up to manage the kernel sysfs permissions.  Use the
configure script to enable this mode of operation:

    ./configure --with-udev && make
    sudo make install

This installs the file `90-backlight.rules` into `/lib/udev/rules.d/`.
If your udev rules are located elsewhere, use `--with-udev=PATH`.



Origin & References
-------------------

Copyright (C) 2012-2018 Fredrik Haikarainen

This is free software, see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

[Light]:     https://github.com/haikarainen/light
[light-git]: https://aur.archlinux.org/packages/light-git
[light-tag]: https://aur.archlinux.org/packages/light

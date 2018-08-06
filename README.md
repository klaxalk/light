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
full description of the different operating modes, value conversion and
how to operate different controllers.


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

Light has 5 different criteria on flags to use: operation modes, value
mode, target, field and controller mode.  Flags from these different
modes can never be used in conjunction, i.e. you cannot get and set a
value at the same time, that requires different invocations.

**Note:** like most UNIX applications, light only print errors if you
  are using it incorrectly. If something goes wrong, and you can't
  figure out why, try setting the verbosity flag with -v:

* 0: No debug output
* 1: Errors
* 2: Errors, warnings
* 3: Errors, warnings, notices


### Operation modes

The operation modes describe **what** you want to do.

* -G: Which **reads/gets** brightness/data from controllers/files
* -S: Which **writes/sets** brightness/data to controllers/files
* -A: Which does like -S but instead **adds** the value
* -U: Which does like -S but instead '**subtracts** the value
* -O: Save the current brightness for later use (usually used on shutdown)
* -I: Restore the previously saved brightness (usually used on boot)
* -L: List the available controllers

When used by themselves operate on the brightness of a controller that
is selected automatically. S, A and U needs another argument -- except
for the main 4 criteria -- which is the value to set/add/subtract.  This
can be specified either in percent or in raw values, but remember to
specify the value mode (read below) if you want to write raw values.


### Value modes

The value mode specify in what unit you want to read or write values
in. The default one (if not specified) is in percent, the other one is
raw mode and should always be used when you need very precise values (or
only have a controller with a very small amount of brightness levels).

* -p: Percent
* -r: Raw mode

Remember, this is the unit that will be used when you set, get, add or
subtract brightness values.


### Target

You can choose which target to act on:

* -l: Act on screen backlight
* -k: Act on keyboard backlight and LEDs


### Field

As you can not only handle the **brightness** of controllers, you may
also specify a field to read/write from/to:

* -b: Current brightness of selected controller
* -m: Maximum brightness of selected controller
* -c: Minimum brightness (cap) of selected controller

The minimum brightness is a feature implemented as some controllers make
the screen go pitch black at 0%, if you have a controller like that, it
is recommended to set this value (in percent or raw mode).  These values
are saved in raw mode though, so if you specify it in percent it might
not be too accurate depending on your controller.


### Controller modes

Finally, you can either use the built-in controller selection to get the
controller with the maximum precision, or you can specify one manually
with the -s flag. The -a flag will force automatic mode and is
default. Use -L to get a list of controllers to use with the -s flag (to
specify which controller to use).

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

**Optional:** Instead of SUID root you can set up udev rules to manage
   sysfs permissions, you may skip the `make install` step and instead
   copy the file `90-backlight.rules` to `/etc/udev/rules.d/`:


    ACTION=="add", SUBSYSTEM=="backlight", RUN+="/bin/chgrp video /sys/class/backlight/%k/brightness"
    ACTION=="add", SUBSYSTEM=="backlight", RUN+="/bin/chmod g+w /sys/class/backlight/%k/brightness"


Origin & References
-------------------

Copyright (C) 2012-2018 Fredrik Haikarainen

This is free software, see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

[Light]:     https://github.com/haikarainen/light
[light-git]: https://aur.archlinux.org/packages/light-git
[light-tag]: https://aur.archlinux.org/packages/light

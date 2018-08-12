Change Log
==========

All relevant changes to the project are documented in this file.


[v1.2][UNRELEASED] - 2018-08-xx
-------------------------------

### Changes
- Convert to GNU configure & build system
- Add support for installing non-SUID root using an udev rule
  enabled `--with-udev` to the new configure script
- Migrate to use `~/.cache/light` instead of `/etc/light` for
  unpriviliged operation
- Add proper light.1 man page, remvoes help2man dependency
- Simplify presentation of commands and options in built-in help text,
  as well as in the README and man page
- Change to Linux coding style
- Add Fedora installation instructions


[v1.1.2][] - 2018-06-20
-----------------------

Panic release to fix save/restore.

### Changes
- Add help2man dependency in README
- Better Support for Overriding Install Prefix
- Restore DESTDIR support

### Fixes
- Issue #29: Fix save and restore arguments
- Issue #27: Use the install command instead of raw cp/mv/chmod.


[v1.1][] - 2017-11-23
---------------------

Various fixes and improvements.  Credits to Abdullah ibn Nadjo

### Changes
- Add `-k` flag for keyboard backlight support
- Cache max brightness data from automatic controller detection
- Improve overall logging
- Logging of clamps, saves and restores
- Support for save, restore, get [max] brightness etc. for both screen
  and keyboard controllers

### Fixes
- Avoid checking for write permission if just getting value
- Check if controller is accessible before getting value
- Avoid redondant checking
- Don't truncate file contents when checking if file is writable
- Fix `light_controllerAccessible()` and `light_getBrightness()` this
  functions were:
   - Reading values from the controller
   - Checking write permission even when we just want reading values
   - Checking the mincap file instead of the actual controller
- Don't try to read brightness values when only targetting max bright
- Fix issues with string buffers and pointers
  - Use `NAME_MAX` and `PATH_MAX` instead of hardcoded values
  - Allow paths to be longer than 256 chars
  - Check pointers everywhere
  - Use `strncpy()`/`snprintf()` instead of `strcpy()`/`sprintf()`
  - Validate controllers' name (`-s` flag + a very long name = bad
    things happening)
  - Get rid of globals for dir iteration


[v1.0][] - 2016-05-10
---------------------

First major release.  Light has been around for a while now and seems to
make some people happy.  Also someone wanted a new release, so here you
go!

### Changes
- Added save/restore functionality
- Generate man page on `make install`

### Fixes
- Issue #5: Can't increase brightness on ATI propietary driver
- Issue #10: Honor `$DESTDIR` on man page installation


[v0.9][] - 2014-06-08
---------------------

### Changes
- Complete rewrite of program (Every single byte)
- Cleaner, safer code
- Completely new intuitive usage (Sorry, it was needed)
- Added functionality:
  - Ability to set/get minimum brightness directly from commandline
  - Ability to specify the backlight controller to use directly from commandline
  - Better verbosity
- Probably missed some stuff


v0.7 - 2012-11-18
-----------------

### Changes
- Ported bash script to C


[UNRELEASED]: https://github.com/haikarainen/light/compare/v1.1.2...HEAD
[v1.1.2]:     https://github.com/haikarainen/light/compare/v1.1...v1.1.2
[v1.1]:       https://github.com/haikarainen/light/compare/v1.0...v1.1
[v1.0]:       https://github.com/haikarainen/light/compare/v0.9...v1.0
[v0.9]:       https://github.com/haikarainen/light/compare/v0.7...v0.9

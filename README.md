DESFire-Shell
=============

This tool allows you to modify DESFire NFC tags via a simple command line user
interface. The command line language is simply an interactive Lua shell.
DESFire operations are mapped to Lua function calls.


Installation
------------

### Prerequirements

#### Build Time

 * pkgconfig

#### Run Time

 * GNU Readline (https://tiswww.case.edu/php/chet/readline/rltop.html)
 * Lua 5.1 or newer (http://www.lua.org/)
 * OpenSSL 1.1.1 (https://www.openssl.org/)
 * libnfc (https://github.com/nfc-tools/libnfc)
 * libfreefare (https://github.com/nfc-tools/libfreefare)


### Build

```
$ make
$ cp desfsh /usr/local/bin
```

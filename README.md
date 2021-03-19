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


Usage
-----

### Show Devices and Tags

Simply calling `desfsh` list the available card reader devices. In the
following example all readers are virtually the same, just the driver to
access the reader differs.

```
3 devices found:
 0: acr122_usb:001:097
 1: acr122_pcsc:ACS ACR122U PICC Interface 01 00
  ** Unable to open device. **
 2: acr122_pcsc:ACS ACR122U PICC Interface 02 00
```

With a NFC tag present, each reader lists detected tags. Here two NFC tags are
present in the radio field of the reader.

```
3 devices found:
 0: acr122_usb:001:097
   0: Mifare DESFire --> 044f6bf2893180
   1: Mifare DESFire --> 04257a020c5180
 1: acr122_pcsc:ACS ACR122U PICC Interface 01 00
  ** Unable to open device. **
 2: acr122_pcsc:ACS ACR122U PICC Interface 02 00
   0: Mifare DESFire --> 044f6bf2893180
   1: Mifare DESFire --> 04257a020c5180
```

### Access a Tag

To access a distinct tag, you have to provide the reader and tag identifier as
provided by the above-mentioned output. You may either specify the index ...

```
./desfsh -D acr122_usb:001:097 -T 044f6bf2893180
```

... or the textual identifier ...

```
./desfsh -d 0 -t 0
```

... or a combination of both.

```
./desfsh -d 0 -T 044f6bf2893180
```

You will get an interactive Lua shell which lets you execute commands against
the tag.

```
> result, errmsg, aids = cmd.appids()
> print(result, errmsg)
0       OK
> for idx, aid in ipairs(aids) do
>> print(idx, aid)
>> end
1       0x000001
```

### Command Mode

You can specifiy a command via the `-c`-option. The program exits after the
command string has been executed.

In the following example the script `examples/clt21.lua` is executed.

**CAUTION:** This script will delete all applications irremediably on the card.
  It will try to authenticated with a zero DES or AES key respectively and
  change the PICC Master Key to the zero AES key. Afterwards a sample
  application with AID 1 and a couple of sample files is created.

```
./desfsh -d 0 -t 0 -c 'dofile("examples/clt21.lua")'
```

Using the `-i`-option, the program will enter the interactive mode after
executing the provided command string instead of exiting.

### Offline Mode

Using the `-o`-option enters offline mode. In offline mode no card reader is
required. The features of the program are limited to the functions which don't
depend on a connection to cards. This is mainly useful to perform cryptographic
calculations.

### Debug Mode

You can inspect all input and out parameters as well as status codes by setting
the appropriate debug flags. The debug level is a logical or combination of
serveral flags. The following flags are defined.

* `0x01` Show status codes of executed commands. Command names are printed in
  purple. Successful commands are printed in green, failed commands in red.
* `0x02` Show input parameters. Input parameters are printed in blue.
* `0x04` Show returned parameters. Returned parameters are printed in cyan.
* `0x08` Show additions debug information.

```
> debugset(15)
> cmd.appids()
>>> GetApplicationIDs <<<
    STAT <=> 0: OK
     AID <=  0x000001
> cmd.select(1)
>>> SelectApplication <<<
     AID  => 0x000001
    STAT <=> 0: OK
```

In the example all debug flags are set. Then all defined applications are
queried. The command executes successful (`STAT <=> 0: OK`). There is one
applications ID returned (`AID <=  0x000001`). In case multiple applications
would be available on the tag, further `AID`-lines would be printed out. Lastly
the application with ID 1 is selected. The AID 1 is transferred to the tag
(`AID  => 0x000001`). The command executes successfully, too.

### Help

The `help`-command shows an online help for all commands. Commands may have
aliases. For example `cmd.appids`, `cmd.aids` and `cmd.GetApplicationIDs` are
all the same command.

```
> help(cmd.appids)

Get Application List

code, err, [aids] = cmd.appids()

Aliasses: appids, aids, GetApplicationIDs

    code        Return Code
    err         Error String
    aids        List of AIDs

> help(cmd.select)

Select Application

code, err = cmd.selapp(aid)

Aliasses: selapp, select, SelectApplication

    aid         AID
    code        Return Code
    err         Error String
```

Without an command name, all available commands are listed.

```
> help()

buf.concat            Concatenate buffers
buf.fromascii         Read Buffer from ASCII String
buf.fromhexstr        Read Buffer from HEX String
buf.fromtable         Read Buffer from Table
buf.hexdump           Convert Buffer to HEX Dump
buf.toascii           Convert Buffer to ASCII String
buf.tohexstr          Convert Buffer to HEX String
buf.totable           Convert Buffer to Table
cmd.abort             Abort Transaction
cmd.appids            Get Application List
cmd.auth              Authenticate to PICC
cmd.carduid           Get Real Card UID
cmd.cbdf              Create Backup Data File
cmd.ccrf              Create Cyclic Record File
cmd.cfs               Change File Settings
cmd.ck                Change Key
cmd.cks               Change Master Key Configuration
cmd.clrf              Create Linear Record File
cmd.commit            Commit Transaction
cmd.createapp         Create Application
cmd.crec              Clear Record File
cmd.credit            Increase a Files Value
cmd.csdf              Create Standard Data File
cmd.cvf               Create Value File
cmd.debit             Decrease a Files Value
cmd.deleteapp         Delete Application
cmd.delf              Delete File
cmd.fileids           Get File List
cmd.format            Format PICC
cmd.freemem           Get Size of remaining Memory
cmd.getval            Get Value of File
cmd.getver            Get PICC Information
cmd.gfs               Get File Settings
cmd.gks               Get Master Key Configuration
cmd.gkv               Get Key Version
cmd.lcredit           Increase a Files Value by a Limited Amount
cmd.read              Read from File
cmd.rrec              Read from Record
cmd.selapp            Select Application
cmd.wrec              Write to Record
cmd.write             Write to File
crc.crc32             Calculate a CRC-32 checksum
crypto.cmac           Calculate CMAC
crypto.hmac           Calculate HMAC
debugset              Set Debug Flags
help                  Show Help Text
key.create            Create key object
key.diversify         Calculate diversified Key
show.apps             Show Application Information
show.files            Show Files of an Application
show.picc             Show PICC Information
```

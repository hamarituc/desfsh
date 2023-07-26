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
 * OpenSSL 3 (https://www.openssl.org/)
 * zlib (https://www.zlib.net/)
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
following example all readers are virtually the same, just the drivers to
access the reader differ.

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

The `help`-command shows an online help for all commands. Without an command
name, all available commands are listed.

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

When you specify a command, a brief description is printed out including the
input and output parameters of the command. Commands may have aliases. For
example `cmd.appids`, `cmd.aids` and `cmd.GetApplicationIDs` are all the same
command.

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


Commands
--------

Commands are grouped into namespaces.

| Namespace | Description                                         |
| --------- | --------------------------------------------------- |
| `cmd`     | Commands executed against DESFire tags              |
| `buf`     | Creation and manipulation of byte sequence buffers  |
| `key`     | Creation of DESFire key objects                     |
| `crc`     | Checksum functions                                  |
| `crypto`  | Cryptographic functions                             |
| `show`    | Compound functions for analyzing tags               |

### DESFire Commands

The `cmd`-namespace contains all commands which are supported by DESFire tags.
All commands receive an provide the same parameters as specified in the DESFire
standard. A detailed explantion of the DESFire standard is beyond the scope of
this document.

The remainder of this section shows a sample session. The tag used is based on
the example script `examples/clt21.lua`. You can prepare you own card by
executing this script inside the DESFire shell.

Activate debugging.

```
> debugset(255)
```

Show all present DESFire applications.

```
> cmd.appids()
>>> GetApplicationIDs <<<
    STAT <=> 0: OK
     AID <=  0x000001
```

Change current application to AID 1.

```
> cmd.select(1)
>>> SelectApplication <<<
     AID  => 0x000001
    STAT <=> 0: OK
```

Show all files of this application.

```
> cmd.fileids()
>>> GetFileIDs <<<
    STAT <=> 0: OK
     FID <=  0
     FID <=  1
     FID <=  2
     FID <=  3
     FID <=  4
```

There are five files defined numbered from 0 through 4.

Show settings of file 0.

```
> cmd.gfs(0) -- get file settings
>>> GetFileSettings <<<
     FID  => 0
    STAT <=> 0: OK
    COMM <=  0x03 (CRYPT)
     ACL <=  RD:01 WR:02 RW:03 CA:00
    FSET <=  [SDF]  SIZE: 32
```

It is a standard data file of 32 bytes length. File access operations are
encrypted. Key 1 is allowed to read that file, key 2 is allowed to write,
key 3 is allowd to read and write. Key 0 is allowed to change these access
settings.

Try to to read the file 0.

```
> cmd.read(0, 0, 32)
>>> ReadData <<<
     FID  => 0
     OFF  => 0
     LEN  => 32
         *I* Executing GetFileSettings() to determine file type and size.
    STAT <=> 174: AUTHENTICATION_ERROR
```

This operations fails as we are unauthenticated.

Authenticate with key 1.

```
> cmd.auth(1, AES("11111111111111111111111111111111"))
>>> Authenticate <<<
     KNO  => 1
     KEY  => AES:11111111111111111111111111111111 (V:000)
    STAT <=> 0: OK
```

Now try to read file 0 again.

```
> cmd.read(0, 0, 32)
>>> ReadData <<<
     FID  => 0
     OFF  => 0
     LEN  => 32
         *I* Executing GetFileSettings() to determine file type and size.
    STAT <=> 0: OK
     BUF <=  00000000  43 68 65 6d  6e 69 74 7a  |Chemnitz|
     BUF <=  00000008  65 72 00 00  00 00 00 00  |er......|
     BUF <=  00000010  00 00 00 00  00 00 00 00  |........|
     BUF <=  00000018  00 00 00 00  00 00 00 00  |........|
```

As we gained read permission, the command succeeds.

Writing to the file is disallowed in the current authentication status. To
write to the file, we authenticate using key number 3.

```
> cmd.write(0, 11, buf.fromascii("Linux-Tage"))
>>> WriteData <<<
     FID  => 0
     OFF  => 11
     BUF  => 0000000b  4c 69 6e 75  78 2d 54 61  |Linux-Ta|
     BUF  => 00000013  67 65                     |ge      |
    STAT <=> 174: AUTHENTICATION_ERROR
> cmd.auth(3, AES("33333333333333333333333333333333"))
>>> Authenticate <<<
     KNO  => 3
     KEY  => AES:33333333333333333333333333333333 (V:000)
    STAT <=> 0: OK
> cmd.write(0, 11, buf.fromascii("Linux-Tage"))
>>> WriteData <<<
     FID  => 0
     OFF  => 11
     BUF  => 0000000b  4c 69 6e 75  78 2d 54 61  |Linux-Ta|
     BUF  => 00000013  67 65                     |ge      |
    STAT <=> 0: OK
```

As we are authenticated using key number 3, reading the file is allowed, too.

```
> cmd.read(0, 0, 32)
>>> ReadData <<<
     FID  => 0
     OFF  => 0
     LEN  => 32
         *I* Executing GetFileSettings() to determine file type and size.
    STAT <=> 0: OK
     BUF <=  00000000  43 68 65 6d  6e 69 74 7a  |Chemnitz|
     BUF <=  00000008  65 72 00 4c  69 6e 75 78  |er.Linux|
     BUF <=  00000010  2d 54 61 67  65 00 00 00  |-Tage...|
     BUF <=  00000018  00 00 00 00  00 00 00 00  |........|
```

Finally we inspect the security settings of the currently selected application.

```
> cmd.gks()
>>> GetKeySettings <<<
    STAT <=> 0: OK
  KEYSET <=  0x0f = AKC:AMK CONF:M CA/F:* DA/F:M/* LIST:* MKC:M
 MAXKEYS <=  4
```

The application has four keys. The key settings are encoded in the key settings
byte `0x0f` and interpreted as following.

* `AKC:AMK`: Each application key (1 through 3) can be changed by the
  application master key (key number 0).
* `CONF:M`: The security settings can by changed by the application master key.
* `CA/F:*`: Files can be created unauthenticaed.
* `DA/F:M/*`: Files can be deleted unauthenticated. The application can be
  deleted by the applications master key.
* `LIST:*`: File listing is permitted without authentication.
* `MKC:M`: The application master key (key number 0) can be changed by itself.

For a detailed description of the security settings, refer to the DESFire
specification.

### Byte Sequence Buffers

Access to files is byte oriented. The file content is stored in buffer objects
which can be transformed to different representations:

* UTF-8 text strings
* Hex strings
* Lua tables

This expression creates a buffer object `x` from the text string `Hello
World!`.

```
> x = buf.fromascii("Hello World!")
```

This buffer can be converted into a hex string ...

```
> print(x:tohexstr())
48656c6c6f20576f726c6421
```

... or into a Lua table ...

```
> for i, v in ipairs(x:totable()) do
>>   print(i, v)
>> end
1       72
2       101
3       108
4       108
5       111
6       32
7       87
8       111
9       114
10      108
11      100
12      33
```

... or into a hex dump.

```
> print(x:hexdump())
00000000  48 65 6c 6c  6f 20 57 6f  |Hello Wo|
00000008  72 6c 64 21               |rld!    |
```

### Key Objetcs

Cryptographic key values are stored in lua tables. These key objects are
manipulated by functions of the `key`-namespace. Each key has a type, a key
value and a key version number. The following code creates the AES-key
`00112233445566778899aabbccddeeff` with key version 0.

```
k = key.create({ t = "AES", k = "00112233445566778899aabbccddeeff", v = 0 })
```

The shortcut function `AES()` simplifies this expression considerably.

```
k = AES("00112233445566778899aabbccddeeff", 0)
```

If you skip the key version number, it will default to 0.

```
k = AES("00112233445566778899aabbccddeeff")
```

If you skip the key value, the key value will be set to zero. This is the
default key.

```
k = AES()
```

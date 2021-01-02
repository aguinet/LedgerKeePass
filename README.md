# Keepass Ledger application

![Tests](https://github.com/aguinet/ledger-keepass/workflows/Tests/badge.svg)

This repository contains the code to an Ledger BOLOS application to derive
and/or store encryption keys for Keepass databases.

**WARNING: this application and all the code using it are still in development
(see TODO). Please do not use this in production yet. Databases could be lost
or corrupted, and/or secrets leaked. You have been warned.**

The userland part has been implemented in a [fork of
KeepassXC](https://github.com/aguinet/keepassxc/tree/feature/ledger). It uses
the provided ``libkpl`` library, to talk with the Ledger application.  The goal
is to have this merged in the official [KeepassXC](kpxc) application when the
Ledger application will be considered stable (see
[PR](https://github.com/keepassxreboot/keepassxc/pull/5842)).

This application supports Ledger Nano S and X devices. Blue isn't supported.

## Screenshots

The app in action, running in Speculos!

While opening a database:

![KeepassXC database open](imgs/kp_ledger_open.png)

Database creation:

![KeepassXC database create](imgs/kp_ledger_create.png)

## Build & install

### Ledger application

You first need to install the [BOLOS
SDK](https://ledger.readthedocs.io/en/latest/userspace/getting_started.html).
You especially need to setup the ``BOLOS_SDK`` environment variable to the
directory where you extracted the SDK, and get a working Clang cross compiler.

Once this is setup, just run ``make`` in the ``app`` directory. Application
binaries will be in the ``app/bin`` directory.

### KPL (KeePass Ledger) userland library

Dependencies:

* [cmake](cmake)
* [libsodium](sodium)
* [hidapi](https://github.com/signal11/hidapi)

[libsodium](sodium) has been choosen for the userland cryptographic operations,
because [KeepassXC](kpxc) already links with it (no extra dependency involved).

#### Linux

On Debian-based systems, you can install the dependencies like this:

```
$ sudo apt install libsodium-dev libhidapi-dev cmake
```

To build the library:

```
cd libkpl && make build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cmake -DCMAKE_INSTALL_PREFIX=/path/to/prefix -P cmake_install.cmake
```

This will install ``libkpl`` into the prefix of your choice. This prefix will
be useful to compile the [KeepassXC
fork](https://github.com/aguinet/keepassxc/tree/feature/ledger).

#### Windows

One way to do it is to use vcpkg to gather dependencies, and CMake to build the
project. It has been tested with Visual Studio 2019's compiler.

First, [install vcpkg](https://github.com/microsoft/vcpkg#quick-start-windows).
Then, in a Visual Studio 2019 x64 command prompt:

```
> \path\to\vcpkg\vcpkg install libsodium hidapi --triplet x64-windows
> cd \path\to\ledger-keepass\libkpl
> mkdir build
> cd build
> cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..
> cmake --build . --config release
```

### KeepassXC fork

Please refer to [KeepassXC build
instructions](https://github.com/keepassxreboot/keepassxc/blob/develop/INSTALL.md).
The only things that changes is when running ``cmake``: you need to enable the
Ledger plugin, and provide the path to ``libkpl`` cmake files:

```
$ cmake -DWITH_XC_LEDGER=ON -Dkpl_DIR=/path/to/kpl_prefix/lib/cmake [other parameters]
```

## Usage

The app is capable of provide keys to [KeepassXC](kpxc) in two ways:

* by deriving a 32 bytes key using the device's seed and a user provided string
  (e.g. `perso`)
* by giving user-provided 32 bytes keys stored on the device. The app is
  capable of storing 8 differents keys.

User interaction is needed before a key is sent to the KeepassXC application.

The KeepassXC GUI can't yet store user-provided keys. The ``set`` tool compiled
alongside the ``kpl`` library can do this.

## Run with the Speculos emulator

[Build and
install](https://github.com/LedgerHQ/speculos/blob/master/doc/build.md) the
[Speculos](speculos) emulator. You need to use [this
branch](https://github.com/aguinet/speculos/tree/feature/curve25519), waiting
for the [associated PR](https://github.com/LedgerHQ/speculos/pull/116) to be
review and merged.

Once the Ledger application is compiled, you can run it with Speculos:

```
/path/to/speculos.py app/bin/app.elf -k 1.6
```

You can the emulated application with [KeepassXC](kpxc) by setting these environment variables:

```
export LEDGER_PROXY_ADDRESS=127.0.0.1
export LEDGER_PROXY_PORT=9999
```

(By default, [Speculos](speculos) listens on TCP port 9999).

## Protocol

See [protocol.md](protocol.md).

## Tests

Tests needs Python 3 to run, with some packages:

```
pip install -r tests/requirements.txt
```

Tests will use the tools build with `libkpl` to test this library. You thus
need to specify a `libkpl` build directory to the tests, with the path to the
Speculos main script:

```
tests/run.sh /path/to/speculos.py /path/to/libkpl/build nanos
```

This will test the application with a pure Python implementation of ``libkpl``,
and then ``libkpl`` itself.

It also specifies that the application has been compiled for the ``nanos``
model. To test the ``nanox`` version, replace ``nanos`` by ``nanox`` in the
``run.sh`` command line.


## TODO

* KeepassXC GUI for key slots
* test on actual devices!


[speculos]: https://github.com/LedgerHQ/speculos/
[kpxc]: https://github.com/keepassxreboot/keepassxc/
[sodium]: https://github.com/jedisct1/libsodium

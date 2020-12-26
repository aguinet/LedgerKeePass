# Protocol

The protocol uses [APDU](adpu) packets. ``CLA`` is always ``0xE0``. This
section describes the various commands.

The applicaion supports a maximum of 8 slots.

## GET_APP_CONFIGURATION

Request:

* ``INS`` = 0
* ``P1`` = 0
* ``P2`` = 0
* No extra data

Answer bytes:

* 1 byte: version major
* 1 byte: version minor
* 1 byte: version patch

Expected SW answer:

* ``0x9000``: success

## STORE_KEY

Request:

* ``INS`` = 1
* ``P1`` = slot index
* ``P2`` = 0
* No extra data

Answer bytes: none

Expected SW answer:

* ``0x9000``: success
* ``0x6820``: user refused to erase already set slot

## GET_KEY

Request:

* ``INS`` = 2
* ``P1`` = slot index
* ``P2`` = 0
* Extra data:
  - 32 bytes of a public ephemeral x25519 key

Answer bytes (if success):

* 32 bytes: public x25519 ephemeral key of the app
* 32 bytes: resulting key xored with the exchanged ephemeral x25519 key

Expected SW answer:

* ``0x9000``: success
* ``0x6820``: user refused to provide key
* ``0x6802``: slot not set

## GET_KEY_FROM_NAME

Request:

* ``INS`` = 3
* ``P1`` = 0
* ``P2`` = 0
* Extra data:
  - 32 bytes of a public ephemeral x25519 key
  - user-provided name (20 bytes maximum)

Answer bytes (if success):

* 32 bytes: public x25519 ephemeral key of the app
* 32 bytes: resulting key xored with the exchanged ephemeral x25519 key

Expected SW answer:

* ``0x9000``: success
* ``0x6820``: user refused to provide key

## GET_VALID_KEY_SLOTS

Request:

* ``INS`` = 4
* ``P1`` = 0
* ``P2`` = 0

Answer bytes:

* 1 byte: bitfield representing valid slots

Expected SW answer:

* ``0x9000``: success

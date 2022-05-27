# TON Transaction Serialization

## Overview

TON Ledger App requires custom format for transactions since it have to be assembed in ledger for security.

## Amount units

TON uses 256 bit integers for amounts, but TON app limits it to 64bit uint. This limits amount that could be sent as 18M TON. This is substantinal and we don't expect cases where you would need to send much more in a single transaction.

## Wallet Contract

TON Ledger App works only with Wallet V4 contract and doesn't support legacy wallets. This also meants that it is impossible to work with older wallets.

## Address format

Address is serialized by appending a single byte of chain - 0x00 or 0xff and then 32 bytes of hash.

## Hints

NOTE: This is not released yet

Hints are arbitrary that could be used by Nano App

## Structure

### Transaction

| Field | Size (bytes) | Description |
| --- | :---: | --- |
| `tag` | 1 | 0x00. Reserved for future. |
| `seqno` | 4 | A sequence number used to prevent message replay |
| `timeout` | 4 | Message timeout |
| `value` | 8 | The amount in nanotons to send to the destination address |
| `bounce` | 1 | 0x01 or 0x00 for bounce flag |
| `send_mode` | 1 | send mode of a message |
| `state_init` | 1 | 0x01 if state init is present |
| (opt) `state_init_depth` | 2 | state_init' Cell depth if state_init is 0x01 |
| (opt) `state_init_hash` | 32 | state_init' Cell hash is 0x01 |
| `payload` | 1 | 0x01 if payload is present |
| (opt) `payload_depth` | 2 | payload' Cell depth if state_init is 0x01 |
| (opt) `payload_hash` | 32 | payload' Cell hash is 0x01 |
| `hints` | 1 | 0x01 if hints exists |
| (opt) `hints_len` | 16 | hints data length |
| (opt) `hints_data` | <var> | hints data |

### Fee

You won't find any fee in the transaction structure because the TON chain has constant fees.
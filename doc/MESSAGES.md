# List of supported messages and their descriptors

This list contains a number of messages that ledger could assemble and display critical information (hints) about what's this transaction is about.

| ID | Message | Description |
| --- | --- | --- |
| 0x00 | Message with Comment| Typical transaction with a comment|
| 0x01 | Upgrade | Universal message for upgrading code of a contract |
| 0x02 | Update | Universal message to update parameters of a contract |
| 0x03 | Deposit | Whales Nominators deposit command |
| 0x04 | Withdraw | Whales Nominators withdraw command |
| 0x05 | Create Proposal | Universal message for creating DAO proposal |
| 0x06 | Vote | Universal message for voting YES/NO/ABSTAIN for a proposal in DAO |
| 0x07 | Execute proposal | Universal message for executing proposal in DAO |
| 0x08 | Abort proposal | Universal message for aborting proposal in DAO |
| 0x09 | Transfer ownership | Universal message for transfering ownership of the contract |

# 0x00: Message with comment

This is just a usual transaction with a comment, but it have it's limitations:
* Only ASCII-printable symbols are supported
* 120 symbols max

### TL-B
```
message#00000000 text: string = MsgBody;
```

### Hints
| Value | Length | Description |
| --- | --- | --- |
| `message` | `var` | ASCII-only message |

# 0x01: Upgrade
Universal code upgrade message

### TL-B
```
upgrade#dbfaf817 query_id:uint64 gas_limit:Coins code: ^Cell = MsgBody;
```

### Hints
| Value | Length | Description |
| --- | --- | --- |
| `query_id` | 8 | query id |
| `gas_limit`| 8 | gas_limit |
| `code_ref` | 34 | Cell Reference of a Code hash to be shown to user in Base64 format |

# 0x023cd52c: Update
Universal contract update message

### TL-B
```
update#23cd52c state: ^Cell = MsgBody;
```

### Hints
| Value | Length | Description |
| --- | --- | --- |
| `code_ref` | 34 | Cell Reference of a Code hash to be shown to user in Base64 format |

# 0x7bcd1fef: Deposit
Deposit to a TON Whales staking pool

### TL-B
```
deposit#7bcd1fef = MsgBody;
```

### Hints
Emtpy buffer

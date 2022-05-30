# List of supported messages and their descriptors

This list contains a number of messages that ledger could assemble and display critical information (hints) about what's this transaction is about.

| ID | Message | Description |
| --- | --- | --- |
| 0x00000000 | Message with Comment| Typical transaction with a comment|
| 0x00000001 | Upgrade | Universal message for upgrading code of a contract |
| 0x00000002 | Update | Universal message to update parameters of a contract |
| 0x00000003 | Donate | Universal message donating to a contract |
| 0x00000004 | Deposit | Whales Nominators deposit command |
| 0x00000005 | Withdraw | Whales Nominators withdraw command |
| 0x00000006 | Create Proposal | Universal message for creating DAO proposal |
| 0x00000007 | Vote | Universal message for voting YES/NO/ABSTAIN for a proposal in DAO |
| 0x00000008 | Execute proposal | Universal message for executing proposal in DAO |
| 0x00000009 | Abort proposal | Universal message for aborting proposal in DAO |
| 0x0000000a | Transfer ownership | Universal message for transfering ownership of the contract |

# 0x00000000: Message with comment

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

# List of supported messages and their descriptors

This list contains a number of messages that ledger could assemble and display critical information (hints) about what's this transaction is about.

| ID | Message | Description |
| --- | --- | --- |
| 0x00000000 | Message with Comment| Typical transaction with a comment|
| 0x00000001 | Code Upgrade | Universal message for upgrading code of a contract |

# 0 - Message with comment

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

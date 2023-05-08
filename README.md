# TON Ledger App

This is a Nano App for Ledger S/X/S+ for TON blockchain.

* 🚀 Compatible with latest Wallet v4 contract
* 🔎 Absolutely minimal footprint and very straight forward code that easy to audit and contribute
* 📱 Compatible with TON wallet apps

Developed by [Whales Corp.](https://whalescorp.com)

## Installation

Right now TON app is not available through Ledger Live and have to be sideloaded to Ledger Nano S+. We support Ledger Nano S, but we generally recommend a new one. Everything have to be done on Linux on x64 CPU with docker installed.
* Checkout this repository
* Connect Ledger to the computer, turn it on and unlock.
* Start and login into environment. Run in terminal: `./env.sh`
* Run `make load` to build and upload app to the Ledger

## Web Client

You can use Ledger TON App by navigating to [Tonblack Web App page](https://tonblack.com), connect your ledger to the browser and make transfers.

## Programmatic Usage

There are some experimental (but stable) libraries:
* [ton-ledger-ts](https://github.com/ton-community/ton-ledger-ts)

## Documentation

* [Supported TON messages](doc/MESSAGES.md)
* [Low level ADPU](doc/APDU.md)
* [Commands](doc/COMMANDS.md)
* [Base Transaction](doc/TRANSACTION.md)

## Development

For Mac or Windows users we recommend to use VS Code Remote with a x64 Linux laptop and do all work there and connect ledger directly to it. 

To build you need to have Nano App Builder docker image built:
```bash
git clone https://github.com/LedgerHQ/ledger-app-builder.git
cd ledger-app-builder
docker build -t ledger-app-builder:latest .
```

To build and run just launch Ledger App Builder environment by calling `./env.sh` and then:
* `make` - to build app
* `make load` - to build and upload to a Ledger
* `make clean` - to clean build
* `make scan-build` - for Clang static analyzer
* `cmake -Bbuild -H. && make -C build && CTEST_OUTPUT_ON_FAILURE=1 make -C build test` - for unit tests in `unit-tests` directory
* `doxygen .doxygen/Doxyfile` - to generate html and latex documentation

## Tests & Continuous Integration

The flow processed in [GitHub Actions](https://github.com/features/actions) is the following:

- Code formatting with [clang-format](http://clang.llvm.org/docs/ClangFormat.html)
- Compilation of the application for Ledger Nano S in [ledger-app-builder](https://github.com/LedgerHQ/ledger-app-builder)
- Unit tests of C functions with [cmocka](https://cmocka.org/) (see [unit-tests/](unit-tests/))
- End-to-end tests with [Speculos](https://github.com/LedgerHQ/speculos) emulator (see [tests/](tests/))
- Code coverage with [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)/[lcov](http://ltp.sourceforge.net/coverage/lcov.php) and upload to [codecov.io](https://about.codecov.io)
- Documentation generation with [doxygen](https://www.doxygen.nl)

It outputs 4 artifacts:

- `boilerplate-app-debug` within output files of the compilation process in debug mode
- `speculos-log` within APDU command/response when executing end-to-end tests
- `code-coverage` within HTML details of code coverage
- `documentation` within HTML auto-generated documentation

docker run --rm -ti -v "/dev/bus/usb:/dev/bus/usb" -v "$(realpath .):/app" -e "BOLOS_SDK=/opt/nanosplus-secure-sdk" --privileged ledger-app-builder:latest
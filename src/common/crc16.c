#import "crc16.h"

uint16_t crc16(char *ptr, size_t count) {
    size_t crc = 0;
    uint8_t i;
    while (--count >= 0) {
        crc = crc ^ (size_t)*ptr++ << 8;
        i = 8;
        do {
            if (crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;
        } while (--i);
    }
    return (crc);
}
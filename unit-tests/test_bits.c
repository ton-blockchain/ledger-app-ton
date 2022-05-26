#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <cmocka.h>

#include "common/bits.h"

static void test_bits(void **state) {
    
    uint8_t expected[21] = {
        0xe3,0x79,0x2f,0x59,0x01,0x0a,0x30,0xf4,0x24,0x05,0x02,0x54,0x0b,0xe4,0x00,0x50,0x2d,0xdd,0xef,0xa0,0x38
    };

    BitStream_t bits;
    BitStream_init(&bits);

    // Test bits writes
    BitStream_storeBit(&bits, 1);
    assert_true(bits.data_cursor == 1);
    BitStream_storeBit(&bits, 2);
    assert_true(bits.data_cursor == 2);
    BitStream_storeBit(&bits, 3);
    assert_true(bits.data_cursor == 3);
    BitStream_storeBit(&bits, 0);
    assert_true(bits.data_cursor == 4);
    BitStream_storeBit(&bits, -1);
    assert_true(bits.data_cursor == 5);
    BitStream_storeBit(&bits, -2);
    assert_true(bits.data_cursor == 6);
    BitStream_storeBit(&bits, 1);
    assert_true(bits.data_cursor == 7);
    BitStream_storeBit(&bits, 1);
    assert_true(bits.data_cursor == 8);
    assert_int_equal(bits.data[0], 0xe3);

    // Test uint writes
    BitStream_storeUint(&bits, 121, 8);
    BitStream_storeUint(&bits, 12121, 16);
    assert_int_equal(bits.data[1], 0x79);
    assert_int_equal(bits.data[2], 0x2f);
    assert_int_equal(bits.data[3], 0x59);

    // Test coins writes
    // 010a30f4240502540be400
    BitStream_storeCoins(&bits, 0);
    assert_true(bits.data_cursor == 36);
    BitStream_storeCoins(&bits, 10);
    assert_true(bits.data_cursor == 48);
    BitStream_storeCoins(&bits, 1000000);
    assert_true(bits.data_cursor == 76);
    BitStream_storeCoins(&bits, 10000000000);
    assert_true(bits.data_cursor == 120);
    BitStream_storeCoins(&bits, 12312312323);
    assert_true(bits.data_cursor == 164);

    // Finalize
    BitStream_finalize(&bits);
    
    // Contents and hash
    assert_memory_equal(bits.data, expected, sizeof(expected));
    
}

int main() {
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_bits)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}

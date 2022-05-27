def test_get_public_key(cmd):
    pub_key = cmd.get_public_key(
        bip32_path="m/44'/607'/0'/0'",
        display=False
    )

    assert len(pub_key) == 32

    pub_key2 = cmd.get_public_key(
        bip32_path="m/44'/607'/0'/1'",
        display=False
    )

    assert len(pub_key2) == 32
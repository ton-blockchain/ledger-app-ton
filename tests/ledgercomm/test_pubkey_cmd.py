def test_get_public_key(cmd):
    pub_key = cmd.get_public_key(
        bip32_path="m/44'/607'/0'/0'",
        display=False
    )  # type: bytes, bytes

    assert len(pub_key) == 65

    pub_key2 = cmd.get_public_key(
        bip32_path="m/44'/607'/0'/1'",
        display=False
    )  # type: bytes, bytes

    assert len(pub_key2) == 65

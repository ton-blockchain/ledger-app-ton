def test_version(cmd):
    assert cmd.get_version() == (1, 1, 0)

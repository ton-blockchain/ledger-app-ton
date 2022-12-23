def test_version(cmd):
    assert cmd.get_version() == (1, 2, 0)

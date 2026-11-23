import hashlib

from mmoportal import passwords


def test_argon2id_round_trip():
    stored = passwords.hash_password("correct horse")
    assert stored.startswith("$argon2id$v=19$m=65536,t=2,p=1$")
    assert passwords.check(stored, "correct horse") == (True, False)
    assert passwords.check(stored, "wrong horse") == (False, False)


def test_libsodium_hash_from_the_login_server():
    # Made by crypto_pwhash_str with the LoginServer's interactive limits.
    stored = "$argon2id$v=19$m=65536,t=2,p=1$GyEIOp8wTpSPUk5bZpWs2w$B5tlagNqfJTmCHbG2AazVkIxE0mbF4Txc450ZoKgxGQ"
    assert passwords.check(stored, "fromserver") == (True, False)
    assert passwords.check(stored, "fromServer") == (False, False)


def test_legacy_sha1_is_accepted_and_needs_rehash():
    stored = hashlib.sha1(b"old password").hexdigest()
    assert passwords.check(stored, "old password") == (True, True)
    assert passwords.check(stored.upper(), "old password") == (True, True)
    assert passwords.check(stored, "other") == (False, False)


def test_weaker_argon2id_needs_rehash():
    import nacl.pwhash

    weak = nacl.pwhash.argon2id.str(
        b"pw", opslimit=nacl.pwhash.argon2id.OPSLIMIT_MIN, memlimit=nacl.pwhash.argon2id.MEMLIMIT_MIN
    ).decode()
    assert passwords.check(weak, "pw") == (True, True)


def test_password_rules():
    assert passwords.problem("short") is not None
    assert passwords.problem("x" * 129) is not None
    assert passwords.problem("SomeUser1", "someuser1") is not None
    assert passwords.problem("long enough") is None

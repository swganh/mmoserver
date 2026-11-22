"""Password hashing, the same way the LoginServer does it.

The LoginServer (src/LoginServer/Password.cpp) stores Argon2id hashes in
libsodium's string form with libsodium's interactive limits. PyNaCl calls
the same libsodium functions, so a password set here works in game and the
other way round. Accounts that still hold an old unsalted SHA-1 hex hash
are checked against that and stored as Argon2id from then on, as the
LoginServer does at login.
"""
import hashlib
import hmac

import nacl.exceptions
import nacl.pwhash

OPS = nacl.pwhash.argon2id.OPSLIMIT_INTERACTIVE
MEM = nacl.pwhash.argon2id.MEMLIMIT_INTERACTIVE

MIN_LENGTH = 8
MAX_LENGTH = 128


def hash_password(text):
    return nacl.pwhash.argon2id.str(text.encode("utf-8"), opslimit=OPS, memlimit=MEM).decode("ascii")


def is_argon2id(stored):
    return stored.startswith("$argon2id$")


_DUMMY = None


def check_nothing(text):
    """Spends a check's time, so a missing account answers as slowly as a
    wrong password and does not reveal which usernames exist."""
    global _DUMMY
    if _DUMMY is None:
        _DUMMY = hash_password("no such account")
    try:
        nacl.pwhash.verify(_DUMMY.encode("ascii"), text.encode("utf-8"))
    except nacl.exceptions.InvalidkeyError:
        pass


def check(stored, text):
    """Returns (right, needs_rehash)."""
    data = text.encode("utf-8")
    if is_argon2id(stored):
        try:
            nacl.pwhash.verify(stored.encode("ascii"), data)
        except (nacl.exceptions.InvalidkeyError, nacl.exceptions.CryptoError, ValueError):
            return False, False
        return True, _needs_rehash(stored)

    # Old unsalted SHA-1 hex, as MariaDB's SHA1() writes it.
    legacy = hashlib.sha1(data).hexdigest()
    if hmac.compare_digest(stored.strip().lower().encode(), legacy.encode()):
        return True, True
    check_nothing(text)
    return False, False


def _needs_rehash(stored):
    # $argon2id$v=19$m=65536,t=2,p=1$...: weaker than today's limits?
    try:
        params = dict(kv.split("=") for kv in stored.split("$")[3].split(","))
        return int(params["m"]) * 1024 < MEM or int(params["t"]) < OPS
    except (IndexError, KeyError, ValueError):
        return True


def problem(text, username=""):
    """Why a new password cannot be used, or None."""
    if len(text) < MIN_LENGTH:
        return f"Use a password of at least {MIN_LENGTH} characters."
    if len(text) > MAX_LENGTH:
        return f"Use a password of at most {MAX_LENGTH} characters."
    if username and text.lower() == username.lower():
        return "The password cannot be the username."
    return None


def session_tag(stored):
    """A short fingerprint of the stored hash, kept in the session: when the
    password changes, sessions made with the old one stop working."""
    return hashlib.sha256(stored.encode("utf-8")).hexdigest()[:16]

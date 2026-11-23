"""Changes to game accounts, shared by the player and admin pages."""
import hashlib
import secrets

from flask import current_app, url_for

from . import db, mail, passwords


def username_taken(username):
    return db.one("SELECT 1 AS x FROM {game}.account WHERE account_username = %s", username) is not None


def email_taken(email, except_account=None):
    if not email:
        return False
    row = db.one(
        "SELECT account_id FROM {game}.account WHERE LOWER(account_email) = LOWER(%s) AND account_id <> %s LIMIT 1",
        email, except_account or 0,
    )
    return row is not None


def create(username, password, email):
    """Creates a game account as mmoserverdb/setup.sh add-account does.
    Returns the new account_id."""
    stored = passwords.hash_password(password)
    with db.transaction():
        _, account_id = db.run(
            "INSERT INTO {game}.account (account_username, account_password, account_email, account_active) "
            "VALUES (%s, %s, %s, 1)",
            username, stored, email,
        )
        db.run(
            "UPDATE {game}.account SET account_station_id = 100000000 + account_id WHERE account_id = %s",
            account_id,
        )
    return account_id


def set_password(account_id, password):
    """Stores a new password and cancels the account's reset links.
    Returns the stored hash."""
    stored = passwords.hash_password(password)
    db.run("UPDATE {game}.account SET account_password = %s WHERE account_id = %s", stored, account_id)
    db.run(
        "UPDATE {portal}.password_reset SET used_at = NOW() WHERE account_id = %s AND used_at IS NULL",
        account_id,
    )
    return stored


def store_rehash(account_id, password):
    db.run(
        "UPDATE {game}.account SET account_password = %s WHERE account_id = %s",
        passwords.hash_password(password), account_id,
    )


# --- Reset links ----------------------------------------------------------------

def _token_hash(token):
    return hashlib.sha256(token.encode("ascii")).hexdigest()


def make_reset_link(account_id, requested_by=None):
    token = secrets.token_urlsafe(32)
    minutes = current_app.config["RESET_LINK_MINUTES"]
    db.run(
        "INSERT INTO {portal}.password_reset (account_id, token_hash, expires_at, requested_by) "
        "VALUES (%s, %s, NOW() + INTERVAL %s MINUTE, %s)",
        account_id, _token_hash(token), minutes, requested_by,
    )
    return current_app.config["BASE_URL"] + url_for("auth.reset", token=token)


def send_reset_link(account, requested_by=None):
    """Emails a reset link to the account's address. Returns False when the
    account has no email address."""
    if not account["account_email"]:
        return False
    link = make_reset_link(account["account_id"], requested_by)
    mail.send_template(
        account["account_email"],
        "Reset your password",
        "mail/reset.txt",
        username=account["account_username"],
        link=link,
        minutes=current_app.config["RESET_LINK_MINUTES"],
    )
    return True


def account_for_token(token):
    """The account a valid, unused, unexpired reset token is for, or None."""
    if not token or len(token) > 100:
        return None
    row = db.one(
        "SELECT account_id FROM {portal}.password_reset "
        "WHERE token_hash = %s AND used_at IS NULL AND expires_at > NOW()",
        _token_hash(token),
    )
    return row["account_id"] if row else None


def characters(account_id):
    return db.all(
        "SELECT id, firstname, lastname, archived FROM {game}.characters WHERE account_id = %s ORDER BY firstname",
        account_id,
    )


def lift_expired_bans():
    """Unbans accounts whose temporary ban is over. The LoginServer only
    reads account_banned, so the portal lifts them (on its pages, at most
    every few minutes, and with `flask lift-expired-bans`)."""
    count, _ = db.run(
        "UPDATE {game}.account SET account_banned = 0, banned_until = NULL "
        "WHERE account_banned = 1 AND banned_until IS NOT NULL AND banned_until <= NOW()"
    )
    return count

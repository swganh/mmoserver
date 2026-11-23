"""Who is signed in, CSRF protection, rate limits and the audit log."""
import hmac
import logging
import secrets
from functools import wraps

from flask import abort, flash, g, redirect, request, session, url_for

from . import db, passwords, permissions

log = logging.getLogger("mmoportal")

USERNAME_RULE = r"^[A-Za-z0-9_]{3,32}$"   # as mmoserverdb/setup.sh add-account

ACCOUNT_COLUMNS = (
    "account_id, account_username, account_password, account_email, account_station_id, "
    "account_csr, account_banned, banned_until, banned_date, account_active, account_joindate, "
    "account_lastlogin, account_characters_allowed, account_loggedin"
)


def find_account(account_id=None, username=None):
    if account_id is not None:
        return db.one(f"SELECT {ACCOUNT_COLUMNS} FROM {{game}}.account WHERE account_id = %s", account_id)
    return db.one(f"SELECT {ACCOUNT_COLUMNS} FROM {{game}}.account WHERE account_username = %s", username)


def is_banned(account):
    return bool(account["account_banned"])


# --- Sessions -------------------------------------------------------------------

def load_user():
    """Runs before every request: g.user is the signed-in account, or None."""
    g.user = None
    account_id = session.get("account_id")
    if not account_id:
        return
    account = find_account(account_id=account_id)
    if (
        account is None
        or is_banned(account)
        or not hmac.compare_digest(session.get("pw_tag", ""), passwords.session_tag(account["account_password"]))
    ):
        session.clear()
        return
    roles, perms, admin = permissions.load_for(account_id)
    account["roles"] = roles
    account["permissions"] = perms
    account["is_admin"] = admin
    g.user = account


def login(account):
    session.clear()
    session.permanent = True
    session["account_id"] = account["account_id"]
    session["pw_tag"] = passwords.session_tag(account["account_password"])
    session["csrf"] = secrets.token_urlsafe(32)


def refresh_password_tag(stored_hash):
    """After the signed-in user changes their own password: this session
    stays signed in; every other one is signed out."""
    session["pw_tag"] = passwords.session_tag(stored_hash)


def logout():
    session.clear()


def require_login_redirect():
    flash("Please sign in first.", "info")
    return redirect(url_for("auth.login", next=request.full_path if request.method == "GET" else None))


def login_required(view):
    @wraps(view)
    def inner(*args, **kwargs):
        if not g.get("user"):
            return require_login_redirect()
        return view(*args, **kwargs)

    return inner


# --- CSRF -----------------------------------------------------------------------

def csrf_token():
    if "csrf" not in session:
        session["csrf"] = secrets.token_urlsafe(32)
    return session["csrf"]


def check_csrf():
    """Runs before every request: a POST must carry the session's token."""
    if request.method in ("GET", "HEAD", "OPTIONS"):
        return
    sent = request.form.get("csrf_token", "")
    expected = session.get("csrf", "")
    if not expected or not hmac.compare_digest(sent, expected):
        abort(400, "The form expired. Go back, reload the page and try again.")


# --- Rate limits ----------------------------------------------------------------

LIMITS = {
    # kind: (minutes, most per address, most per username)
    "login": (15, 20, 8),
    "reset": (60, 5, 3),
    "register": (60, 5, 0),
}


def client_ip():
    return (request.remote_addr or "")[:45]


def limited(kind, username=""):
    minutes, per_ip, per_name = LIMITS[kind]
    row = db.one(
        "SELECT COUNT(*) AS n FROM {portal}.attempt WHERE kind = %s AND ip = %s "
        "AND at > NOW() - INTERVAL %s MINUTE",
        kind, client_ip(), minutes,
    )
    if row["n"] >= per_ip:
        return True
    if per_name and username:
        row = db.one(
            "SELECT COUNT(*) AS n FROM {portal}.attempt WHERE kind = %s AND username = %s "
            "AND at > NOW() - INTERVAL %s MINUTE",
            kind, username.lower()[:64], minutes,
        )
        if row["n"] >= per_name:
            return True
    return False


def record_attempt(kind, username=""):
    db.run(
        "INSERT INTO {portal}.attempt (kind, ip, username) VALUES (%s, %s, %s)",
        kind, client_ip(), username.lower()[:64],
    )
    # Keep the table small.
    db.run("DELETE FROM {portal}.attempt WHERE at < NOW() - INTERVAL 1 DAY LIMIT 100")


def clear_attempts(kind, username):
    db.run("DELETE FROM {portal}.attempt WHERE kind = %s AND username = %s", kind, username.lower()[:64])


# --- Audit log ------------------------------------------------------------------

def audit(action, target_account_id=None, detail=""):
    actor = g.user["account_id"] if g.get("user") else None
    db.run(
        "INSERT INTO {portal}.audit_log (actor_account_id, target_account_id, action, detail, ip) "
        "VALUES (%s, %s, %s, %s, %s)",
        actor, target_account_id, action, detail[:500], client_ip(),
    )
    log.info("audit: %s actor=%s target=%s %s", action, actor, target_account_id, detail)

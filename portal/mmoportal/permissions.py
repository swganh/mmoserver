"""Staff permissions.

Permissions are named in code (below); roles, made in the admin area, are
sets of them, and accounts are given roles. The builtin Administrator role
holds every permission and is the only one that may manage roles.

Two rules keep a permission from being used to gain more:
  * only Administrators manage roles and give them to accounts;
  * only Administrators act on staff accounts (any account with a role), so
    a CSR cannot reset an Administrator's password and log in as them.

New features add their permissions here, e.g. 'shop.manage' or
'mail.read' for the in-game item shop or mail portal.
"""
from functools import wraps

from flask import abort, g

from . import db

ADMINISTRATOR_ROLE = 1

PERMISSIONS = {
    "admin.access": "Open the admin area",
    "accounts.view": "Look up accounts and their characters",
    "accounts.create": "Create game accounts",
    "accounts.edit": "Change an account's email, character limit and active flag",
    "accounts.reset_password": "Set an account's password or send it a reset link",
    "accounts.ban": "Ban and unban accounts",
    "accounts.game_csr": "Give or take the in-game CSR flag",
    "audit.view": "Read the audit log",
}


def load_for(account_id):
    """(set of role names, set of permissions, is_administrator)."""
    rows = db.all(
        "SELECT r.role_id, r.name FROM {portal}.account_role ar "
        "JOIN {portal}.role r ON r.role_id = ar.role_id WHERE ar.account_id = %s",
        account_id,
    )
    roles = {r["name"] for r in rows}
    is_admin = any(r["role_id"] == ADMINISTRATOR_ROLE for r in rows)
    if is_admin:
        return roles, set(PERMISSIONS), True
    perms = {
        r["permission"]
        for r in db.all(
            "SELECT DISTINCT rp.permission FROM {portal}.account_role ar "
            "JOIN {portal}.role_permission rp ON rp.role_id = ar.role_id WHERE ar.account_id = %s",
            account_id,
        )
        if r["permission"] in PERMISSIONS
    }
    return roles, perms, False


def has(permission):
    user = g.get("user")
    return bool(user) and permission in user["permissions"]


def is_admin():
    user = g.get("user")
    return bool(user) and user["is_admin"]


def is_staff(account_id):
    return db.one("SELECT 1 AS x FROM {portal}.account_role WHERE account_id = %s LIMIT 1", account_id) is not None


def may_act_on(account_id):
    """Whether the signed-in user may change this account from the admin area."""
    return is_admin() or not is_staff(account_id)


def administrator_count():
    row = db.one("SELECT COUNT(*) AS n FROM {portal}.account_role WHERE role_id = %s", ADMINISTRATOR_ROLE)
    return row["n"]


def require(*permissions):
    """View decorator: signed in, and holding every permission named."""

    def wrap(view):
        @wraps(view)
        def inner(*args, **kwargs):
            from .auth_state import require_login_redirect

            if not g.get("user"):
                return require_login_redirect()
            if not all(has(p) for p in permissions):
                abort(403)
            return view(*args, **kwargs)

        return inner

    return wrap


def require_admin(view):
    @wraps(view)
    def inner(*args, **kwargs):
        from .auth_state import require_login_redirect

        if not g.get("user"):
            return require_login_redirect()
        if not is_admin():
            abort(403)
        return view(*args, **kwargs)

    return inner

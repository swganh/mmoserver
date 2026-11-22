"""The admin area: accounts, staff roles and the audit log."""
import re

from flask import Blueprint, abort, flash, g, redirect, render_template, request, url_for

from .. import accounts, auth_state, db, game, passwords, permissions
from ..auth_state import USERNAME_RULE
from ..permissions import ADMINISTRATOR_ROLE, PERMISSIONS, require, require_admin
from .auth import EMAIL_RULE

bp = Blueprint("admin", __name__, url_prefix="/admin")

PAGE_SIZE = 50


def _account_or_404(account_id):
    account = auth_state.find_account(account_id=account_id)
    if account is None:
        abort(404)
    return account


def _guard(account):
    """Staff accounts are only changed by Administrators."""
    if not permissions.may_act_on(account["account_id"]):
        abort(403, "Only an Administrator can change a staff account.")


def _back(account_id):
    return redirect(url_for("admin.account", account_id=account_id))


# --- Overview -------------------------------------------------------------------

@bp.route("/")
@require("admin.access")
def index():
    stats = db.one(
        "SELECT COUNT(*) AS accounts, COALESCE(SUM(account_banned), 0) AS banned, "
        "COALESCE(SUM(account_loggedin <> 0), 0) AS online FROM {game}.account"
    )
    stats["staff"] = db.one("SELECT COUNT(DISTINCT account_id) AS n FROM {portal}.account_role")["n"]
    recent = []
    if permissions.has("audit.view"):
        recent = _audit_rows(limit=10)
    return render_template("admin/index.html", stats=stats, recent=recent)


# --- Accounts -------------------------------------------------------------------

@bp.route("/accounts")
@require("admin.access", "accounts.view")
def accounts_list():
    search = request.args.get("q", "").strip()
    page = max(1, request.args.get("page", 1, type=int))
    where, args = "", []
    if search:
        where = "WHERE account_username LIKE %s OR account_email LIKE %s OR account_id = %s"
        like = "%" + search.replace("\\", "\\\\").replace("%", "\\%").replace("_", "\\_") + "%"
        args = [like, like, search if search.isdigit() else -1]
    rows = db.all(
        "SELECT account_id, account_username, account_email, account_banned, account_csr, account_active, "
        f"account_joindate, account_lastlogin, account_loggedin FROM {{game}}.account {where} "
        "ORDER BY account_id DESC LIMIT %s OFFSET %s",
        *args, PAGE_SIZE + 1, (page - 1) * PAGE_SIZE,
    )
    more = len(rows) > PAGE_SIZE
    rows = rows[:PAGE_SIZE]
    staff = {}
    if rows:
        ids = [r["account_id"] for r in rows]
        marks = ",".join(["%s"] * len(ids))
        for r in db.all(
            "SELECT ar.account_id, r.name FROM {portal}.account_role ar JOIN {portal}.role r ON r.role_id = ar.role_id "
            f"WHERE ar.account_id IN ({marks}) ORDER BY r.name",
            *ids,
        ):
            staff.setdefault(r["account_id"], []).append(r["name"])
    return render_template("admin/accounts.html", rows=rows, search=search, page=page, more=more, staff=staff)


@bp.route("/accounts/new", methods=["GET", "POST"])
@require("admin.access", "accounts.create")
def account_new():
    form = {"username": request.form.get("username", "").strip(), "email": request.form.get("email", "").strip()}
    if request.method == "POST":
        password = request.form.get("password", "")
        error = None
        if not re.match(USERNAME_RULE, form["username"]):
            error = "Usernames are 3 to 32 letters, digits or underscores."
        elif form["email"] and (not EMAIL_RULE.match(form["email"]) or len(form["email"]) > 64):
            error = "Enter a valid email address, or none."
        else:
            error = passwords.problem(password, form["username"])
        if not error and accounts.username_taken(form["username"]):
            error = "That username is taken."
        if not error and accounts.email_taken(form["email"]):
            error = "Another account uses that email address."
        if error:
            flash(error, "error")
            return render_template("admin/account_new.html", form=form), 400
        account_id = accounts.create(form["username"], password, form["email"])
        auth_state.audit("admin.account_create", account_id, form["username"])
        flash(f"Account {form['username']} created.", "success")
        return _back(account_id)
    return render_template("admin/account_new.html", form=form)


@bp.route("/accounts/<int:account_id>")
@require("admin.access", "accounts.view")
def account(account_id):
    acc = _account_or_404(account_id)
    roles, _, _ = permissions.load_for(account_id)
    characters = accounts.characters(account_id)
    try:
        admin_skills = game.held([c["id"] for c in characters])
        skills_readable = True
    except game.NoAccess:
        admin_skills, skills_readable = {}, False
    return render_template(
        "admin/account.html",
        account=acc,
        roles=roles,
        all_roles=db.all("SELECT role_id, name, description FROM {portal}.role ORDER BY role_id"),
        characters=characters,
        admin_skills=admin_skills,
        skills_readable=skills_readable,
        SKILLS=game.SKILLS,
        may_change=permissions.may_act_on(account_id),
        history=_audit_rows(target=account_id, limit=20) if permissions.has("audit.view") else [],
    )


@bp.route("/accounts/<int:account_id>/edit", methods=["POST"])
@require("admin.access", "accounts.edit")
def account_edit(account_id):
    acc = _account_or_404(account_id)
    _guard(acc)
    email = request.form.get("email", "").strip()
    try:
        allowed = int(request.form.get("characters_allowed", ""))
    except ValueError:
        allowed = -1
    active = 1 if request.form.get("active") else 0
    if email and (not EMAIL_RULE.match(email) or len(email) > 64):
        flash("Enter a valid email address, or none.", "error")
    elif accounts.email_taken(email, account_id):
        flash("Another account uses that email address.", "error")
    elif not 0 <= allowed <= 10:
        flash("The character limit is 0 to 10.", "error")
    else:
        db.run(
            "UPDATE {game}.account SET account_email = %s, account_characters_allowed = %s, account_active = %s "
            "WHERE account_id = %s",
            email, allowed, active, account_id,
        )
        auth_state.audit(
            "admin.account_edit", account_id,
            f"email={email} characters_allowed={allowed} active={active}",
        )
        flash("Saved.", "success")
    return _back(account_id)


@bp.route("/accounts/<int:account_id>/password", methods=["POST"])
@require("admin.access", "accounts.reset_password")
def account_password(account_id):
    acc = _account_or_404(account_id)
    _guard(acc)
    password = request.form.get("password", "")
    error = passwords.problem(password, acc["account_username"])
    if error:
        flash(error, "error")
    else:
        accounts.set_password(account_id, password)
        auth_state.audit("admin.password_set", account_id)
        flash("Password set. Tell the player, and ask them to change it.", "success")
    return _back(account_id)


@bp.route("/accounts/<int:account_id>/send-reset", methods=["POST"])
@require("admin.access", "accounts.reset_password")
def account_send_reset(account_id):
    acc = _account_or_404(account_id)
    _guard(acc)
    if accounts.send_reset_link(acc, requested_by=g.user["account_id"]):
        auth_state.audit("admin.reset_link_sent", account_id)
        flash(f"Reset link sent to {acc['account_email']}.", "success")
    else:
        flash("The account has no email address; set a password instead.", "error")
    return _back(account_id)


@bp.route("/accounts/<int:account_id>/ban", methods=["POST"])
@require("admin.access", "accounts.ban")
def account_ban(account_id):
    acc = _account_or_404(account_id)
    _guard(acc)
    if account_id == g.user["account_id"]:
        abort(400, "You cannot ban yourself.")
    reason = request.form.get("reason", "").strip()[:200]
    days = request.form.get("days", "").strip()
    if days and (not days.isdigit() or not 1 <= int(days) <= 3650):
        flash("Ban for 1 to 3650 days, or leave it empty for a permanent ban.", "error")
        return _back(account_id)
    db.run(
        "UPDATE {game}.account SET account_banned = 1, banned_date = NOW(), "
        "banned_until = IF(%s = 0, NULL, NOW() + INTERVAL %s DAY) WHERE account_id = %s",
        int(days or 0), int(days or 0), account_id,
    )
    auth_state.audit("admin.ban", account_id, f"{days + ' days' if days else 'permanent'}: {reason}")
    flash("Banned. It takes effect at the player's next login.", "success")
    return _back(account_id)


@bp.route("/accounts/<int:account_id>/unban", methods=["POST"])
@require("admin.access", "accounts.ban")
def account_unban(account_id):
    acc = _account_or_404(account_id)
    _guard(acc)
    db.run(
        "UPDATE {game}.account SET account_banned = 0, banned_until = NULL WHERE account_id = %s", account_id
    )
    auth_state.audit("admin.unban", account_id)
    flash("Ban lifted.", "success")
    return _back(account_id)


@bp.route("/accounts/<int:account_id>/csr", methods=["POST"])
@require("admin.access", "accounts.game_csr")
def account_csr(account_id):
    acc = _account_or_404(account_id)
    _guard(acc)
    value = 1 if request.form.get("csr") == "1" else 0
    db.run("UPDATE {game}.account SET account_csr = %s WHERE account_id = %s", value, account_id)
    auth_state.audit("admin.game_csr", account_id, f"csr={value}")
    flash("In-game CSR flag " + ("given" if value else "taken away") + "; it applies " + RELOAD_HINT + ".", "success")
    return _back(account_id)


@bp.route("/accounts/<int:account_id>/roles", methods=["POST"])
@require_admin
def account_roles(account_id):
    _account_or_404(account_id)
    wanted = {int(r) for r in request.form.getlist("role") if r.isdigit()}
    known = {r["role_id"]: r["name"] for r in db.all("SELECT role_id, name FROM {portal}.role")}
    wanted &= set(known)
    current = {r["role_id"] for r in db.all("SELECT role_id FROM {portal}.account_role WHERE account_id = %s", account_id)}

    if ADMINISTRATOR_ROLE in current and ADMINISTRATOR_ROLE not in wanted and permissions.administrator_count() <= 1:
        flash("This is the last Administrator; make another one first.", "error")
        return _back(account_id)

    with db.transaction():
        for role_id in current - wanted:
            db.run("DELETE FROM {portal}.account_role WHERE account_id = %s AND role_id = %s", account_id, role_id)
        for role_id in wanted - current:
            db.run(
                "INSERT INTO {portal}.account_role (account_id, role_id, granted_by) VALUES (%s, %s, %s)",
                account_id, role_id, g.user["account_id"],
            )
    if wanted != current:
        auth_state.audit(
            "admin.roles", account_id,
            "now: " + (", ".join(sorted(known[r] for r in wanted)) or "none"),
        )
    flash("Roles saved.", "success")
    return _back(account_id)


# --- In-game staff commands -------------------------------------------------------

# The ZoneServer keeps a logged-out character in memory for Zone_Player_Logout
# seconds (300 by default) so a dropped connection can resume, and a login in
# that time reuses it: skills and the CSR flag are only read from the
# database when the character is loaded again.
RELOAD_HINT = "once the character is fully out of the game: 5 minutes after logging out, or after its zone server restarts"

NO_SKILL_ACCESS = (
    "The portal's database user cannot read or change character skills yet: "
    "run install.sh install again to give it access."
)


@bp.route("/csr")
@require("admin.access", "accounts.view")
def csr():
    try:
        holders, client_only, readable = game.holders(), game.client_only_commands(), True
    except game.NoAccess:
        holders, client_only, readable = [], [], False
    csr_accounts = db.all(
        "SELECT account_id, account_username FROM {game}.account WHERE account_csr <> 0 ORDER BY account_username"
    )
    return render_template(
        "admin/csr.html",
        commands=game.COMMANDS, skills=game.SKILLS, flag_effects=game.CSR_FLAG_EFFECTS,
        holders=holders, client_only=client_only, readable=readable, csr_accounts=csr_accounts,
        no_access=NO_SKILL_ACCESS,
    )


@bp.route("/characters/<int:character_id>/admin-skills", methods=["POST"])
@require("admin.access", "game.admin_commands")
def character_admin_skills(character_id):
    row = db.one("SELECT account_id, firstname FROM {game}.characters WHERE id = %s", character_id)
    if row is None:
        abort(404)
    acc = _account_or_404(row["account_id"])
    _guard(acc)
    try:
        added, removed = game.set_skills(character_id, request.form.getlist("skill"))
    except game.NoAccess:
        flash(NO_SKILL_ACCESS, "error")
        return _back(acc["account_id"])
    if added or removed:
        auth_state.audit(
            "admin.game_admin_skills", acc["account_id"],
            f"{row['firstname']} (#{character_id}): +{','.join(sorted(added)) or '-'} "
            f"-{','.join(sorted(removed)) or '-'}",
        )
        flash(f"In-game admin commands for {row['firstname']} saved; they apply " + RELOAD_HINT + ".", "success")
    else:
        flash("Nothing changed.", "info")
    return _back(acc["account_id"])


# --- Roles ----------------------------------------------------------------------

@bp.route("/roles")
@require_admin
def roles():
    rows = db.all(
        "SELECT r.role_id, r.name, r.description, r.builtin, "
        "(SELECT COUNT(*) FROM {portal}.account_role ar WHERE ar.role_id = r.role_id) AS members "
        "FROM {portal}.role r ORDER BY r.role_id"
    )
    perms = {}
    for r in db.all("SELECT role_id, permission FROM {portal}.role_permission"):
        perms.setdefault(r["role_id"], []).append(r["permission"])
    return render_template("admin/roles.html", rows=rows, perms=perms, PERMISSIONS=PERMISSIONS)


@bp.route("/roles/new", methods=["GET", "POST"])
@bp.route("/roles/<int:role_id>", methods=["GET", "POST"])
@require_admin
def role(role_id=None):
    current = None
    if role_id is not None:
        current = db.one("SELECT role_id, name, description, builtin FROM {portal}.role WHERE role_id = %s", role_id)
        if current is None:
            abort(404)
        if current["builtin"]:
            abort(400, "The Administrator role always has every permission and cannot be changed.")
    held = set()
    if current:
        held = {r["permission"] for r in db.all("SELECT permission FROM {portal}.role_permission WHERE role_id = %s", role_id)}

    if request.method == "POST":
        name = request.form.get("name", "").strip()
        description = request.form.get("description", "").strip()[:255]
        chosen = {p for p in request.form.getlist("permission") if p in PERMISSIONS}
        other = db.one("SELECT role_id FROM {portal}.role WHERE name = %s", name)
        if not 2 <= len(name) <= 64:
            flash("Give the role a name of 2 to 64 characters.", "error")
        elif other and other["role_id"] != role_id:
            flash("Another role has that name.", "error")
        else:
            with db.transaction():
                if current:
                    db.run("UPDATE {portal}.role SET name = %s, description = %s WHERE role_id = %s", name, description, role_id)
                    db.run("DELETE FROM {portal}.role_permission WHERE role_id = %s", role_id)
                else:
                    _, role_id = db.run("INSERT INTO {portal}.role (name, description) VALUES (%s, %s)", name, description)
                for p in sorted(chosen):
                    db.run("INSERT INTO {portal}.role_permission (role_id, permission) VALUES (%s, %s)", role_id, p)
            auth_state.audit("admin.role_save", None, f"{name}: {', '.join(sorted(chosen)) or 'no permissions'}")
            flash(f"Role {name} saved.", "success")
            return redirect(url_for("admin.roles"))
        current = {"role_id": role_id, "name": name, "description": description, "builtin": 0}
        held = chosen

    return render_template("admin/role.html", role=current, held=held, PERMISSIONS=PERMISSIONS)


@bp.route("/roles/<int:role_id>/delete", methods=["POST"])
@require_admin
def role_delete(role_id):
    current = db.one("SELECT name, builtin FROM {portal}.role WHERE role_id = %s", role_id)
    if current is None:
        abort(404)
    if current["builtin"]:
        abort(400, "The Administrator role cannot be deleted.")
    db.run("DELETE FROM {portal}.role WHERE role_id = %s", role_id)
    auth_state.audit("admin.role_delete", None, current["name"])
    flash(f"Role {current['name']} deleted.", "success")
    return redirect(url_for("admin.roles"))


# --- Audit log ------------------------------------------------------------------

def _audit_rows(target=None, limit=100, offset=0):
    where, args = "", []
    if target is not None:
        where, args = "WHERE a.target_account_id = %s", [target]
    return db.all(
        "SELECT a.at, a.action, a.detail, a.ip, a.actor_account_id, a.target_account_id, "
        "actor.account_username AS actor_name, target.account_username AS target_name "
        "FROM {portal}.audit_log a "
        "LEFT JOIN {game}.account actor ON actor.account_id = a.actor_account_id "
        "LEFT JOIN {game}.account target ON target.account_id = a.target_account_id "
        f"{where} ORDER BY a.audit_id DESC LIMIT %s OFFSET %s",
        *args, limit, offset,
    )


@bp.route("/audit")
@require("admin.access", "audit.view")
def audit():
    page = max(1, request.args.get("page", 1, type=int))
    rows = _audit_rows(limit=PAGE_SIZE + 1, offset=(page - 1) * PAGE_SIZE)
    return render_template("admin/audit.html", rows=rows[:PAGE_SIZE], page=page, more=len(rows) > PAGE_SIZE)

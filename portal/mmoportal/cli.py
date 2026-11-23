"""Commands for the people running the portal:

    flask --app wsgi init-db              create or update the portal's tables
    flask --app wsgi create-admin NAME    make an account an Administrator
    flask --app wsgi check                test the settings and database
    flask --app wsgi lift-expired-bans    for cron, if the portal is quiet

install.sh runs the first three.
"""
import getpass
import os
import re
import sys

import click

from . import accounts, auth_state, db, passwords
from .permissions import ADMINISTRATOR_ROLE

SCHEMA_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "schema")


def _statements(text):
    # Schema files hold plain statements ending in ";" (no procedures, and
    # no ";" inside strings or comments).
    text = "\n".join(line for line in text.splitlines() if not line.strip().startswith("--"))
    return [s.strip() for s in text.split(";") if s.strip()]


def apply_schema():
    """Applies schema/NNN_*.sql files newer than the database's version."""
    have = 0
    try:
        row = db.one("SELECT MAX(version) AS v FROM {portal}.schema_version")
        have = row["v"] or 0
    except Exception:  # noqa: BLE001 - the table does not exist yet
        have = 0
    applied = []
    for name in sorted(os.listdir(SCHEMA_DIR)):
        m = re.match(r"^(\d+)_.*\.sql$", name)
        if not m or int(m.group(1)) <= have:
            continue
        with open(os.path.join(SCHEMA_DIR, name), encoding="utf-8") as f:
            for statement in _statements(f.read()):
                db.run(statement)
        db.run("INSERT IGNORE INTO {portal}.schema_version (version) VALUES (%s)", int(m.group(1)))
        applied.append(name)
    return applied


def register(app):
    @app.cli.command("init-db")
    def init_db():
        """Create or update the portal's tables."""
        applied = apply_schema()
        click.echo("Applied: " + ", ".join(applied) if applied else "The portal's tables are up to date.")

    @app.cli.command("create-admin")
    @click.argument("username")
    def create_admin(username):
        """Give an account the Administrator role, creating it if needed."""
        if not re.match(auth_state.USERNAME_RULE, username):
            sys.exit("error: usernames are 3 to 32 letters, digits or underscores")
        account = auth_state.find_account(username=username)
        if account is None:
            click.echo(f"There is no account '{username}'; creating it.")
            if sys.stdin.isatty():
                password = getpass.getpass(f"Password for {username}: ")
                if password != getpass.getpass("Repeat password: "):
                    sys.exit("error: the passwords do not match")
            else:
                password = sys.stdin.readline().rstrip("\n")
            problem = passwords.problem(password, username)
            if problem:
                sys.exit("error: " + problem)
            account_id = accounts.create(username, password, "")
        else:
            account_id = account["account_id"]
        db.run(
            "INSERT IGNORE INTO {portal}.account_role (account_id, role_id) VALUES (%s, %s)",
            account_id, ADMINISTRATOR_ROLE,
        )
        db.run(
            "INSERT INTO {portal}.audit_log (target_account_id, action, detail) VALUES (%s, 'cli.create_admin', '')",
            account_id,
        )
        click.echo(f"{username} is an Administrator. Sign in to the portal with the game account's password.")

    @app.cli.command("check")
    def check():
        """Test the settings and the database connection."""
        problems = 0
        try:
            row = db.one("SELECT COUNT(*) AS n FROM {game}.account")
            click.echo(f"ok    game database: {row['n']} accounts")
        except Exception as e:  # noqa: BLE001
            click.echo(f"FAIL  game database: {e}")
            return sys.exit(1)
        col = db.one(
            "SELECT CHARACTER_MAXIMUM_LENGTH AS n FROM information_schema.COLUMNS "
            "WHERE TABLE_SCHEMA = %s AND TABLE_NAME = 'account' AND COLUMN_NAME = 'account_password'",
            app.config["GAME_DB"],
        )
        if col and col["n"] >= 128:
            click.echo("ok    account passwords hold Argon2id hashes (migration 014)")
        else:
            problems += 1
            click.echo("FAIL  account_password is too short for Argon2id: run 'mmoserverdb/setup.sh update'")
        try:
            row = db.one("SELECT MAX(version) AS v FROM {portal}.schema_version")
            click.echo(f"ok    portal database: schema version {row['v']}")
        except Exception as e:  # noqa: BLE001
            problems += 1
            click.echo(f"FAIL  portal database: {e} (run 'flask --app wsgi init-db')")
        admins = db.one("SELECT COUNT(*) AS n FROM {portal}.account_role WHERE role_id = %s", ADMINISTRATOR_ROLE) if not problems else None
        if admins is not None:
            if admins["n"]:
                click.echo(f"ok    {admins['n']} Administrator(s)")
            else:
                click.echo("warn  no Administrator yet: run 'flask --app wsgi create-admin NAME'")
        click.echo("ok    email: " + (app.config["SMTP_HOST"] or "not set up; reset links go to the log"))
        sys.exit(1 if problems else 0)

    @app.cli.command("lift-expired-bans")
    def lift_expired_bans():
        """Unban accounts whose temporary ban has ended."""
        click.echo(f"{accounts.lift_expired_bans()} ban(s) lifted.")

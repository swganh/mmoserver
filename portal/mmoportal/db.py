"""Database access: one PyMySQL connection per request.

Game tables are written as {game}.table and portal tables as {portal}.table
in the SQL; q() fills in the configured database names (checked to be plain
identifiers when the portal starts). Values are always bound as parameters.
"""
import pymysql
import pymysql.cursors
from flask import current_app, g


def connect(config):
    where = {"unix_socket": config["DB_SOCKET"]} if config.get("DB_SOCKET") else {
        "host": config["DB_HOST"], "port": config["DB_PORT"]}
    return pymysql.connect(
        **where,
        user=config["DB_USER"],
        password=config["DB_PASSWORD"],
        charset="utf8mb4",
        cursorclass=pymysql.cursors.DictCursor,
        autocommit=True,
    )


def get():
    if "db" not in g:
        g.db = connect(current_app.config)
    return g.db


def close(_error=None):
    db = g.pop("db", None)
    if db is not None:
        db.close()


def q(sql):
    """Fills in {game} and {portal} with the configured database names."""
    cfg = current_app.config
    return sql.replace("{game}", f"`{cfg['GAME_DB']}`").replace("{portal}", f"`{cfg['PORTAL_DB']}`")


def one(sql, *args):
    with get().cursor() as cur:
        cur.execute(q(sql), args or None)
        return cur.fetchone()


def all(sql, *args):  # noqa: A001 - reads well at call sites: db.all(...)
    with get().cursor() as cur:
        cur.execute(q(sql), args or None)
        return cur.fetchall()


def run(sql, *args):
    """Executes a statement; returns (rows affected, last insert id)."""
    with get().cursor() as cur:
        cur.execute(q(sql), args or None)
        return cur.rowcount, cur.lastrowid


class transaction:
    """with db.transaction(): ... commits at the end, rolls back on error."""

    def __enter__(self):
        get().begin()

    def __exit__(self, exc_type, _exc, _tb):
        if exc_type is None:
            get().commit()
        else:
            get().rollback()
        return False

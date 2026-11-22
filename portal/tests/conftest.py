"""Tests run against a real MariaDB/MySQL server, in two scratch databases
(portal_test_game, portal_test_portal) that are dropped and made again.

Connection: PORTAL_TEST_DB_HOST (default 127.0.0.1), PORTAL_TEST_DB_PORT,
PORTAL_TEST_DB_USER (default root), PORTAL_TEST_DB_PASSWORD; or
PORTAL_TEST_DB_SOCKET to use a socket. Without a reachable server the tests
that need one are skipped.
"""
import os
import re
import sys

import pymysql
import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

GAME = "portal_test_game"
PORTAL = "portal_test_portal"

# swganh.account as mmoserverdb/swganh/scripts/account.sql makes it, with
# migration 014, and the characters columns the portal reads.
GAME_TABLES = f"""
CREATE TABLE {GAME}.account (
  account_id bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  account_username char(32) NOT NULL DEFAULT '',
  account_password varchar(128) NOT NULL,
  account_station_id bigint(20) unsigned NOT NULL DEFAULT '0',
  account_csr int(1) NOT NULL DEFAULT '0',
  account_banned tinyint(1) NOT NULL DEFAULT '0',
  account_email char(64) NOT NULL DEFAULT '',
  account_joindate datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  account_lastlogin timestamp NULL DEFAULT NULL,
  account_active tinyint(1) DEFAULT NULL,
  account_loggedin int(1) NOT NULL DEFAULT '0',
  account_authenticated tinyint(1) NOT NULL DEFAULT '0',
  account_characters_allowed tinyint(3) unsigned NOT NULL DEFAULT '2',
  account_session_key varchar(32) DEFAULT NULL,
  account_lastcreate datetime DEFAULT NULL,
  banned_until datetime DEFAULT NULL,
  banned_date datetime DEFAULT NULL,
  PRIMARY KEY (account_id),
  UNIQUE KEY account_username (account_username)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE {GAME}.characters (
  id bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  account_id bigint(20) unsigned NOT NULL DEFAULT '0',
  firstname char(32) DEFAULT NULL,
  lastname char(32) DEFAULT NULL,
  archived tinyint(1) unsigned DEFAULT '0',
  PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE {GAME}.skills (
  skill_id int(10) unsigned NOT NULL,
  skill_name char(255) NOT NULL,
  PRIMARY KEY (skill_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
INSERT INTO {GAME}.skills VALUES (1, 'combat_brawler_novice'), (1069, 'admin_skill'),
  (1070, 'admin_skill_novice'), (1071, 'admin_skill_01_01'), (1076, 'admin_skill_02_01'),
  (1080, 'admin_skill_03_01');
CREATE TABLE {GAME}.character_skills (
  character_id bigint(20) unsigned NOT NULL DEFAULT 0,
  skill_id int(10) unsigned NOT NULL,
  PRIMARY KEY (character_id, skill_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE {GAME}.command_table (
  id int(10) unsigned NOT NULL,
  commandname char(255) NOT NULL,
  characterability char(255) NOT NULL,
  PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
INSERT INTO {GAME}.command_table VALUES (1, 'broadcast', 'admin_broadcast'), (2, 'goto', 'admin'),
  (3, 'kneel', '')
"""


def _connect():
    kwargs = dict(
        user=os.environ.get("PORTAL_TEST_DB_USER", "root"),
        password=os.environ.get("PORTAL_TEST_DB_PASSWORD", ""),
        autocommit=True,
        cursorclass=pymysql.cursors.DictCursor,
    )
    if os.environ.get("PORTAL_TEST_DB_SOCKET"):
        kwargs["unix_socket"] = os.environ["PORTAL_TEST_DB_SOCKET"]
    else:
        kwargs["host"] = os.environ.get("PORTAL_TEST_DB_HOST", "127.0.0.1")
        kwargs["port"] = int(os.environ.get("PORTAL_TEST_DB_PORT", "3306"))
    return pymysql.connect(**kwargs), kwargs


@pytest.fixture(scope="session")
def server():
    try:
        conn, kwargs = _connect()
    except pymysql.err.MySQLError as e:
        pytest.skip(f"no test database server: {e}")
    return conn, kwargs


@pytest.fixture
def app(server, tmp_path):
    conn, kwargs = server
    with conn.cursor() as cur:
        for db in (GAME, PORTAL):
            cur.execute(f"DROP DATABASE IF EXISTS {db}")
            cur.execute(f"CREATE DATABASE {db}")
        for statement in GAME_TABLES.split(";"):
            cur.execute(statement)

    cfg = tmp_path / "portal.cfg"
    cfg.write_text(
        "[portal]\nsite_name = Test Portal\nbase_url = http://portal.test\n"
        f"secret_key = {'x' * 40}\nregistration = open\n"
        "[database]\n"
        f"host = {kwargs.get('host', '127.0.0.1')}\nport = {kwargs.get('port', 3306)}\n"
        f"socket = {kwargs.get('unix_socket', '')}\n"
        f"user = {kwargs['user']}\npassword = {kwargs['password']}\n"
        f"game_database = {GAME}\nportal_database = {PORTAL}\n"
        "[mail]\nsmtp_host =\n"
    )
    from mmoportal import create_app

    application = create_app(str(cfg))
    application.config["TESTING"] = True
    with application.app_context():
        from mmoportal.cli import apply_schema

        apply_schema()
    return application


@pytest.fixture
def sql(server):
    conn, _ = server

    def run(statement, *args):
        with conn.cursor() as cur:
            cur.execute(statement.replace("{game}", GAME).replace("{portal}", PORTAL), args or None)
            return cur.fetchall()

    return run


class Browser:
    """A test client that keeps the CSRF token from the last page."""

    def __init__(self, client):
        self.client = client
        self.token = ""

    def get(self, url, **kw):
        r = self.client.get(url, **kw)
        m = re.search(rb'name="csrf_token" value="([^"]+)"', r.data)
        if m:
            self.token = m.group(1).decode()
        return r

    def post(self, url, data=None, follow=False, **kw):
        if not self.token:
            self.get("/login")
        data = dict(data or {})
        data.setdefault("csrf_token", self.token)
        r = self.client.post(url, data=data, follow_redirects=follow, **kw)
        m = re.search(rb'name="csrf_token" value="([^"]+)"', r.data)
        if m:
            self.token = m.group(1).decode()
        return r

    def login(self, username, password):
        self.get("/login")
        return self.post("/login", {"username": username, "password": password})


@pytest.fixture
def browser(app):
    return Browser(app.test_client())


@pytest.fixture
def make_account(app):
    from mmoportal import accounts

    def make(username, password="password123", email="", roles=()):
        with app.app_context():
            account_id = accounts.create(username, password, email)
            from mmoportal import db

            for role in roles:
                db.run("INSERT INTO {portal}.account_role (account_id, role_id) VALUES (%s, %s)", account_id, role)
        return account_id

    return make

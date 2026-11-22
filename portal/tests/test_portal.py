import hashlib
import re

import pytest

ADMIN, CSR = 1, 2


def account(sql, name):
    rows = sql("SELECT * FROM {game}.account WHERE account_username = %s", name)
    return rows[0] if rows else None


# --- Signing in -------------------------------------------------------------------

def test_login_and_logout(browser, make_account):
    make_account("player1")
    r = browser.login("player1", "password123")
    assert r.status_code == 302 and r.headers["Location"].endswith("/account/")
    assert b"player1" in browser.get("/account/").data
    browser.post("/logout")
    assert browser.get("/account/").status_code == 302


def test_wrong_password_and_unknown_user_look_the_same(browser, make_account):
    make_account("player1")
    a = browser.login("player1", "nope-nope")
    b = browser.login("nobody", "nope-nope")
    assert a.status_code == b.status_code == 401
    assert b"Wrong username or password." in a.data and b"Wrong username or password." in b.data


def test_post_without_csrf_token_is_refused(app, make_account):
    make_account("player1")
    r = app.test_client().post("/login", data={"username": "player1", "password": "password123"})
    assert r.status_code == 400


def test_login_rate_limit(browser, make_account):
    make_account("player1")
    for _ in range(8):
        browser.login("player1", "bad-password")
    assert browser.login("player1", "password123").status_code == 429


def test_legacy_sha1_password_is_upgraded_at_login(browser, sql, make_account):
    make_account("oldtimer")
    sql("UPDATE {game}.account SET account_password = SHA1('legacy-pass') WHERE account_username = 'oldtimer'")
    assert browser.login("oldtimer", "legacy-pass").status_code == 302
    assert account(sql, "oldtimer")["account_password"].startswith("$argon2id$")
    # Still signed in after the upgrade.
    assert browser.get("/account/").status_code == 200


def test_banned_account_cannot_sign_in(browser, sql, make_account):
    make_account("baddie")
    sql("UPDATE {game}.account SET account_banned = 1 WHERE account_username = 'baddie'")
    assert browser.login("baddie", "password123").status_code == 403


def test_open_redirect_is_ignored(browser, make_account):
    make_account("player1")
    browser.get("/login")
    r = browser.post("/login?next=//evil.example/x", {"username": "player1", "password": "password123"})
    assert r.headers["Location"].endswith("/account/")


# --- Registration -------------------------------------------------------------------

def test_register_creates_a_game_account(browser, sql):
    browser.get("/register")
    r = browser.post("/register", {"username": "newbie", "email": "n@example.com",
                                   "password": "password123", "confirm": "password123"})
    assert r.status_code == 302
    acc = account(sql, "newbie")
    assert acc["account_password"].startswith("$argon2id$")
    assert acc["account_station_id"] == 100000000 + acc["account_id"]
    assert acc["account_active"] == 1


def test_register_refuses_taken_name_and_email(browser, make_account):
    make_account("taken", email="t@example.com")
    browser.get("/register")
    r = browser.post("/register", {"username": "TAKEN", "email": "x@example.com",
                                   "password": "password123", "confirm": "password123"})
    assert r.status_code == 400
    r = browser.post("/register", {"username": "other", "email": "T@example.com",
                                   "password": "password123", "confirm": "password123"})
    assert r.status_code == 400


def test_register_closed(app, browser):
    app.config["REGISTRATION_OPEN"] = False
    assert browser.get("/register").status_code == 404


# --- Own account ----------------------------------------------------------------------

def test_change_password_signs_out_other_sessions(app, browser, make_account, sql):
    from conftest import Browser

    make_account("player1")
    other = Browser(app.test_client())
    other.login("player1", "password123")
    browser.login("player1", "password123")
    browser.get("/account/password")
    r = browser.post("/account/password", {"current": "password123", "password": "newpassword1", "confirm": "newpassword1"})
    assert r.status_code == 302
    assert browser.get("/account/").status_code == 200      # this one stays
    assert other.get("/account/").status_code == 302        # the other is out
    assert browser.login("player1", "newpassword1").status_code == 302


def test_change_password_needs_current(browser, make_account):
    make_account("player1")
    browser.login("player1", "password123")
    browser.get("/account/password")
    r = browser.post("/account/password", {"current": "wrong-one", "password": "newpassword1", "confirm": "newpassword1"})
    assert r.status_code == 400


def test_change_email(browser, make_account, sql):
    make_account("player1")
    browser.login("player1", "password123")
    browser.get("/account/email")
    assert browser.post("/account/email", {"email": "me@example.com", "current": "password123"}).status_code == 302
    assert account(sql, "player1")["account_email"] == "me@example.com"


# --- Password reset ---------------------------------------------------------------------

def _reset_link(caplog):
    m = re.search(r"http://portal\.test(/reset/[A-Za-z0-9_-]+)", caplog.text)
    return m.group(1) if m else None


def test_forgot_password_flow(browser, make_account, caplog, sql):
    make_account("player1", email="p@example.com")
    browser.get("/forgot")
    caplog.set_level("WARNING", logger="mmoportal")
    browser.post("/forgot", {"username": "player1"})
    link = _reset_link(caplog)
    assert link
    assert browser.get(link).status_code == 200
    r = browser.post(link, {"password": "brandnew123", "confirm": "brandnew123"})
    assert r.status_code == 302
    assert browser.login("player1", "brandnew123").status_code == 302
    # The link works once.
    assert browser.get(link).status_code == 302


def test_forgot_password_unknown_account_answers_the_same(browser, caplog):
    browser.get("/forgot")
    r = browser.post("/forgot", {"username": "nobody"}, follow=True)
    assert b"If that account has an email address" in r.data
    assert _reset_link(caplog) is None


def test_expired_reset_link(browser, make_account, caplog, sql):
    make_account("player1", email="p@example.com")
    caplog.set_level("WARNING", logger="mmoportal")
    browser.get("/forgot")
    browser.post("/forgot", {"username": "player1"})
    sql("UPDATE {portal}.password_reset SET expires_at = NOW() - INTERVAL 1 MINUTE")
    assert browser.get(_reset_link(caplog)).status_code == 302


def test_reset_token_is_stored_hashed(browser, make_account, caplog, sql):
    make_account("player1", email="p@example.com")
    caplog.set_level("WARNING", logger="mmoportal")
    browser.get("/forgot")
    browser.post("/forgot", {"username": "player1"})
    token = _reset_link(caplog).rsplit("/", 1)[1]
    stored = sql("SELECT token_hash FROM {portal}.password_reset")[0]["token_hash"]
    assert stored == hashlib.sha256(token.encode()).hexdigest()


# --- Admin area -------------------------------------------------------------------------

def test_players_cannot_open_admin(browser, make_account):
    make_account("player1")
    browser.login("player1", "password123")
    assert browser.get("/admin/").status_code == 403
    assert browser.get("/admin/accounts").status_code == 403


def test_admin_finds_and_edits_accounts(browser, make_account, sql):
    make_account("boss", roles=[ADMIN])
    target = make_account("player1")
    browser.login("boss", "password123")
    assert b"player1" in browser.get("/admin/accounts?q=play").data
    browser.get(f"/admin/accounts/{target}")
    browser.post(f"/admin/accounts/{target}/edit", {"email": "x@example.com", "characters_allowed": "5", "active": "1"})
    acc = account(sql, "player1")
    assert (acc["account_email"], acc["account_characters_allowed"]) == ("x@example.com", 5)
    assert sql("SELECT action FROM {portal}.audit_log WHERE target_account_id = %s", target)[-1]["action"] == "admin.account_edit"


def test_admin_ban_and_unban(browser, make_account, sql):
    make_account("boss", roles=[ADMIN])
    target = make_account("player1")
    browser.login("boss", "password123")
    browser.get(f"/admin/accounts/{target}")
    browser.post(f"/admin/accounts/{target}/ban", {"reason": "spam", "days": "3"})
    acc = account(sql, "player1")
    assert acc["account_banned"] == 1 and acc["banned_until"] is not None
    browser.post(f"/admin/accounts/{target}/unban")
    assert account(sql, "player1")["account_banned"] == 0


def test_expired_bans_are_lifted(app, make_account, sql):
    target = make_account("player1")
    sql("UPDATE {game}.account SET account_banned = 1, banned_until = NOW() - INTERVAL 1 HOUR WHERE account_id = %s", target)
    with app.app_context():
        from mmoportal.accounts import lift_expired_bans

        assert lift_expired_bans() == 1
    assert account(sql, "player1")["account_banned"] == 0


def test_admin_sets_password(browser, make_account):
    make_account("boss", roles=[ADMIN])
    target = make_account("player1")
    browser.login("boss", "password123")
    browser.get(f"/admin/accounts/{target}")
    browser.post(f"/admin/accounts/{target}/password", {"password": "setbystaff1"})
    from conftest import Browser

    other = Browser(browser.client.application.test_client())
    assert other.login("player1", "setbystaff1").status_code == 302


def test_admin_creates_account(browser, make_account, sql):
    make_account("boss", roles=[ADMIN])
    browser.login("boss", "password123")
    browser.get("/admin/accounts/new")
    assert browser.post("/admin/accounts/new", {"username": "fresh", "email": "", "password": "password123"}).status_code == 302
    assert account(sql, "fresh") is not None


def test_csr_permissions(browser, make_account, sql):
    make_account("helper", roles=[CSR])
    target = make_account("player1")
    browser.login("helper", "password123")
    assert browser.get(f"/admin/accounts/{target}").status_code == 200
    browser.post(f"/admin/accounts/{target}/ban", {"reason": "x", "days": ""})
    assert account(sql, "player1")["account_banned"] == 1
    # Not in the CSR role: editing, the in-game CSR flag, roles.
    assert browser.post(f"/admin/accounts/{target}/edit", {"email": "", "characters_allowed": "9"}).status_code == 403
    assert browser.post(f"/admin/accounts/{target}/csr", {"csr": "1"}).status_code == 403
    assert browser.post(f"/admin/accounts/{target}/roles", {"role": "1"}).status_code == 403
    assert browser.get("/admin/roles").status_code == 403


def test_csr_cannot_touch_staff_accounts(browser, make_account, sql):
    make_account("helper", roles=[CSR])
    boss = make_account("boss", roles=[ADMIN])
    browser.login("helper", "password123")
    browser.get(f"/admin/accounts/{boss}")
    assert browser.post(f"/admin/accounts/{boss}/password", {"password": "takeover123"}).status_code == 403
    assert browser.post(f"/admin/accounts/{boss}/ban", {"reason": "x"}).status_code == 403


def test_last_administrator_cannot_be_removed(browser, make_account, sql):
    boss = make_account("boss", roles=[ADMIN])
    browser.login("boss", "password123")
    browser.get(f"/admin/accounts/{boss}")
    browser.post(f"/admin/accounts/{boss}/roles", {})
    assert sql("SELECT * FROM {portal}.account_role WHERE account_id = %s", boss)


def test_roles_can_be_made_and_given(browser, make_account, sql):
    make_account("boss", roles=[ADMIN])
    target = make_account("qa1")
    browser.login("boss", "password123")
    browser.get("/admin/roles/new")
    browser.post("/admin/roles/new", {"name": "QA", "description": "testers",
                                      "permission": ["admin.access", "accounts.view", "made.up"]})
    role = sql("SELECT role_id FROM {portal}.role WHERE name = 'QA'")[0]["role_id"]
    perms = {r["permission"] for r in sql("SELECT permission FROM {portal}.role_permission WHERE role_id = %s", role)}
    assert perms == {"admin.access", "accounts.view"}
    browser.get(f"/admin/accounts/{target}")
    browser.post(f"/admin/accounts/{target}/roles", {"role": [str(role)]})

    from conftest import Browser

    qa = Browser(browser.client.application.test_client())
    qa.login("qa1", "password123")
    assert qa.get("/admin/accounts").status_code == 200
    assert qa.get("/admin/audit").status_code == 403


def test_administrator_role_cannot_be_edited_or_deleted(browser, make_account):
    make_account("boss", roles=[ADMIN])
    browser.login("boss", "password123")
    assert browser.get("/admin/roles/1").status_code == 400
    browser.get("/admin/roles")
    assert browser.post("/admin/roles/1/delete").status_code == 400


def test_create_admin_cli(app, make_account, sql):
    make_account("someone")
    result = app.test_cli_runner().invoke(args=["create-admin", "someone"])
    assert result.exit_code == 0, result.output
    acc = account(sql, "someone")
    assert sql("SELECT role_id FROM {portal}.account_role WHERE account_id = %s", acc["account_id"])[0]["role_id"] == ADMIN


def test_pages_render(browser, make_account):
    boss = make_account("boss", roles=[ADMIN])
    browser.login("boss", "password123")
    for url in ("/account/", "/account/password", "/account/email", "/admin/", "/admin/accounts",
                f"/admin/accounts/{boss}", "/admin/accounts/new", "/admin/csr", "/admin/roles", "/admin/roles/new",
                "/admin/roles/2", "/admin/audit"):
        assert browser.get(url).status_code == 200, url


# --- In-game admin commands ------------------------------------------------------------

def _character(sql, account_id, name="Kyla"):
    sql("INSERT INTO {game}.characters (account_id, firstname) VALUES (%s, %s)", account_id, name)
    return sql("SELECT MAX(id) AS id FROM {game}.characters")[0]["id"]


def _skills(sql, character_id):
    return {r["skill_id"] for r in sql("SELECT skill_id FROM {game}.character_skills WHERE character_id = %s", character_id)}


def test_csr_page_lists_commands(browser, make_account, sql):
    boss = make_account("boss", roles=[ADMIN])
    sql("UPDATE {game}.account SET account_csr = 1 WHERE account_id = %s", boss)
    browser.login("boss", "password123")
    page = browser.get("/admin/csr").data
    assert b"/broadcastGalaxy" in page and b"/admin_warp_self" in page
    assert b"/goto" in page          # known to the client only
    assert b"/kneel" not in page     # not an admin command
    assert b">boss</a>" in page      # has the CSR flag


def test_admin_gives_and_takes_admin_commands(browser, make_account, sql):
    make_account("boss", roles=[ADMIN])
    player = make_account("player1")
    char = _character(sql, player)
    sql("INSERT INTO {game}.character_skills VALUES (%s, 1)", char)   # an ordinary skill stays
    browser.login("boss", "password123")
    browser.get(f"/admin/accounts/{player}")
    # Shutdown alone also brings the novice skill it builds on.
    browser.post(f"/admin/characters/{char}/admin-skills", {"skill": ["admin_skill_02_01", "made_up"]})
    assert _skills(sql, char) == {1, 1070, 1076}
    assert b"Galaxy shutdown" in browser.get(f"/admin/accounts/{player}").data
    assert b"Kyla" in browser.get("/admin/csr").data
    browser.post(f"/admin/characters/{char}/admin-skills", {})
    assert _skills(sql, char) == {1}
    actions = [r["action"] for r in sql("SELECT action FROM {portal}.audit_log WHERE target_account_id = %s", player)]
    assert actions.count("admin.game_admin_skills") == 2


def test_csr_role_cannot_give_admin_commands(browser, make_account, sql):
    make_account("helper", roles=[CSR])
    player = make_account("player1")
    char = _character(sql, player)
    browser.login("helper", "password123")
    assert browser.get("/admin/csr").status_code == 200
    browser.get(f"/admin/accounts/{player}")
    assert browser.post(f"/admin/characters/{char}/admin-skills", {"skill": ["admin_skill_novice"]}).status_code == 403
    assert _skills(sql, char) == set()

"""Reads portal.cfg into Flask's config."""
import configparser
import os
import re

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_PATH = os.path.join(HERE, "portal.cfg")

_IDENTIFIER = re.compile(r"^[A-Za-z0-9_]{1,64}$")


class ConfigError(Exception):
    pass


def _bool(value):
    return str(value).strip().lower() in ("1", "true", "yes", "on")


def load(path=None):
    """Returns the settings as a dict of Flask config keys."""
    path = path or os.environ.get("PORTAL_CONFIG") or DEFAULT_PATH
    parser = configparser.ConfigParser(interpolation=None)
    if not parser.read(path):
        raise ConfigError(f"{path} not found; run install.sh, or copy portal.cfg.example")

    p = parser["portal"] if parser.has_section("portal") else {}
    d = parser["database"] if parser.has_section("database") else {}
    m = parser["mail"] if parser.has_section("mail") else {}

    return {
        "SITE_NAME": p.get("site_name", "MMOServer Portal"),
        "BASE_URL": p.get("base_url", "http://127.0.0.1:8080").rstrip("/"),
        "SECRET_KEY": p.get("secret_key", ""),
        "REGISTRATION_OPEN": p.get("registration", "closed").strip().lower() == "open",
        "SESSION_COOKIE_SECURE": _bool(p.get("secure_cookies", "false")),
        "BEHIND_PROXY": _bool(p.get("behind_proxy", "false")),
        "RESET_LINK_MINUTES": int(p.get("reset_link_minutes", "60")),
        "DB_HOST": d.get("host", "127.0.0.1"),
        "DB_PORT": int(d.get("port", "3306")),
        "DB_SOCKET": d.get("socket", ""),
        "DB_USER": d.get("user", "swganh_portal"),
        "DB_PASSWORD": d.get("password", ""),
        "GAME_DB": d.get("game_database", "swganh"),
        "PORTAL_DB": d.get("portal_database", "swganh_portal"),
        "SMTP_HOST": m.get("smtp_host", ""),
        "SMTP_PORT": int(m.get("smtp_port", "587")),
        "SMTP_SECURITY": m.get("security", "starttls").strip().lower(),
        "SMTP_USER": m.get("user", ""),
        "SMTP_PASSWORD": m.get("password", ""),
        "MAIL_FROM": m.get("from_address", "portal@example.com"),
    }


def validate(config):
    """Refuses settings the portal cannot run safely with."""
    if len(config.get("SECRET_KEY") or "") < 32:
        raise ConfigError("secret_key in portal.cfg must be at least 32 characters (install.sh generates one)")
    # Database names go into SQL as identifiers, so only plain names.
    for key in ("GAME_DB", "PORTAL_DB"):
        if not _IDENTIFIER.match(config.get(key, "")):
            raise ConfigError(f"{key.lower()} must be letters, digits and underscores")
    if config.get("SMTP_SECURITY") not in ("none", "starttls", "ssl"):
        raise ConfigError("mail security must be none, starttls or ssl")

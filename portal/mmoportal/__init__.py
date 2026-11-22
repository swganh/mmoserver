"""MMOServer portal: accounts for players, and an admin area for staff.

A Flask application kept apart from the game servers: it only shares the
game database's account table (and reads characters). See portal/README.md.
"""
import logging
import time
from datetime import timedelta

from flask import Flask, g, render_template
from werkzeug.middleware.proxy_fix import ProxyFix

from . import auth_state, config, db, permissions


def create_app(config_path=None, overrides=None):
    app = Flask(__name__)
    app.config.update(config.load(config_path))
    if overrides:
        app.config.update(overrides)
    config.validate(app.config)

    app.config.update(
        SESSION_COOKIE_HTTPONLY=True,
        SESSION_COOKIE_SAMESITE="Lax",
        SESSION_COOKIE_NAME="mmoportal",
        PERMANENT_SESSION_LIFETIME=timedelta(days=7),
        MAX_CONTENT_LENGTH=64 * 1024,
    )
    if app.config["BEHIND_PROXY"]:
        app.wsgi_app = ProxyFix(app.wsgi_app, x_for=1, x_proto=1, x_host=1)

    if not app.logger.handlers or app.debug:
        logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(name)s: %(message)s")

    app.teardown_appcontext(db.close)
    app.before_request(auth_state.check_csrf)
    app.before_request(auth_state.load_user)

    last_ban_sweep = [0.0]

    @app.before_request
    def lift_expired_bans():
        if time.monotonic() - last_ban_sweep[0] > 300:
            last_ban_sweep[0] = time.monotonic()
            from .accounts import lift_expired_bans as lift

            lift()

    @app.after_request
    def security_headers(response):
        response.headers.setdefault(
            "Content-Security-Policy",
            "default-src 'self'; style-src 'self'; img-src 'self' data:; form-action 'self'; frame-ancestors 'none'",
        )
        response.headers.setdefault("X-Content-Type-Options", "nosniff")
        response.headers.setdefault("Referrer-Policy", "same-origin")
        response.headers.setdefault("X-Frame-Options", "DENY")
        if g.get("user"):
            response.headers.setdefault("Cache-Control", "no-store")
        return response

    @app.context_processor
    def template_helpers():
        return {
            "csrf_token": auth_state.csrf_token,
            "has_permission": permissions.has,
            "is_admin": permissions.is_admin,
            "site_name": app.config["SITE_NAME"],
            "registration_open": app.config["REGISTRATION_OPEN"],
        }

    for code, title in ((400, "Bad request"), (403, "Not allowed"), (404, "Not found")):
        app.register_error_handler(code, _error_page(code, title))

    from .views import account, admin, auth

    app.register_blueprint(auth.bp)
    app.register_blueprint(account.bp)
    app.register_blueprint(admin.bp)

    from . import cli

    cli.register(app)
    return app


def _error_page(code, title):
    def handler(error):
        message = getattr(error, "description", "") or ""
        return render_template("error.html", code=code, title=title, message=message), code

    return handler

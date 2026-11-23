"""Sign in and out, registration, and password resets."""
import re
from urllib.parse import urlparse

from flask import Blueprint, abort, current_app, flash, g, redirect, render_template, request, url_for

from .. import accounts, auth_state, passwords
from ..auth_state import USERNAME_RULE

bp = Blueprint("auth", __name__)

EMAIL_RULE = re.compile(r"^[^@\s]{1,64}@[^@\s]+\.[^@\s]+$")


def _safe_next(target):
    """Only redirect to a path on this site after signing in."""
    if not target:
        return None
    parts = urlparse(target)
    if parts.scheme or parts.netloc or not target.startswith("/") or target.startswith("//"):
        return None
    return target


@bp.route("/")
def index():
    if g.user:
        return redirect(url_for("account.overview"))
    return redirect(url_for("auth.login"))


@bp.route("/login", methods=["GET", "POST"])
def login():
    if g.user:
        return redirect(url_for("account.overview"))
    username = request.form.get("username", "").strip()
    if request.method == "POST":
        password = request.form.get("password", "")
        if auth_state.limited("login", username):
            flash("Too many failed attempts. Wait a few minutes and try again.", "error")
            return render_template("auth/login.html", username=username), 429

        account = auth_state.find_account(username=username) if username else None
        right = False
        if account is None:
            passwords.check_nothing(password)
        else:
            right, rehash = passwords.check(account["account_password"], password)

        if not right:
            auth_state.record_attempt("login", username)
            flash("Wrong username or password.", "error")
            return render_template("auth/login.html", username=username), 401

        if auth_state.is_banned(account):
            flash("This account is banned.", "error")
            return render_template("auth/login.html", username=username), 403

        if rehash:
            accounts.store_rehash(account["account_id"], password)
            account = auth_state.find_account(account_id=account["account_id"])
        auth_state.clear_attempts("login", username)
        auth_state.login(account)
        return redirect(_safe_next(request.args.get("next")) or url_for("account.overview"))

    return render_template("auth/login.html", username=username)


@bp.route("/logout", methods=["POST"])
def logout():
    auth_state.logout()
    flash("Signed out.", "info")
    return redirect(url_for("auth.login"))


@bp.route("/register", methods=["GET", "POST"])
def register():
    if not current_app.config["REGISTRATION_OPEN"]:
        abort(404)
    if g.user:
        return redirect(url_for("account.overview"))
    form = {"username": request.form.get("username", "").strip(), "email": request.form.get("email", "").strip()}
    if request.method == "POST":
        password = request.form.get("password", "")
        error = None
        if auth_state.limited("register"):
            error = "Too many accounts were made from your address. Try again later."
        elif not re.match(USERNAME_RULE, form["username"]):
            error = "Usernames are 3 to 32 letters, digits or underscores."
        elif not EMAIL_RULE.match(form["email"]) or len(form["email"]) > 64:
            error = "Enter a valid email address; it is used to reset your password."
        elif password != request.form.get("confirm", ""):
            error = "The passwords do not match."
        else:
            error = passwords.problem(password, form["username"])
        if not error and accounts.username_taken(form["username"]):
            error = "That username is taken."
        if not error and accounts.email_taken(form["email"]):
            error = "An account already uses that email address. Use “Forgot password” to get into it."
        if error:
            flash(error, "error")
            return render_template("auth/register.html", form=form), 400

        account_id = accounts.create(form["username"], password, form["email"])
        auth_state.record_attempt("register")
        account = auth_state.find_account(account_id=account_id)
        auth_state.login(account)
        g.user = account
        auth_state.audit("account.register", account_id)
        flash("Your account is ready. Use the same username and password in the game.", "success")
        return redirect(url_for("account.overview"))

    return render_template("auth/register.html", form=form)


@bp.route("/forgot", methods=["GET", "POST"])
def forgot():
    if request.method == "POST":
        name = request.form.get("username", "").strip()
        if not auth_state.limited("reset", name):
            auth_state.record_attempt("reset", name)
            account = auth_state.find_account(username=name) if name else None
            if account and not auth_state.is_banned(account):
                accounts.send_reset_link(account)
        # The same answer whether or not the account exists or has an email.
        flash(
            "If that account has an email address, a link to reset its password is on its way. "
            "No email? Ask a staff member to reset it.",
            "info",
        )
        return redirect(url_for("auth.login"))
    return render_template("auth/forgot.html")


@bp.route("/reset/<token>", methods=["GET", "POST"])
def reset(token):
    account_id = accounts.account_for_token(token)
    account = auth_state.find_account(account_id=account_id) if account_id else None
    if account is None:
        flash("That reset link is not valid any more. Ask for a new one.", "error")
        return redirect(url_for("auth.forgot"))

    if request.method == "POST":
        password = request.form.get("password", "")
        error = passwords.problem(password, account["account_username"])
        if not error and password != request.form.get("confirm", ""):
            error = "The passwords do not match."
        if error:
            flash(error, "error")
            return render_template("auth/reset.html", account=account), 400
        accounts.set_password(account["account_id"], password)
        auth_state.logout()
        g.user = None
        auth_state.audit("account.password_reset", account["account_id"], "with a reset link")
        flash("Password changed. Sign in with the new one, here and in the game.", "success")
        return redirect(url_for("auth.login"))

    return render_template("auth/reset.html", account=account)

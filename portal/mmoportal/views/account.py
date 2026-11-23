"""A player's own account."""
from flask import Blueprint, flash, g, redirect, render_template, request, url_for

from .. import accounts, auth_state, db, passwords
from ..auth_state import login_required
from .auth import EMAIL_RULE

bp = Blueprint("account", __name__, url_prefix="/account")


@bp.route("/")
@login_required
def overview():
    return render_template("account/overview.html", characters=accounts.characters(g.user["account_id"]))


@bp.route("/password", methods=["GET", "POST"])
@login_required
def password():
    if request.method == "POST":
        current = request.form.get("current", "")
        new = request.form.get("password", "")
        right, _ = passwords.check(g.user["account_password"], current)
        error = None if right else "Your current password is wrong."
        if not error:
            error = passwords.problem(new, g.user["account_username"])
        if not error and new != request.form.get("confirm", ""):
            error = "The new passwords do not match."
        if error:
            flash(error, "error")
            return render_template("account/password.html"), 400
        stored = accounts.set_password(g.user["account_id"], new)
        auth_state.refresh_password_tag(stored)
        auth_state.audit("account.password_change", g.user["account_id"])
        flash("Password changed. Use the new one in the game too; other browsers were signed out.", "success")
        return redirect(url_for("account.overview"))
    return render_template("account/password.html")


@bp.route("/email", methods=["GET", "POST"])
@login_required
def email():
    if request.method == "POST":
        address = request.form.get("email", "").strip()
        right, _ = passwords.check(g.user["account_password"], request.form.get("current", ""))
        error = None
        if not right:
            error = "Your password is wrong."
        elif not EMAIL_RULE.match(address) or len(address) > 64:
            error = "Enter a valid email address."
        elif accounts.email_taken(address, g.user["account_id"]):
            error = "Another account uses that email address."
        if error:
            flash(error, "error")
            return render_template("account/email.html", address=address), 400
        old = g.user["account_email"]
        db.run("UPDATE {game}.account SET account_email = %s WHERE account_id = %s", address, g.user["account_id"])
        auth_state.audit("account.email_change", g.user["account_id"], f"{old} -> {address}")
        flash("Email address saved.", "success")
        return redirect(url_for("account.overview"))
    return render_template("account/email.html", address=g.user["account_email"])

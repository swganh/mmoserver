"""Sends email through the SMTP server in portal.cfg. With none set, the
message is written to the log instead, for an admin to pass on."""
import logging
import smtplib
import ssl
from email.message import EmailMessage

from flask import current_app, render_template

log = logging.getLogger("mmoportal")


def send_template(to, subject, template, **context):
    cfg = current_app.config
    body = render_template(template, site_name=cfg["SITE_NAME"], **context)
    subject = f"{cfg['SITE_NAME']}: {subject}"

    if not cfg["SMTP_HOST"]:
        log.warning("No SMTP server configured; email to %s not sent:\n%s\n%s", to, subject, body)
        return

    msg = EmailMessage()
    msg["From"] = cfg["MAIL_FROM"]
    msg["To"] = to
    msg["Subject"] = subject
    msg.set_content(body)

    context = ssl.create_default_context()
    try:
        if cfg["SMTP_SECURITY"] == "ssl":
            server = smtplib.SMTP_SSL(cfg["SMTP_HOST"], cfg["SMTP_PORT"], timeout=15, context=context)
        else:
            server = smtplib.SMTP(cfg["SMTP_HOST"], cfg["SMTP_PORT"], timeout=15)
        with server:
            if cfg["SMTP_SECURITY"] == "starttls":
                server.starttls(context=context)
            if cfg["SMTP_USER"]:
                server.login(cfg["SMTP_USER"], cfg["SMTP_PASSWORD"])
            server.send_message(msg)
    except (OSError, smtplib.SMTPException):
        # The page answers the same either way; the admin sees why here.
        log.exception("Could not send email to %s", to)

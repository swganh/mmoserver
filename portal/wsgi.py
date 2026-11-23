"""Entry point for gunicorn: gunicorn wsgi:app (install.sh sets this up)."""
from mmoportal import create_app

app = create_app()

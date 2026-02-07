import dash
from app import create_app
from config import SETTINGS


def main() -> None:
    app: dash.Dash = create_app()
    app.run(host="0.0.0.0", port=8050, debug=SETTINGS.DEBUG)


if __name__ == "__main__":
    main()

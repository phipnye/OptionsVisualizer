import dash
from app import create_app
from config import SETTINGS


def main() -> None:
    app: dash.Dash = create_app()
    app.run(debug=SETTINGS.DEBUG)


if __name__ == "__main__":
    main()

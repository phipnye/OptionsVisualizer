import dash
from app import create_app
from config import SETTINGS


def main() -> None:
    app: dash.Dash = create_app()
    app.run(debug=SETTINGS.DEBUG, dev_tools_props_check=True)


if __name__ == "__main__":
    main()

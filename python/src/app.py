import dash
import dash_bootstrap_components as dbc
from callbacks import register_callbacks
from config import SETTINGS
from dash_bootstrap_templates import load_figure_template
from panels import create_control_panel, create_heatmap_grid


# Define app layout (left column for control panel, right column for heatmaps)
def create_layout() -> dbc.Container:
    return dbc.Container(
        fluid=True,
        className="vh-100 d-flex flex-column",  # full viewport height + enable vertical flex sizing
        children=[
            dbc.Row(
                className="flex-grow-1",  # allow row to expand vertically
                children=[
                    # Control panel (left column) 25% width on md+
                    dbc.Col(create_control_panel(), md=3, className="h-100"),
                    # Heatmaps (right column) 75% width on md+
                    dbc.Col(create_heatmap_grid(), md=9, className="h-100"),
                ],
            )
        ],
    )


def create_app() -> dash.Dash:
    app: dash.Dash = dash.Dash(__name__, external_stylesheets=[dbc.themes.DARKLY])
    load_figure_template(SETTINGS.PLOT_THEME)
    app.layout = create_layout()
    register_callbacks(app)
    return app

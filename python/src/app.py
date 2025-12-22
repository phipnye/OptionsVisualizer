import dash
import dash_bootstrap_components as dbc
from callbacks import register_callbacks
from config import SETTINGS
from dash_bootstrap_templates import load_figure_template
from panels import create_control_panel, create_heatmap_grid


def create_layout() -> dbc.Container:
    return dbc.Container(
        fluid=True,
        className="vh-100 d-flex flex-column",  # vh-100 and flex-column ensure the app locks to the browser window size
        children=[
            dbc.Row(
                className="flex-grow-1",  # expands row to fill remaining vertical space
                children=[
                    # Side panel occupies 3/12 columns on medium+ screens
                    dbc.Col(create_control_panel(), md=3, className="h-100"),
                    # Heatmaps occupy 9/12 columns
                    dbc.Col(create_heatmap_grid(), md=9, className="h-100"),
                ],
            )
        ],
    )


def create_app() -> dash.Dash:
    app: dash.Dash = dash.Dash(__name__, external_stylesheets=[dbc.themes.DARKLY])
    load_figure_template(SETTINGS.PLOT_THEME)  # syncs Plotly figure defaults with the bootstrap darkly theme
    app.layout = create_layout()
    register_callbacks(app)
    return app

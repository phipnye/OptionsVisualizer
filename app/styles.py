# --- Colors

COLOR_PRIMARY: str = '#375A7F'  # primary Accent Color (Blue)
COLOR_ACCENT: str = '#00BC8C'  # secondary Accent Color (Success Green)
COLOR_BG_PRIMARY: str = '#272B30'  # main Background Color (The "paper" color)
COLOR_BG_SECONDARY: str = '#303030'  # plot Background Color (The "plot" color)
COLOR_TEXT_DEFAULT: str = '#FFFFFF'  # default Text Color (White)
COLOR_SLIDER_MARK: str = '#9C9C9C'  # slider Mark Color (Muted Gray)

# --- General sizing
PADDING_MD: str = "20px"
GAP_LG: str = "30px"
BORDER_THICKNESS: str = "1px solid "
BORDER_STYLE: str = BORDER_THICKNESS + COLOR_PRIMARY

# --- Panel and grid dimensions
CONTROL_PANEL_WIDTH: str = "20%"  # flex basis for the input column
GRID_ITEM_SIZE: str = "50%"  # for the 2x2 grid (width and height)

# --- Font sizes
FONT_SIZE_XL: str = "1.5rem"  # card header title
FONT_SIZE_LG: int = 18  # plot title size
FONT_SIZE_MD: str = "1.1rem"  # subheading size
FONT_SIZE_SM: str = "0.95rem"  # parameter summary text
FONT_SIZE_XS: str = "0.75rem"  # slider mark text

# --- Plot/Trace configuration
MARGIN_PLOT: dict[str, int] = dict(t=40, b=40, l=40, r=40)
FONT_PLOT_LBL: int = 14  # axis/hover label font size
FONT_PLOT_TICK: int = 12  # tick mark font size
FONT_PLOT_BAR_TITLE: int = 16  # colorbar title font size
FONT_PLOT_BAR_THICKNESS: int = 30  # colorbar thickness

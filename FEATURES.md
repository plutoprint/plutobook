## Features

| **Features** | **Properties** | **Notes / Limitations** |
|--------------|----------------|-------------------------|
| [Backgrounds](https://www.w3.org/TR/css-backgrounds-3/#backgrounds) | `background-color`, `background-image`, `background-repeat`, `background-attachment`, `background-position`, `background-clip`, `background-origin`, `background-size` | Used to control the background appearance of elements. Multiple background layers are currently not supported. |
| [Borders](https://www.w3.org/TR/css-backgrounds-3/#borders) | `border-color`, `border-style`, `border-width`, `border-radius` | Used to control the appearance and shape of element borders. |
| [Outlines](https://www.w3.org/TR/css-ui-3/#outline-props) | `outline-color`, `outline-style`, `outline-width`, `outline-offset` | Used to draw a line around elements without affecting layout. |
| [Box Model](https://www.w3.org/TR/css-box-3/) | `margin-top`, `margin-right`, `margin-bottom`, `margin-left`, `padding`, `padding-top`, `padding-right`, `padding-bottom`, `padding-left` | Used to control spacing around and inside elements. |
| [Box Sizing](https://www.w3.org/TR/css-sizing-3/) | `width`, `height`, `min-width`, `min-height`, `max-width`, `max-height`, `box-sizing` | Used to control the dimensions of elements. |
| [Positioning](https://www.w3.org/TR/css-position-3/) | `position`, `top`, `right`, `bottom`, `left`, `z-index`, `float`, `clear` | Used to control the placement of elements in the layout. |
| [Display](https://www.w3.org/TR/css-display-3/) | `display`, `visibility` | Used to control box generation and element visibility. Complex values like `display: contents` or multi-keyword values (e.g., `block flow`) are not supported. `visibility: collapse` is currently treated as `hidden` |
| [Color](https://www.w3.org/TR/css-color-3/) | `color`, `opacity` | Used to define the foreground color and opacity of an element. Supported color values include keywords, `#rgb`, `#rrggbb`, `rgb()`, `rgba()`, `hsl()`, and `hsla()` |
| [Fonts](https://www.w3.org/TR/css-fonts-4/) | `font`, `font-family`, `font-size`, `font-style`, `font-weight`, `font-stretch`, `font-variant`, `font-variant-caps`, `font-variant-ligatures`, `font-variant-position`, `font-variant-numeric`, `font-variant-east-asian`, `font-feature-settings`, `font-variation-settings`, `font-kerning` | Used to control font selection, typographic features, and advanced font behavior. Supports basic font styling, many OpenType features, and the `@font-face` rule |

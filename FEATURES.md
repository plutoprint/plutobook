# Features

- [Fonts](#fonts)
- [Color](#color)
- [Backgrounds and Borders](#backgrounds-and-borders)
- [Outlines](#outlines)
- [Box Model](#box-model)
- [Box Sizing](#box-sizing)
- [Positioning](#positioning)
- [Scalable Vector Graphics](#scalable-vector-graphics)

## Fonts

PlutoBook supports a wide range of CSS font properties, as specified in the [CSS Fonts Module Level 4](https://www.w3.org/TR/css-fonts-4). It lets you control typefaces, size, weight, style, and stretch through properties like `font`, `font-family`, `font-size`, `font-style`, `font-weight`, and `font-stretch`. For advanced typography, PlutoBook provides full support for OpenType features through both dedicated properties and convenient shorthands. This includes `font-feature-settings` and `font-kerning`, as well as `font-variant` and its subproperties: `font-variant-caps` for controlling capital letter forms, `font-variant-ligatures` for selecting standard and discretionary ligatures, `font-variant-position` for superscript and subscript placement, `font-variant-numeric` for numeric figure and fraction styles, and `font-variant-east-asian`, which controls the use of alternate glyphs and typographic features specific to East Asian scripts such as Japanese and Chinese. Variable fonts are supported through `font-variation-settings`, allowing fine-grained adjustment of axes like weight, width, or slant.

PlutoBook also supports the `@font-face` rule for loading fonts, with both local and remote sources. It can handle TrueType, OpenType, and WOFF font formats, and also supports bitmap fonts and SVG color emoji fonts, providing rich typographic and iconographic rendering.

## Color

PlutoBook fully supports standard CSS color formats as defined in the [CSS Color Module Level 3](https://www.w3.org/TR/css-color-3), including hexadecimal (`#rgb` and `#rrggbb`), functional notations like `rgb()`, `rgba()`, `hsl()`, and `hsla()`, as well as both basic and extended keyword colors. It also recognizes the special keywords `transparent` and `currentColor`, but does not support deprecated system color keywords such as `ButtonFace` and `WindowText`, which have been removed from modern CSS. In addition, PlutoBook honors the `opacity` property, which applies a uniform alpha transparency to an element and all its descendants, so the entire subtree appears with the same level of transparency.

## Backgrounds and Borders

PlutoBook implements the [CSS Backgrounds and Borders Module Level 3](https://www.w3.org/TR/css-backgrounds-3), providing control over the decoration of the border area and the background of the content, padding, and border areas. It supports properties like `background-color` for solid fills and `background-image` for adding images, along with `background-repeat`, `background-attachment`, `background-position`, `background-clip`, `background-origin`, and `background-size` for controlling placement and scaling. The `background` shorthand is fully supported, but multiple background layers are not yet supported.

For borders, PlutoBook handles `border-color`, `border-style`, and `border-width`, which can be combined using the `border` shorthand. Side-specific border properties such as `border-top`, `border-right`, `border-bottom`, and `border-left` are also supported for precise control of each edge. The shape of corners can be adjusted using `border-radius`, allowing smooth rounding for any or all corners.

## Outlines

PlutoBook supports outlines as defined in the [CSS Basic User Interface Module Level 3](https://www.w3.org/TR/css-ui-3). This includes `outline-color`, `outline-style`, `outline-width`, and `outline-offset`, allowing authors to draw a line around elements without affecting their box dimensions or layout flow. You can specify an outline’s color, style, and width using `outline-color`, `outline-style`, and `outline-width`, or conveniently set them all at once with the `outline` shorthand. The distance between the outline and the element’s border can be adjusted with `outline-offset`. Just like borders, outlines respect `border-radius`, so corners can be rounded consistently. Unlike borders, outlines do not take up space or influence the element’s size, making them useful for adding emphasis or separation without changing layout.

## Box Model

PlutoBook implements the CSS Box Model as described in the [CSS Box Model Module Level 3](https://www.w3.org/TR/css-box-3). It supports the standard margin and padding properties, allowing precise control over element spacing. Outer margins can be defined individually using `margin-top`, `margin-right`, `margin-bottom`, and `margin-left`, or set in a single declaration using the `margin` shorthand. Similarly, padding inside an element’s box can be specified with `padding-top`, `padding-right`, `padding-bottom`, and `padding-left`, or combined using the `padding` shorthand. These properties ensure consistent spacing between elements and their content, making it straightforward to build clear, well-structured layouts.

## Box Sizing

PlutoBook supports box sizing properties as described in the [CSS Box Sizing Module Level 3](https://www.w3.org/TR/css-sizing-3). It provides control over element dimensions using `width`, `height`, `min-width`, `min-height`, `max-width`, and `max-height`. These properties make it easy to define fixed, minimum, or maximum sizes to ensure flexible and predictable layouts. The `box-sizing` property is also supported, allowing you to choose whether an element’s `width` and `height` include its padding and border (`border-box`) or apply only to its content (`content-box`). This gives precise control over how an element’s total size is calculated in relation to its content and surrounding box model properties.

## Display

PlutoBook supports the `display` and `visibility` properties as defined in the [CSS Display Module Level 3](https://www.w3.org/TR/css-display-3/). These properties provide essential control over element rendering and visibility, enabling the creation of diverse and flexible layouts. The `display` property accepts a variety of values including `none`, `block`, `flex`, `inline`, `inline-block`, `inline-flex`, `inline-table`, `list-item`, `table`, and all key table display types such as `table-caption`, `table-cell`, `table-column`, `table-column-group`, `table-footer-group`, `table-header-group`, `table-row`, and `table-row-group`. Multi-keyword values (for example, `block flow`) are not supported.

For the `visibility` property, PlutoBook supports the standard values `visible` and `hidden`. The value `collapse` is currently treated the same as `hidden`.

## Positioning

PlutoBook supports positioning properties as defined in the [CSS Positioned Layout Module Level 3](https://www.w3.org/TR/css-position-3). The `position` property accepts the values `static`, `relative`, `absolute`, and `fixed`, allowing elements to be positioned accordingly within their containing block. In addition, CSS supports a more advanced value, `running(name)`, which enables elements to be captured and reused as running headers or footers. However, this feature is currently not supported in PlutoBook. Offsets can be controlled using `top`, `right`, `bottom`, and `left` to precisely move elements based on their positioning. The `z-index` property manages stacking order for overlapping elements. Together, these properties provide flexible control over element layout and layering.

## Scalable Vector Graphics

PlutoBook supports Scalable Vector Graphics (SVG), which can be embedded directly as standalone XML, inserted inline within HTML content, or referenced as external images. The top-level `svg` element behaves like a replaced element, similar to an `<img>`, and can use layout properties such as `display`, `float`, `margin`, `padding`, `border`, and `background`.

PlutoBook’s SVG implementation covers core features defined in the SVG 1.1 and SVG 1.2 Tiny specifications, with selected support for features from SVG 2.0 where practical. This includes basic shapes, paths, text, fills and strokes, gradients, patterns, markers, masks, clip paths, and transformations. Presentation attributes and CSS styling properties are both supported, allowing authors to style SVG content with inline attributes or external stylesheets. Animation within SVG is currently out of scope, but support for filters and scripting capabilities may be added in future versions.

# Features

- [Fonts](#fonts)
- [Color](#color)
- [Backgrounds and Borders](#backgrounds-and-borders)
- [Outlines](#outlines)
- [Box Model](#box-model)
- [Box Sizing](#box-sizing)
- [Display](#display)
- [Positioning](#positioning)
- [Floats](#floats)
- [Lists and Counters](#lists-and-counters)
- [Counter Styles](#counter-styles)
- [Tables](#tables)
- [Multiple Columns](#multiple-columns)
- [Flexible Box](#flexible-box)
- [Custom Properties](#custom-properties)
- [Values and Units](#values-and-units)
- [Transforms](#transforms)
- [Media Queries](#media-queries)
- [Paged Media](#paged-media)
- [Scalable Vector Graphics](#scalable-vector-graphics)

## Fonts

PlutoBook supports a wide range of CSS font properties, as specified in the [CSS Fonts Module Level 4](https://www.w3.org/TR/css-fonts-4). It lets you control typefaces, size, weight, style, and stretch through properties like `font`, `font-family`, `font-size`, `font-style`, `font-weight`, and `font-stretch`. For advanced typography, PlutoBook provides full support for OpenType features through both dedicated properties and convenient shorthands. This includes `font-feature-settings` and `font-kerning`, as well as `font-variant` and its subproperties: `font-variant-caps` for controlling capital letter forms, `font-variant-ligatures` for selecting standard and discretionary ligatures, `font-variant-position` for superscript and subscript placement, `font-variant-numeric` for numeric figure and fraction styles, and `font-variant-east-asian`, which controls the use of alternate glyphs and typographic features specific to East Asian scripts such as Japanese and Chinese. Variable fonts are supported through `font-variation-settings`, allowing fine-grained adjustment of axes like weight, width, or slant.

PlutoBook also supports the `@font-face` rule for loading fonts, with both local and remote sources. It can handle TrueType, OpenType, and WOFF font formats, and also supports bitmap fonts and SVG color emoji fonts, providing rich typographic and iconographic rendering.

## Color

PlutoBook fully supports standard CSS color formats as defined in the [CSS Color Module Level 3](https://www.w3.org/TR/css-color-3), including hexadecimal (`#rgb` and `#rrggbb`), functional notations like `rgb()`, `rgba()`, `hsl()`, and `hsla()`, as well as both basic and extended keyword colors. It also recognizes the special keywords `transparent` and `currentColor`, but does not support deprecated system color keywords such as `ButtonFace` and `WindowText`, which have been removed from modern CSS. In addition, PlutoBook honors the `opacity` property, which applies a uniform alpha transparency to an element and all its descendants, so the entire subtree appears with the same level of transparency.

PlutoBook also provides partial support for [CSS Color Module Level 4](https://www.w3.org/TR/css-color-4). This includes modern hexadecimal forms with alpha (`#rgba`, `#rrggbbaa`), the `hwb()` function, and the updated space-separated syntax with an optional `/` for alpha (e.g. `rgb(255 0 0 / 0.5)`). More advanced Level 4 features such as `lab()`, `lch()`, and `color()` are not yet supported.

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

## Floats

PlutoBook supports floating elements as defined in the [CSS 2 Visual Formatting Model](https://www.w3.org/TR/CSS2/visuren.html#floats). The `float` property allows elements to be taken out of the normal flow and positioned to the left or right of their containing block, enabling text and inline content to wrap around them. Supported values for `float` are `left`, `right`, and `none`.

To manage how content behaves around floated elements, PlutoBook supports the `clear` property. This property controls the placement of block-level elements by preventing them from flowing next to floated elements on the specified side(s). Supported values for `clear` are `left`, `right`, `both`, and `none`.

## Lists and Counters

PlutoBook supports list rendering and counters as described in the [CSS Lists and Counters Module Level 3](https://www.w3.org/TR/css-lists-3). The `list-style-type` property sets the marker (such as a disc, character, or custom counter style) for a list item element, allowing for a wide variety of list appearances like bullets, numbers, or custom symbols. The `list-style-position` property controls whether the marker is placed inside or outside the list item’s content box, while `list-style-image` lets you specify an image to be used as the marker.

These properties can be conveniently combined using the `list-style` shorthand, which sets the marker type, position, and image in a single declaration.

PlutoBook also implements the `::marker` pseudo-element, enabling fine-grained styling of list item markers with properties such as `color`, `font`, and other text-related features, without affecting the list item’s main content.

In addition to basic list styling, PlutoBook provides robust support for CSS counters, making it possible to create complex numbered structures such as multi-level outlines or custom numbering schemes. The `counter-reset` property initializes one or more named counters, while `counter-increment` controls how counters are advanced when an element is rendered. The `counter-set` property directly sets a counter to a specific value.

To display counter values, PlutoBook supports the `counter()` and `counters()` functions within the `content` property, allowing you to output counter values with optional custom formatting or separators.

## Counter Styles

PlutoBook implements custom counter styles as defined in the [CSS Counter Styles Module Level 3](https://www.w3.org/TR/css-counter-styles-3). This module extends how lists and counters display by allowing authors to define their own numbering systems or reuse predefined ones, in addition to the common types like `decimal`, `lower-roman`, `upper-roman`, `lower-alpha`, and `upper-alpha`.

Using the `@counter-style` at-rule, you can give a custom counter style a name, specify a sequence of symbols (including emojis or other Unicode characters), choose a fallback system, and define optional rules for prefixes, suffixes, or negative values. This makes it possible to design culturally appropriate, decorative, or playful counters that match the tone of your content.

Once defined, custom counter styles can be applied with `list-style-type` or used in counter functions such as `counter()` and `counters()`. PlutoBook renders these user-defined styles just like built-in ones, giving you full flexibility over numbering and marker design, including the creative use of emoji markers.

## Tables

PlutoBook implements table layout and styling as defined in the [CSS Tables Module Level 3](https://www.w3.org/TR/css-tables-3). This includes support for semantic table elements and the key properties needed to create well-structured, flexible tables.

The `display` property recognizes table-specific values such as `table`, `inline-table`, `table-row`, `table-row-group`, `table-header-group`, `table-footer-group`, `table-column`, `table-column-group`, `table-cell`, and `table-caption`. These allow authors to build tables using standard HTML `<table>` markup or to arrange elements in a tabular format entirely with CSS.

PlutoBook supports `border-collapse` and `border-spacing` for controlling how cell borders are rendered and how much space appears between them. The `caption-side` property determines where the table’s caption is placed relative to the table box. The `table-layout` property is supported with both `auto` and `fixed` values, allowing you to choose whether column widths depend on cell content or a fixed layout.

Cells can span multiple rows and columns using `row-span` and `column-span` attributes or equivalent CSS properties. The special value `row-span: 0` indicates that a cell should automatically span all remaining rows in its row group, which is useful for creating flexible, dynamically sized tables.

Row and column sizing can be managed using `width` on columns or cells, along with box model and border properties for precise control of cell padding, borders, and backgrounds. These features ensure that tables remain clear, readable, and well-integrated with surrounding content.

## Multiple Columns

PlutoBook supports multi-column layout features as defined in the [CSS Multi-column Layout Module Level 1](https://www.w3.org/TR/css-multicol-1/). These properties make it easy to flow content into multiple columns, similar to a newspaper or magazine, for more compact and visually engaging text presentation.

You can specify the number of columns using `column-count` or set the desired column width with `column-width`. PlutoBook calculates how many columns can fit based on these constraints and the available space. For more precise control, both can be used together, balancing between a fixed width and a specific number of columns.

The `columns` shorthand provides a convenient way to set both `column-width` and `column-count` in a single declaration. Gaps between columns can be adjusted using `column-gap` to ensure comfortable reading flow.

Decorative dividers between columns can be created with `column-rule-color`, `column-rule-style`, and `column-rule-width`, or all at once with the `column-rule` shorthand.

The `column-span` property lets an element span across all columns, which is especially useful for headings, banners, or figures that should break the column flow.

Finally, PlutoBook supports `column-fill`, which controls how content is distributed when there is extra vertical space. It accepts values like `balance` to balance content evenly across columns, or `auto` to fill columns sequentially.

## Flexible Box

PlutoBook supports flexible box layout as described in the [CSS Flexible Box Layout Module Level 1](https://www.w3.org/TR/css-flexbox-1/). Flexbox provides an efficient way to arrange and distribute space among items within a container, even when their size is unknown or dynamic.

The `display` property accepts `flex` and `inline-flex` to define a flex container. Flex containers establish a flex formatting context for their direct children, known as flex items. Within a flex container, you can control the direction of item placement with `flex-direction`, manage wrapping behavior with `flex-wrap`, and combine these settings conveniently using the `flex-flow` shorthand.

Alignment and spacing are handled using `justify-content` (for main-axis alignment), `align-items` (for cross-axis alignment of items), and `align-content` (for alignment of multiple lines when wrapping). Individual flex items can override alignment with `align-self`.

Flex items can grow, shrink, and maintain base sizes using the `flex` shorthand or its longhand properties: `flex-grow`, `flex-shrink`, and `flex-basis`. The `order` property lets you control the visual order of items independent of their source order, providing powerful layout flexibility.

Spacing between flex items is managed with the `gap` property, which defines a uniform spacing value. More granular control is available with `row-gap` (for spacing between rows) and `column-gap` (for spacing between columns).

## Custom Properties

PlutoBook supports CSS custom properties as described in the [CSS Custom Properties for Cascading Variables Module Level 1](https://www.w3.org/TR/css-variables-1/), also known as CSS variables. Custom properties allow reusable values to be defined once and referenced throughout a stylesheet using the `var()` function. They provide a powerful way to centralize colors, sizes, and other design tokens, improving maintainability and consistency across documents.

Custom properties are declared with identifiers prefixed by `--` and can be applied anywhere a property value is accepted. PlutoBook also supports fallback values in the `var()` function, so `var(--unknown, red)` will use `red` if `--unknown` is not defined.

PlutoBook fully supports the cascading and inheritance behavior of custom properties as defined in the specification, ensuring that updates propagate naturally through the document. However, advanced features introduced in later drafts, such as typed custom properties defined with the `@property` at-rule, are not yet supported.

## Values and Units

PlutoBook supports CSS values and units as described in the [CSS Values and Units Module Level 3](https://www.w3.org/TR/css-values-3/), which define the data types and measurement systems used throughout CSS. Values and units form the foundation of style rules, allowing authors to express sizes, positions, colors, and other aspects of presentation with precision.

PlutoBook currently supports the most common absolute and relative length units, including `px`, `pt`, `cm`, `mm`, `in`, `pc`, `em`, `rem`, `%`, and viewport-based units like `vw`, `vh`, `vmin`, and `vmax`. Color values are supported through keywords (such as `red`), hexadecimal notation, and functional syntaxes like `rgb()` and `hsl()`. Angles (`deg`, `rad`, `grad`, `turn`) are also supported wherever relevant.

PlutoBook also supports CSS wide keywords (`initial`, `inherit`, and `unset`), which provide standardized mechanisms for resetting or inheriting property values. These keywords work consistently across all supported properties, following the behavior defined in the specification.

PlutoBook also supports CSS math functions, which allow property values to be calculated dynamically at render time. Math functions provide flexibility when combining lengths and other units in styles. Currently, the functions `calc()`, `min()`, `max()`, and `clamp()` are supported, including nested expressions, so complex formulas with addition, subtraction, multiplication, and division are possible. However, percentage values inside math functions are not supported, and calculations must use absolute or relative length units instead.

## Transforms

PlutoBook supports CSS transforms as described in the [CSS Transforms Module Level 1](https://drafts.csswg.org/css-transforms-1/). Transforms allow authors to modify the coordinate space of elements for effects such as translation, rotation, scaling, and skewing, enabling rich layout and visual transformations without altering document flow.

PlutoBook currently supports the `transform` and `transform-origin` properties and the full set of 2D transform functions: `matrix()`, `rotate()`, `translate()`, `translateX()`, `translateY()`, `scale()`, `scaleX()`, `scaleY()`, `skew()`, `skewX()`, and `skewY()`. These functions may be combined in `transform` lists and accept length, angle, and number values according to the specification. `transform-origin` is supported to control the transform's reference point.

PlutoBook does not currently support the 3D transform related properties and functions, nor the 3D transform rendering behaviors. Specifically, `transform-style`, `perspective`, `perspective-origin`, and `backface-visibility` are not supported, and the 3D transform functions `matrix3d()`, `rotate3d()`, `rotateX()`, `rotateY()`, `rotateZ()`, `translate3d()`, `translateZ()`, `scale3d()`, and `scaleZ()` are not available.

## Media Queries

PlutoBook supports media queries as described in [CSS Media Queries Level 3](https://www.w3.org/TR/css3-mediaqueries), allowing styles to adapt based on the characteristics of the output environment. This makes it possible to adjust layouts and presentation to match different devices and contexts. PlutoBook understands the standard media types: `all` for any device, `print` for paged output like PDFs, and `screen` for typical on-screen displays.

Authors can refine conditions using media features that test the dimensions of the viewport or page, such as `width`, `min-width`, `max-width`, `height`, `min-height`, and `max-height`. It also supports checking the `orientation` to distinguish between portrait and landscape layouts. Multiple conditions can be combined using the `and` keyword, creating precise rules that apply only when every condition is true.

To further control when styles apply, PlutoBook recognizes the `only` and `not` restrictors. The `only` keyword ensures that a media query is used only if the entire query is fully understood, while `not` reverses the condition so styles apply in all other cases. This flexible system helps authors craft responsive designs that look good across different screens and when printed.

## Paged Media

PlutoBook provides support for paged media as defined in the [CSS Paged Media Module Level 3](https://www.w3.org/TR/css-page-3/). This allows precise control over how content is laid out when divided across multiple pages. PlutoBook fully supports margin boxes, so authors can define running headers and footers using properties like `@top-left` and `@bottom-center` inside `@page` rules. These margin boxes can be styled and positioned separately for each page edge, offering fine control over page design in printed or paginated output.

PlutoBook also supports page counters, which let authors output the current page number using the `counter(page)` function. These counters work within the page context and update automatically as pages progress. General-purpose counters are available too through the `counter()` and `counters()` functions, so you can generate dynamic numbering in both content and page areas.

Leaders, such as repeating dots or lines often used in tables of contents, are supported through the `leader()` function in generated content. This provides an easy way to fill space between elements with consistent patterns.

Cross-referencing is possible with the `target-counter()` and `target-counters()` functions, which allow you to show counter values from linked elements, such as displaying the page number where a section starts.

PlutoBook also supports running elements, which make it possible to reuse document content inside headers or footers. Running headers and footers can be created using CSS `position: running(NAME)` on the source element and `content: element(NAME)` inside margin boxes.

Some limits currently exist. PlutoBook uses only the page size and margin defined for the first page, and `@page :first` is the only pseudo-page selector that works reliably for now. Named pages created with the `@page name` syntax are not fully implemented yet, so custom page styles may not apply as expected.

## Scalable Vector Graphics

PlutoBook supports Scalable Vector Graphics (SVG), which can be embedded directly as standalone XML, inserted inline within HTML content, or referenced as external images. The top-level `svg` element behaves like a replaced element, similar to an `<img>`, and can use layout properties such as `display`, `float`, `margin`, `padding`, `border`, and `background`.

PlutoBook’s SVG implementation covers core features defined in the SVG 1.1 and SVG 1.2 Tiny specifications, with selected support for features from SVG 2.0 where practical. This includes basic shapes, paths, text, fills and strokes, gradients, patterns, markers, masks, clip paths, and transformations. Presentation attributes and CSS styling properties are both supported, allowing authors to style SVG content with inline attributes or external stylesheets. Animation within SVG is currently out of scope, but support for filters and scripting capabilities may be added in future versions.

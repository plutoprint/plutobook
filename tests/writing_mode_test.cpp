#include <plutobook.hpp>
#include "../source/layout/writingmode.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace plutobook;
static_assert(WritingDirection(WritingMode::VerticalRl, Direction::Ltr).blockStart() == BoxSideRight);
static_assert(WritingDirection(WritingMode::VerticalLr, Direction::Ltr).inlineStart() == BoxSideTop);
static_assert(WritingDirection(WritingMode::SidewaysLr, Direction::Ltr).inlineStart() == BoxSideBottom);
static_assert(WritingDirection(WritingMode::SidewaysLr, Direction::Rtl).startStart() == BoxCornerTopLeft);
constexpr int size = 400;
using Pixels = std::vector<uint32_t>;

Pixels render(const std::string& css, const std::string& content)
{
    plutobook::Book book({size * plutobook::units::px, size * plutobook::units::px},
        plutobook::PageMargins::None, plutobook::MediaType::Screen);
    const auto html = "<!doctype html><html lang='ja'><meta charset='utf-8'><style>"
        "body{margin:0;background:white;font:24px/32px sans-serif}"
        "#flow{width:240px;height:240px} .red{color:#ff0000}.blue{color:#0000ff}"
        + css + "</style><body><div id='flow'>" + content + "</div></body></html>";
    if(!book.loadHtml(html))
        throw std::runtime_error("HTML load failed");
    plutobook::ImageCanvas canvas(size, size);
    book.renderDocument(canvas);
    Pixels pixels(size * size);
    for(int y = 0; y < size; ++y)
        std::memcpy(pixels.data() + y * size, canvas.data() + y * canvas.stride(), size * 4);
    return pixels;
}

struct Bounds {
    int left{size}, top{size}, right{-1}, bottom{-1}, count{0};
    int width() const { return right - left + 1; }
    int height() const { return bottom - top + 1; }
};

Bounds bounds(const Pixels& pixels, bool red = true)
{
    Bounds result;
    for(int y = 0; y < size; ++y) {
        for(int x = 0; x < size; ++x) {
            auto pixel = pixels[y * size + x];
            const auto r = (pixel >> 16) & 255;
            const auto g = (pixel >> 8) & 255;
            const auto b = pixel & 255;
            if((pixel >> 24) > 200 && g < 100 && (red ? r > 180 && b < 100 : b > 180 && r < 100)) {
                result.left = std::min(result.left, x);
                result.right = std::max(result.right, x);
                result.top = std::min(result.top, y);
                result.bottom = std::max(result.bottom, y);
                ++result.count;
            }
        }
    }
    return result;
}

void check(bool value, const char* message)
{
    if(!value)
        throw std::runtime_error(message);
    std::cout << "PASS " << message << '\n';
}
}

int main()
{
    try {
        const std::string lines = "<span class='red'>ABCD</span><br><span class='blue'>EFGH</span>";
        const auto horizontal = render("", lines);
        const auto rl = render("#flow{writing-mode:vertical-rl}", lines);
        const auto lr = render("#flow{writing-mode:vertical-lr}", lines);
        check(bounds(horizontal).bottom < bounds(horizontal, false).top, "horizontal lines progress down");
        check(bounds(rl).left > bounds(rl, false).right, "vertical-rl columns progress left");
        check(bounds(lr).right < bounds(lr, false).left, "vertical-lr columns progress right");
        auto sidewaysLr = render("#flow{writing-mode:sideways-lr}", lines);
        check(bounds(sidewaysLr).right < bounds(sidewaysLr, false).left && bounds(sidewaysLr).top > 100,
            "sideways-lr columns progress right and start at the bottom");
        check(rl == render("#flow{writing-mode:sideways-rl}", lines), "sideways-rl uses clockwise Latin shaping");
        check(sidewaysLr == render("#flow{writing-mode:sideways-lr;text-orientation:upright}", lines),
            "text-orientation does not affect sideways writing modes");
        check(bounds(rl).height() > bounds(rl).width(), "mixed Latin advances down the column");

        const std::string latin = "<span class='red'>ABCDE</span>";
        auto upright = render("#flow{writing-mode:vertical-rl;text-orientation:upright}", latin);
        auto sideways = render("#flow{writing-mode:vertical-rl;text-orientation:sideways}", latin);
        check(bounds(upright).height() > bounds(sideways).height() + 10, "upright Latin uses vertical glyph advances");
        check(horizontal == render("#flow{text-orientation:upright}", lines), "text-orientation leaves horizontal text unchanged");
        check(rl == render("#flow{writing-mode:vertical-rl;text-orientation:invalid}", lines), "invalid orientation falls back to mixed");
        check(horizontal == render("#flow{writing-mode:invalid}", lines), "invalid writing-mode falls back to horizontal");
        check(upright == render("#flow{writing-mode:vertical-rl;text-orientation:upright;direction:rtl}", latin), "upright uses left-to-right direction");
        check(upright == render("#flow{writing-mode:vertical-rl}.red{text-orientation:upright}", latin), "inline orientation matches inherited orientation");
        check(render("#flow{writing-mode:vertical-rl}", latin)
            == render("#flow{writing-mode:vertical-rl;text-orientation:upright}.red{text-orientation:initial}", latin),
            "initial resets an inherited text orientation");
        check(render("#flow{writing-mode:vertical-rl}", "<span class='red'>ガ</span>")
            == render("#flow{writing-mode:vertical-rl}", "<span class='red'>ガ</span>"),
            "combining dakuten stays with its Japanese grapheme");

        const std::string wrapText = "<span class='red'>一二三四五六七八九十一二三四五六七八九十</span>";
        auto shortColumn = bounds(render("#flow{writing-mode:vertical-rl;height:100px}", wrapText));
        auto tallColumn = bounds(render("#flow{writing-mode:vertical-rl;height:240px}", wrapText));
        check(shortColumn.count > 0 && shortColumn.width() > tallColumn.width(), "physical height controls Japanese wrapping");
        auto nested = render("#flow{writing-mode:vertical-rl}p{margin:0}",
            "<p class='red'>ABCD</p><p class='blue'>EFGH</p>");
        check(bounds(nested).left > bounds(nested, false).right, "nested blocks follow vertical block progression");
        auto fixedBlocks = render("#flow{writing-mode:vertical-rl}p{width:40px;margin:0}",
            "<p class='red'>AB</p><p class='blue'>CD</p>");
        check(bounds(fixedBlocks).count > 0 && bounds(fixedBlocks, false).count > 0
            && bounds(fixedBlocks).left - bounds(fixedBlocks, false).left < 50,
            "fixed block widths do not absorb horizontal flow margins");

        auto atomic = bounds(render("#flow{writing-mode:vertical-rl}",
            "<span style='display:inline-block;width:20px;height:40px;background:red'></span>"));
        check(atomic.width() == 20 && atomic.height() == 40, "atomic inline box retains physical dimensions");
        auto orthogonal = render("#flow{writing-mode:vertical-rl}",
            "<span class='red' style='writing-mode:horizontal-tb'>ABCDE</span>");
        check(bounds(orthogonal).width() > bounds(orthogonal).height(), "orthogonal inline text stays horizontal");
        auto positionedInline = render("#flow{writing-mode:vertical-rl}.red{position:relative}", latin);
        check(positionedInline == render("#flow{writing-mode:vertical-rl}", latin), "layered inline text uses vertical coordinates");
        auto autoAtomic = bounds(render("#flow{writing-mode:vertical-rl;width:auto}",
            "<span style='display:inline-block;width:20px;height:40px;background:red'></span>"));
        check(autoAtomic.count == 800 && autoAtomic.left >= 0, "auto block width updates atomic child positions");
        auto clip = bounds(render("#flow{writing-mode:vertical-rl;height:40px;overflow:hidden}", latin));
        check(clip.count > 0 && clip.bottom < 40, "overflow clips in physical coordinates");
        check(bounds(render("#flow{writing-mode:vertical-rl;padding:20px}", latin)).top >= 20, "physical top padding becomes inline-start padding");

        // Repeated layout/paint must not accumulate transforms or stale line positions.
        const auto verticalCells = render(
            "table{border-collapse:collapse;table-layout:fixed;width:200px}"
            "th,td{writing-mode:vertical-rl;vertical-align:top;padding:0;font-weight:normal}"
            "th{height:60px;background:red}td{height:120px;background:blue}",
            "<table><tr><th>A</th></tr><tr><td>BCDEFGHIJKLMNOP</td></tr></table>");
        check(bounds(verticalCells).height() == 60 && bounds(verticalCells, false).height() == 120
            && bounds(verticalCells, false).top == 60,
            "vertical th and td honor explicit heights and row positions");
        const auto sharedRow = render(
            "table{border-collapse:collapse;table-layout:fixed;width:200px}"
            "td{writing-mode:vertical-lr;vertical-align:top;padding:0;height:60px;background:red}"
            "td+td{height:120px;background:blue}",
            "<table><tr><td>AB</td><td>CD</td></tr></table>");
        check(bounds(sharedRow).height() == 120 && bounds(sharedRow, false).height() == 120,
            "vertical cells adopt the resolved shared row height");
        const std::string compactCellCss = "table{border-collapse:collapse}"
            "td{writing-mode:vertical-rl;text-orientation:upright;vertical-align:top;padding:0;background:red}";
        const auto autoCell = bounds(render(compactCellCss, "<table><tr><td>ABC</td></tr></table>"));
        check(autoCell.height() > 40 && autoCell.height() < 120 && autoCell.width() <= 40,
            "auto-height vertical text cells size to their text instead of the viewport");
        const auto wrappingCell = bounds(render(compactCellCss + "td{height:96px}",
            "<table><tr><td>一二三四五六七八九十一二</td></tr></table>"));
        check(wrappingCell.height() == 96 && wrappingCell.width() >= 64 && wrappingCell.width() <= 128,
            "automatic table columns measure vertical column extents rather than text advances");
        const auto emptyCell = bounds(render(compactCellCss + "td{padding:10px}",
            "<table><tr><td></td></tr></table>"));
        check(emptyCell.height() == 20 && emptyCell.width() == 20,
            "empty vertical cells retain padding without acquiring viewport height");
        const auto spanningCells = render(
            "table{border-collapse:collapse;table-layout:fixed;width:200px}"
            "td{writing-mode:vertical-rl;vertical-align:top;padding:0;height:60px;background:blue}"
            "td[rowspan]{height:120px;background:red}",
            "<table><tr><td rowspan='2'>ABCD</td><td>EF</td></tr><tr><td>GH</td></tr></table>");
        check(bounds(spanningCells).height() == 120 && bounds(spanningCells, false).height() == 120,
            "vertical rowspan cells cover the combined row heights");
        const auto paddedCell = bounds(render(compactCellCss + "td{box-sizing:border-box;height:120px;padding:10px;border:2px solid red}",
            "<table><tr><td>ABC</td></tr></table>"));
        // Collapsed outer borders extend one pixel beyond each cell edge.
        check(paddedCell.height() == 122, "vertical cell border-box height includes padding and collapsed borders");
        const auto mixedCells = render(
            "table{border-collapse:separate;border-spacing:4px;table-layout:fixed;width:208px}"
            "td{padding:0;vertical-align:top;height:96px;background:red;writing-mode:vertical-rl}"
            "td+td{writing-mode:horizontal-tb;background:blue}",
            "<table><tr><td>AB</td><td>CD</td></tr></table>");
        check(bounds(mixedCells).height() == 96 && bounds(mixedCells, false).height() == 96
            && bounds(mixedCells, false).left > bounds(mixedCells).right + 1,
            "mixed horizontal and vertical cells share row height and preserve separate border spacing");
        const auto colspans = render(
            "table{border-collapse:collapse;table-layout:fixed;width:200px}"
            "td{writing-mode:vertical-lr;vertical-align:top;padding:0;height:60px;background:blue}"
            "td[colspan]{background:red}",
            "<table><tr><td colspan='2'>AB</td></tr><tr><td>CD</td><td>EF</td></tr></table>");
        check(bounds(colspans).width() == 200 && bounds(colspans).height() == 60
            && bounds(colspans, false).width() == 200 && bounds(colspans, false).top == 60,
            "vertical colspan cells preserve table column geometry");
        check(rl == render("#flow{writing-mode:vertical-rl}", lines), "repeat rendering is deterministic");
        return 0;
    } catch(const std::exception& error) {
        std::cerr << "FAIL " << error.what() << '\n';
        return 1;
    }
}

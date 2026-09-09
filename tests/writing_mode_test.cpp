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
        check(rl == render("#flow{writing-mode:vertical-rl}", lines), "repeat rendering is deterministic");
        return 0;
    } catch(const std::exception& error) {
        std::cerr << "FAIL " << error.what() << '\n';
        return 1;
    }
}

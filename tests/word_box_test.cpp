#include "plutobook.hpp"
#include "textbox.h"
#include <cairo.h>

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace plutobook;

namespace {
void check(bool condition, const char* message) {
    if(!condition) throw std::runtime_error(message);
    std::cout << "PASS " << message << '\n';
}
void collect(Box* box, WordBoxList& result) {
    if(box->isTextBox()) {
        const auto words = static_cast<TextBox*>(box)->words();
        result.insert(result.end(), words.begin(), words.end());
    }
    for(auto child = box->firstChild(); child; child = child->nextSibling())
        collect(child, result);
}
WordBoxList inspect(const std::string& content, const std::string& css = {}, const char* preview = nullptr) {
    Book book({400 * units::px, 400 * units::px}, PageMargins::None, MediaType::Screen);
    const auto html = "<!doctype html><html lang='ja'><meta charset='utf-8'><style>"
        "body{margin:0;background:white}#flow{width:320px;height:360px;font:24px/36px sans-serif;color:red}"
        + css + "</style><body><div id='flow'>" + content + "</div></body></html>";
    if(!book.loadHtml(html)) throw std::runtime_error("HTML load failed");
    (void)book.documentWidth();
    WordBoxList words;
    collect(book.rootBox(), words);
    for(const auto& word : words) {
        if(word.fragments.empty() || word.bounds.isEmpty()) throw std::runtime_error("Empty word bounds");
        for(const auto& rect : word.fragments)
            if(!word.bounds.contains(rect)) throw std::runtime_error("Union does not contain fragment");
    }
    if(preview) {
        ImageCanvas canvas(400, 400);
        book.renderDocument(canvas);
        // These fixtures position the containing block at document origin.
        const auto* bytes = canvas.data();
        for(int y = 0; y < 400; ++y) {
            const auto* pixels = reinterpret_cast<const uint32_t*>(bytes + y * canvas.stride());
            for(int x = 0; x < 400; ++x) {
                const auto pixel = pixels[x];
                if(((pixel >> 16) & 255) < 200 || ((pixel >> 8) & 255) > 100 || (pixel & 255) > 100)
                    continue;
                bool covered = false;
                for(const auto& word : words) {
                    for(const auto& rect : word.fragments) {
                        if(x >= rect.x - 2 && x <= rect.right() + 2 && y >= rect.y - 2 && y <= rect.bottom() + 2)
                            covered = true;
                    }
                }
                if(!covered) throw std::runtime_error("Rendered word pixel outside word rectangles");
            }
        }
        auto* context = canvas.context();
        cairo_set_source_rgba(context, 0, 0.4, 0.7, 0.75);
        cairo_set_line_width(context, 1);
        for(const auto& word : words) {
            for(const auto& rect : word.fragments)
                cairo_rectangle(context, rect.x, rect.y, rect.w, rect.h);
        }
        cairo_stroke(context);
        if(!canvas.writeToPng(preview)) throw std::runtime_error("Cannot write word overlay");
    }
    return words; // Owning snapshots survive Book destruction.
}
}

int main() {
    try {
        auto plain = inspect("Hello, world! 123");
        check(plain.size() == 3 && plain[0].word == "Hello" && plain[1].word == "world" && plain[2].word == "123",
            "word iterator excludes punctuation and whitespace");
        check(plain[0].bounds.right() < plain[1].bounds.x, "word bounds exclude intervening punctuation/space");
        auto wrapped = inspect("extraordinary", "#flow{width:65px;word-break:break-all}", "word_bounds_wrapped.png");
        check(wrapped.size() == 1 && wrapped[0].word == "extraordinary" && wrapped[0].fragments.size() > 1,
            "a wrapped word keeps one identity and multiple rectangles");
        auto rtl = inspect("שלום עולם", "#flow{direction:rtl}", "word_bounds_rtl.png");
        check(rtl.size() == 2 && rtl[0].bounds.x > rtl[1].bounds.x, "RTL words iterate logically with visual bounds");
        auto vertical = inspect("Alpha Beta", "#flow{writing-mode:vertical-rl}", "word_bounds_vertical.png");
        check(vertical.size() == 2 && vertical[0].bounds.h > vertical[0].bounds.w && vertical[0].bounds.bottom() < vertical[1].bounds.y,
            "vertical words use physical coordinates");
        auto lr = inspect("Alpha Beta", "#flow{writing-mode:vertical-lr}", "word_bounds_vertical_lr.png");
        check(lr.size() == 2 && lr[0].bounds.x < vertical[0].bounds.x, "vertical-lr maps to left side");
        auto sideways = inspect("Alpha Beta", "#flow{writing-mode:sideways-lr}", "word_bounds_sideways.png");
        check(sideways.size() == 2 && sideways[0].bounds.y > sideways[1].bounds.y, "sideways-lr maps bottom-to-top");
        auto cjk = inspect("日本語の文章。中文測試。", "#flow{writing-mode:vertical-rl}");
        std::string reconstructed;
        for(WordIterator it = cjk.cbegin(); it != cjk.cend(); ++it) reconstructed += it->word;
        check(cjk.size() > 2 && reconstructed == "日本語の文章中文測試", "ICU segments Japanese and Chinese without spaces");
        auto cjkOverlay = inspect("日本語 中文測試", "#flow{writing-mode:vertical-rl;text-orientation:upright}", "word_bounds_cjk.png");
        check(cjkOverlay.size() > 1, "Japanese/Chinese rectangles cover rendered glyphs");
        auto combining = inspect("café 𐐀𐐁");
        check(combining.size() == 2 && combining[0].endOffset == 5 && combining[1].endOffset - combining[1].startOffset == 4,
            "combining characters and supplementary letters preserve UTF-16 offsets");
        auto uppercase = inspect("straße", "#flow{text-transform:uppercase}");
        check(uppercase.size() == 1 && uppercase[0].word == "STRASSE" && uppercase[0].endOffset == 7,
            "word text and offsets reflect CSS text-transform");
        auto natural = inspect("hello");
        auto spaced = inspect("hello", "#flow{letter-spacing:5px}");
        check(std::abs(spaced[0].bounds.w - natural[0].bounds.w - 25) < 0.1f, "bounds include letter spacing");
        auto normalLine = inspect("one two three four five six", "#flow{width:160px}");
        auto justified = inspect("one two three four five six", "#flow{width:160px;text-align:justify}");
        check(justified[1].bounds.x > normalLine[1].bounds.x, "bounds include line justification expansion");
        auto nodes = inspect("hel<b>lo</b>");
        check(nodes.size() == 2 && nodes[0].word == "hel" && nodes[1].word == "lo", "iterator is scoped to individual TextBoxes");
        check(inspect("hidden", "#flow{visibility:hidden}").empty(), "hidden text has no word rectangles");
        check(inspect(" \t\n ").empty(), "whitespace-only text has no words");
        auto ligatures = inspect("office affinity", "#flow{font-family:serif;font-variant-ligatures:common-ligatures}", "word_bounds_ligatures.png");
        check(ligatures.size() == 2, "ligature glyph clusters contribute to complete word bounds");
        check(plain[0].word == "Hello" && plain[0].bounds.w > 0, "snapshots remain valid after Book destruction");
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "FAIL " << e.what() << '\n';
        return 1;
    }
}

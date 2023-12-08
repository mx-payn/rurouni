// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/graphics/font.hpp"
#include "rurouni/graphics/msdf_data.hpp"
#include "rurouni/graphics/text.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/math.hpp"

namespace rr::graphics {

double g_lineCount = 0.0;
float g_horizontalPadding = 0.0f;

static std::array<math::vec2, 4> get_plane_position(
    const msdf_atlas::GlyphGeometry* glyph,
    double x,
    double y,
    double fsScale) {
    double pl, pb, pr, pt;
    glyph->getQuadPlaneBounds(pl, pb, pr, pt);
    math::vec2 quadMin((float)pl, (float)pb);
    math::vec2 quadMax((float)pr, (float)pt);

    quadMin *= fsScale;
    quadMax *= fsScale;
    quadMin += math::vec2(x, y);
    quadMax += math::vec2(x, y);

    return {quadMin, {quadMin.x, quadMax.y}, quadMax, {quadMax.x, quadMin.y}};
}

void advance_x(char current,
               char next,
               double& x,
               double fsScale,
               const msdf_atlas::FontGeometry& fontGeometry) {
    double advance;
    fontGeometry.getAdvance(advance, current, next);
    float kerningOffset = 0.0f;
    x += fsScale * advance + kerningOffset;
}

void push_to_new_line(std::vector<Character>& word,
                      char nextInText,
                      double& x,
                      double& y,
                      double fsScale,
                      const msdf_atlas::FontGeometry& fontGeometry) {
    // need to drop current word on new line
    for (int j = 0; j < word.size(); j++) {
        Character& c = word[j];
        auto glyph = fontGeometry.getGlyph(c.Codepoint);

        // set new position on screen
        auto quadUV = get_plane_position(glyph, x, y, fsScale);
        c.quadPosition_uv = quadUV;

        // advance
        uint32_t nextCharacter;
        if (j == word.size() - 1)
            nextCharacter = nextInText;
        else
            nextCharacter = word[j + 1].Codepoint;

        advance_x(c.Codepoint, nextCharacter, x, fsScale, fontGeometry);
    }
}

static std::vector<Character> parse_line(
    const std::string& line,
    double& x,
    double& y,
    double fsScale,
    double lineHeight,
    float lineHeightOffset,
    float maxWidth,
    const msdf_atlas::FontGeometry& fontGeometry,
    const msdfgen::FontMetrics& fontMetrics,
    std::weak_ptr<Font> font) {
    std::vector<Character> parsedLine;
    std::vector<Character> currentWord;

    for (size_t i = 0; i < line.size(); i++) {
        char character = line[i];

        // current glyph
        auto glyph = fontGeometry.getGlyph(character);
        // unknwon glyph
        if (!glyph)
            glyph = fontGeometry.getGlyph('?');

        // get position coords on screen
        std::array<math::vec2, 4> quadUV =
            get_plane_position(glyph, x, y, fsScale);
        // get subtexture coords
        std::array<math::vec2, 4> texCoords =
            font.lock()->get_char_subtexture_positions()[glyph->getCodepoint()];
        // push character
        Character c = {(uint32_t)character, texCoords, quadUV};
        currentWord.push_back(c);

        // calculate x advance to next character
        char nextCharacter = line[i + 1];
        if (i < line.size()) {
            advance_x(character, nextCharacter, x, fsScale, fontGeometry);
        }

        // current word is exceeding bounds
        if (maxWidth != -1.0f && x > maxWidth - g_horizontalPadding) {
            // move cursor to new line
            x = g_horizontalPadding;
            y -= lineHeight;

            // push all currentWord characters to new line
            push_to_new_line(currentWord, nextCharacter, x, y, fsScale,
                             fontGeometry);

            g_lineCount++;
        }

        // current word is finished or line is finished
        if (glyph->isWhitespace() || i == line.size() - 1) {
            // move current word into parsed line
            parsedLine.insert(parsedLine.end(),
                              std::make_move_iterator(currentWord.begin()),
                              std::make_move_iterator(currentWord.end()));
            currentWord.clear();
        }
    }

    return parsedLine;
}

WrappedText::WrappedText(const std::string& text,
                         std::weak_ptr<Font> font,
                         float maxWidth,
                         float lineHeight,
                         float horizontalPadding) {
    m_Font = font;
    m_Size.x = maxWidth;

    g_lineCount = 0;
    g_horizontalPadding = horizontalPadding;

    auto fontPtr = font.lock();
    const auto& fontGeometry = fontPtr->get_msdf_data()->fontGeometry;
    const auto& metrics = fontGeometry.getMetrics();

    double x = horizontalPadding;
    // double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
    double y = 0.0;
    float lineHeightOffset = 0.0f;
    // m_LineHeight = fsScale * metrics.lineHeight + lineHeightOffset;

    m_LineHeight = lineHeight;
    double fsScale = m_LineHeight / metrics.lineHeight - lineHeightOffset;

    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line)) {
        auto lineCharacters =
            parse_line(line, x, y, fsScale, m_LineHeight, lineHeightOffset,
                       maxWidth, fontGeometry, metrics, m_Font);
        m_Characters.insert(m_Characters.end(),
                            std::make_move_iterator(lineCharacters.begin()),
                            std::make_move_iterator(lineCharacters.end()));

        // start at new line
        x = horizontalPadding;
        y -= m_LineHeight;
        g_lineCount++;
    }

    m_Size.y = g_lineCount * m_LineHeight;

    // adjust positions to be centered around 0,0
    // 0,0 is initially at baseline of fst character, so needs to
    // be adjusted by descender and 1 lineheight
    math::vec2 centerOffset = {
        m_Size.x / 2.0f,
        m_Size.y / 2.0f - (metrics.descenderY * fsScale) - m_LineHeight};
    for (auto& c : m_Characters) {
        for (int i = 0; i < 4; i++) {
            c.quadPosition_uv[i].x -= centerOffset.x;
            c.quadPosition_uv[i].y += centerOffset.y;
        }
    }
}

}  // namespace rr::graphics

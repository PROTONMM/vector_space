#include "vector_font.h"

#include <cctype>

namespace {

// Every pair of digits is an x,y point on a 4x6 grid. A slash lifts the pen.
// Glyphs are designed individually instead of being constrained to a segment
// display, preserving the correct shapes of letters such as M, W, X, and Y.
const char* GlyphPath(char raw) {
    switch ((char)std::toupper((unsigned char)raw)) {
        case 'A': return "0602204246/0343";
        case 'B': return "06003041423303/033344453606";
        case 'C': return "4030100006163646";
        case 'D': return "06003041453606";
        case 'E': return "40000646/0333";
        case 'F': return "060040/0333";
        case 'G': return "40301000061636464323";
        case 'H': return "0006/4046/0343";
        case 'I': return "0040/2026/0646";
        case 'J': return "0040/4045160605";
        case 'K': return "0006/400346";
        case 'L': return "000646";
        case 'M': return "0600234046";
        case 'N': return "06004640";
        case 'O': return "103041453616050110";
        case 'P': return "06003041423303";
        case 'Q': return "103041453616050110/2346";
        case 'R': return "06003041423303/3346";
        case 'S': return "4030100102133344453606";
        case 'T': return "0040/2026";
        case 'U': return "000516364540";
        case 'V': return "0016263640";
        case 'W': return "0006234640";
        case 'X': return "0046/4006";
        case 'Y': return "002340/2326";
        case 'Z': return "00400646";

        case '0': return "103041453616050110/0145";
        case '1': return "1020/2026/0646";
        case '2': return "011030414223060646";
        case '3': return "003041423323/233344453606";
        case '4': return "300343/4046";
        case '5': return "4000033344453606";
        case '6': return "4030100105163645443303";
        case '7': return "004016";
        case '8': return "103041453616050110/0343";
        case '9': return "034342413010010203/4346";
        case '-': return "0343";
        case '.': return "0626";
        case ':': return "0222/0424";
        case '/': return "4006";
        default: return "";
    }
}

void DrawPath(SDL_Renderer* renderer, const char* path, float x, float y,
              float unit) {
    bool havePrevious = false;
    int previousX = 0;
    int previousY = 0;
    for (const char* p = path; *p;) {
        if (*p == '/') {
            havePrevious = false;
            ++p;
            continue;
        }
        if (!std::isdigit((unsigned char)p[0]) || !std::isdigit((unsigned char)p[1])) {
            ++p;
            continue;
        }
        const int px = p[0] - '0';
        const int py = p[1] - '0';
        if (havePrevious) {
            SDL_RenderDrawLine(renderer,
                (int)(x + previousX * unit), (int)(y + previousY * unit),
                (int)(x + px * unit), (int)(y + py * unit));
        }
        previousX = px;
        previousY = py;
        havePrevious = true;
        p += 2;
    }
}

} // namespace

float VectorTextWidth(const char* text, float height) {
    if (!text || !*text) return 0.0f;
    int count = 0;
    for (const char* p = text; *p; ++p) ++count;
    return (count * 5.0f - 1.0f) * height / 6.0f;
}

void DrawVectorText(SDL_Renderer* renderer, const char* text, float x, float y,
                    float height, Color color) {
    const float unit = height / 6.0f;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    for (const char* p = text; p && *p; ++p) {
        DrawPath(renderer, GlyphPath(*p), x, y, unit);
        x += unit * 5.0f;
    }
}

void DrawVectorTextCentered(SDL_Renderer* renderer, const char* text, float cx,
                            float y, float height, Color color) {
    DrawVectorText(renderer, text, cx - VectorTextWidth(text, height) * 0.5f,
                   y, height, color);
}

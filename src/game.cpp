#include "game.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "asteroid.h"
#include "bullet.h"
#include "particle.h"
#include "pirate.h"
#include "ship.h"
#include "vector_font.h"

namespace {
constexpr Color kGreen{0x72, 0xff, 0x88};
constexpr Color kBright{0xcc, 0xff, 0xd4};
constexpr float kPi = 3.14159265f;

float DistanceSquared(const Vec2& a, const Vec2& b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    return dx * dx + dy * dy;
}
}

Game::Game() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return;
    }
    m_window = SDL_CreateWindow("Vector Space",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        kCanvasWidth, kCanvasHeight, SDL_WINDOW_RESIZABLE);
    if (!m_window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return;
    }
    if (SDL_Surface* icon = SDL_LoadBMP("img/icon.bmp")) {
        SDL_SetWindowIcon(m_window, icon);
        SDL_FreeSurface(icon);
    }
    m_renderer = SDL_CreateRenderer(m_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer) {
        // Useful on systems such as Raspberry Pi and sessions without 3D acceleration.
        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE);
        if (!m_renderer) {
            std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
            return;
        }
    }
    SDL_RenderSetLogicalSize(m_renderer, kCanvasWidth, kCanvasHeight);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    m_canvas = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET, kCanvasWidth, kCanvasHeight);
    m_bloom = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET, kCanvasWidth, kCanvasHeight);
    m_glow = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET, kCanvasWidth / 4, kCanvasHeight / 4);
    if (!m_canvas || !m_bloom || !m_glow) {
        std::fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        return;
    }
    SDL_SetTextureBlendMode(m_bloom, SDL_BLENDMODE_ADD);
    SDL_SetTextureBlendMode(m_glow, SDL_BLENDMODE_ADD);
    std::srand((unsigned)SDL_GetTicks());
    m_ship = new Ship();
    m_pirate = new Pirate();
    m_ship->Reset(kCanvasWidth * 0.5f, kCanvasHeight * 0.5f);
    m_ship->alive = false;
    m_running = true;
}

Game::~Game() {
    delete m_pirate;
    delete m_ship;
    SDL_DestroyTexture(m_canvas);
    SDL_DestroyTexture(m_bloom);
    SDL_DestroyTexture(m_glow);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void Game::StartGame() {
    m_score = 0;
    m_lives = 3;
    m_level = 0;
    m_nextExtraLife = 10000;
    m_flashTimer = 0.0f;
    m_pirateTimer = 8.0f + (std::rand() % 500) / 100.0f;
    m_asteroids.clear();
    m_bullets.clear();
    m_particles.clear();
    m_pirate->active = false;
    m_ship->Reset(kCanvasWidth * 0.5f, kCanvasHeight * 0.5f);
    m_invulnerable = 3.0f;
    m_state = State::WaveIntro;
    m_stateTimer = 1.8f;
    SpawnWave(1);
}

void Game::Reship() {
    m_ship->Reset(kCanvasWidth * 0.5f, kCanvasHeight * 0.5f);
    m_bullets.clear();
    m_invulnerable = 3.0f;
    m_state = State::Playing;
}

bool Game::CenterIsSafe() const {
    const Vec2 center{kCanvasWidth * 0.5f, kCanvasHeight * 0.5f};
    for (const Asteroid& asteroid : m_asteroids) {
        const float safeRadius = asteroid.radius + 95.0f;
        if (DistanceSquared(center, asteroid.pos) < safeRadius * safeRadius) return false;
    }
    return true;
}

void Game::DestroyShip() {
    m_ship->alive = false;
    --m_lives;
    for (int i = 0; i < 28; ++i) {
        Particle p;
        p.pos = m_ship->pos;
        const float angle = (std::rand() % 360) * kPi / 180.0f;
        const float speed = 80.0f + (std::rand() % 180);
        p.vel = {std::cos(angle) * speed, std::sin(angle) * speed};
        p.life = p.maxLife = 1.1f;
        m_particles.push_back(p);
    }
    if (m_lives <= 0) {
        m_state = State::GameOver;
        m_stateTimer = 1.0f;
        m_hiScore = std::max(m_hiScore, m_score);
    } else {
        m_state = State::Respawning;
        m_stateTimer = 1.8f;
    }
}

void Game::SpawnAsteroidAt(float x, float y, int size) {
    Asteroid asteroid;
    asteroid.Reset(x, y, size, (uint32_t)std::rand());
    m_asteroids.push_back(asteroid);
}

void Game::SpawnPirate() {
    const bool small = m_level >= 3 && (m_score >= 4000 || (std::rand() % 100) < m_level * 7);
    m_pirate->Spawn((std::rand() & 1) != 0, small, m_level);
}

void Game::FirePirateShot() {
    if (!m_pirate->active || !m_ship->alive) return;
    const float target = std::atan2(m_ship->pos.y - m_pirate->pos.y,
                                    m_ship->pos.x - m_pirate->pos.x);
    // The large saucer fires erratically; the small one becomes very accurate.
    const float spread = m_pirate->small ? std::max(0.08f, 0.48f - m_level * 0.035f)
                                         : 1.45f;
    const float error = ((std::rand() % 2001) / 1000.0f - 1.0f) * spread;
    const float angle = target + error;
    Bullet shot;
    shot.pos = m_pirate->pos;
    shot.vel = {std::cos(angle) * 330.0f, std::sin(angle) * 330.0f};
    shot.life = 2.1f;
    shot.hostile = true;
    m_bullets.push_back(shot);
    m_pirate->fireTimer = m_pirate->small ? 0.72f : 1.25f;
}

void Game::SpawnWave(int level) {
    m_level = level;
    // Classic arcade progression: 4, 6, 8, then 10 large rocks.
    const int count = level == 1 ? 4 : level == 2 ? 6 : level == 3 ? 8 : 10;
    const Vec2 center{kCanvasWidth * 0.5f, kCanvasHeight * 0.5f};
    for (int i = 0; i < count; ++i) {
        Vec2 candidate;
        do {
            candidate = {(float)(std::rand() % kCanvasWidth),
                         (float)(std::rand() % kCanvasHeight)};
        } while (DistanceSquared(candidate, center) < 190.0f * 190.0f);
        SpawnAsteroidAt(candidate.x, candidate.y, Asteroid::Large);
    }
}

void Game::EventLoop() {
    Uint64 previous = SDL_GetPerformanceCounter();
    while (m_running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) m_running = false;
            if (event.type != SDL_KEYDOWN || event.key.repeat) continue;
            const SDL_Keycode key = event.key.keysym.sym;
            if (key == SDLK_ESCAPE) m_running = false;
            if (key == SDLK_p && m_state != State::Title && m_state != State::GameOver)
                m_paused = !m_paused;
            if ((key == SDLK_RETURN || key == SDLK_SPACE || key == SDLK_r) &&
                (m_state == State::Title || m_state == State::GameOver)) {
                m_paused = false;
                StartGame();
            }
        }
        if (!m_running) break;

        const Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)(now - previous) / (float)SDL_GetPerformanceFrequency();
        previous = now;
        dt = std::min(dt, 0.05f);
        if (!m_paused) Update(dt);
        Render();
    }
}

void Game::Update(float dt) {
    for (Particle& particle : m_particles) particle.Update(dt);
    m_particles.erase(std::remove_if(m_particles.begin(), m_particles.end(),
        [](const Particle& p) { return !p.alive; }), m_particles.end());
    if (m_state == State::Title || m_state == State::GameOver) return;
    if (m_flashTimer > 0.0f) m_flashTimer -= dt;

    for (Asteroid& asteroid : m_asteroids) asteroid.Update(dt);
    if (m_pirate->active) {
        m_pirate->Update(dt);
        if (m_pirate->active && m_pirate->fireTimer <= 0.0f) FirePirateShot();
        if (!m_pirate->active)
            m_pirateTimer = std::max(6.0f, 15.0f - m_level * 0.45f) +
                            (std::rand() % 500) / 100.0f;
    } else {
        m_pirateTimer -= dt;
        if (m_pirateTimer <= 0.0f && m_state == State::Playing) SpawnPirate();
    }
    for (Bullet& bullet : m_bullets) bullet.Update(dt);
    m_bullets.erase(std::remove_if(m_bullets.begin(), m_bullets.end(),
        [](const Bullet& b) { return !b.alive; }), m_bullets.end());

    if (m_state == State::WaveIntro) {
        m_stateTimer -= dt;
        if (m_stateTimer <= 0.0f) m_state = State::Playing;
    } else if (m_state == State::Respawning) {
        m_stateTimer -= dt;
        if ((m_stateTimer <= 0.0f && CenterIsSafe()) || m_stateTimer < -2.0f) Reship();
    }

    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    if (m_ship->alive && (m_state == State::Playing || m_state == State::WaveIntro)) {
        m_ship->Update(dt, keys[SDL_SCANCODE_UP], keys[SDL_SCANCODE_LEFT],
                       keys[SDL_SCANCODE_RIGHT]);
        m_fireCooldown -= dt;
        if ((keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_W]) && m_fireCooldown <= 0.0f) {
            Bullet bullet;
            const float muzzle = m_ship->radius * 1.55f;
            bullet.pos = {m_ship->pos.x + std::cos(m_ship->angle) * muzzle,
                          m_ship->pos.y + std::sin(m_ship->angle) * muzzle};
            bullet.vel = {std::cos(m_ship->angle) * 500.0f + m_ship->vel.x,
                          std::sin(m_ship->angle) * 500.0f + m_ship->vel.y};
            bullet.life = 1.25f;
            m_bullets.push_back(bullet);
            m_fireCooldown = 0.22f;
        }
    }

    struct Split { Vec2 pos; int size; };
    std::vector<Split> splits;
    for (Bullet& bullet : m_bullets) {
        if (!bullet.alive || bullet.hostile) continue;
        for (Asteroid& asteroid : m_asteroids) {
            if (!asteroid.alive || DistanceSquared(bullet.pos, asteroid.pos) >=
                asteroid.radius * asteroid.radius) continue;
            bullet.alive = false;
            asteroid.alive = false;
            m_score += asteroid.size == Asteroid::Large ? 20 :
                       asteroid.size == Asteroid::Medium ? 50 : 100;
            if (asteroid.size > Asteroid::Small)
                splits.push_back({asteroid.pos, asteroid.size - 1});
            for (int i = 0; i < 12; ++i) {
                Particle p;
                p.pos = asteroid.pos;
                const float angle = (std::rand() % 360) * kPi / 180.0f;
                const float speed = 60.0f + std::rand() % 130;
                p.vel = {std::cos(angle) * speed, std::sin(angle) * speed};
                p.life = p.maxLife = 0.65f;
                m_particles.push_back(p);
            }
            break;
        }
    }
    // The saucer is a separate high-value target.
    if (m_pirate->active) {
        for (Bullet& bullet : m_bullets) {
            if (!bullet.alive || bullet.hostile) continue;
            if (DistanceSquared(bullet.pos, m_pirate->pos) >=
                m_pirate->radius * m_pirate->radius) continue;
            bullet.alive = false;
            m_pirate->active = false;
            m_score += m_pirate->small ? 1000 : 200;
            m_pirateTimer = std::max(6.0f, 14.0f - m_level * 0.4f) +
                            (std::rand() % 500) / 100.0f;
            for (int i = 0; i < 18; ++i) {
                Particle p;
                p.pos = m_pirate->pos;
                const float angle = (std::rand() % 360) * kPi / 180.0f;
                const float speed = 70.0f + std::rand() % 150;
                p.vel = {std::cos(angle) * speed, std::sin(angle) * speed};
                p.life = p.maxLife = 0.8f;
                m_particles.push_back(p);
            }
            break;
        }
    }
    for (const Split& split : splits)
        for (int i = 0; i < 2; ++i) SpawnAsteroidAt(split.pos.x, split.pos.y, split.size);
    m_asteroids.erase(std::remove_if(m_asteroids.begin(), m_asteroids.end(),
        [](const Asteroid& a) { return !a.alive; }), m_asteroids.end());
    m_bullets.erase(std::remove_if(m_bullets.begin(), m_bullets.end(),
        [](const Bullet& b) { return !b.alive; }), m_bullets.end());

    while (m_score >= m_nextExtraLife) {
        ++m_lives;
        m_nextExtraLife += 10000;
        m_flashTimer = 2.0f;
    }
    if (m_invulnerable > 0.0f) m_invulnerable -= dt;
    if (m_ship->alive && m_invulnerable <= 0.0f) {
        bool destroyed = false;
        for (const Asteroid& asteroid : m_asteroids) {
            const float hitRadius = asteroid.radius + m_ship->radius;
            if (DistanceSquared(m_ship->pos, asteroid.pos) < hitRadius * hitRadius) {
                DestroyShip();
                destroyed = true;
                break;
            }
        }
        if (!destroyed && m_pirate->active) {
            const float hitRadius = m_pirate->radius + m_ship->radius;
            if (DistanceSquared(m_ship->pos, m_pirate->pos) < hitRadius * hitRadius) {
                DestroyShip();
                destroyed = true;
            }
        }
        if (!destroyed) {
            for (Bullet& bullet : m_bullets) {
                if (!bullet.alive || !bullet.hostile) continue;
                const float hitRadius = m_ship->radius + 3.0f;
                if (DistanceSquared(m_ship->pos, bullet.pos) < hitRadius * hitRadius) {
                    bullet.alive = false;
                    DestroyShip();
                    break;
                }
            }
        }
    }
    if (m_asteroids.empty() && m_state != State::Respawning) {
        SpawnWave(m_level + 1);
        m_state = State::WaveIntro;
        m_stateTimer = 2.0f;
        if (!m_ship->alive) Reship();
    }
}

void Game::RenderWorld() {
    for (const Asteroid& asteroid : m_asteroids) asteroid.Draw(m_renderer);
    m_pirate->Draw(m_renderer);
    for (const Bullet& bullet : m_bullets) bullet.Draw(m_renderer);
    if (m_ship->alive && (m_invulnerable <= 0.0f || ((int)(m_invulnerable * 8.0f) & 1)))
        m_ship->Draw(m_renderer);
    RenderParticles();
    RenderHud();
}

void Game::RenderHud() {
    char text[64];
    if (m_state != State::Title) {
        std::snprintf(text, sizeof(text), "%06d", m_score);
        DrawVectorText(m_renderer, text, 24, 22, 24, kGreen);
        std::snprintf(text, sizeof(text), "HI %06d", m_hiScore);
        DrawVectorText(m_renderer, text, kCanvasWidth - VectorTextWidth(text, 18) - 24,
                       24, 18, kGreen);
        std::snprintf(text, sizeof(text), "WAVE %d", m_level);
        DrawVectorTextCentered(m_renderer, text, kCanvasWidth * 0.5f,
                               kCanvasHeight - 42, 17, kGreen);
        SDL_SetRenderDrawColor(m_renderer, kGreen.r, kGreen.g, kGreen.b, 255);
        // Show the complete life count. Previously this displayed only reserve
        // ships, which made a 3 -> 4 bonus look like a 2 -> 3 transition.
        for (int i = 0; i < std::max(0, m_lives); ++i) {
            const int x = 30 + i * 23;
            const int y = kCanvasHeight - 34;
            SDL_RenderDrawLine(m_renderer, x, y + 12, x + 7, y - 8);
            SDL_RenderDrawLine(m_renderer, x + 7, y - 8, x + 14, y + 12);
            SDL_RenderDrawLine(m_renderer, x + 14, y + 12, x + 7, y + 7);
            SDL_RenderDrawLine(m_renderer, x + 7, y + 7, x, y + 12);
        }
        if (m_flashTimer > 0.0f)
            DrawVectorTextCentered(m_renderer, "EXTRA LIFE", kCanvasWidth * 0.5f,
                                   76, 18, kBright);
    }
    if (m_state == State::Title) {
        DrawVectorTextCentered(m_renderer, "VECTOR", kCanvasWidth * 0.5f, 250, 52, kBright);
        DrawVectorTextCentered(m_renderer, "SPACE", kCanvasWidth * 0.5f, 335, 48, kGreen);
        DrawVectorTextCentered(m_renderer, "PRESS SPACE", kCanvasWidth * 0.5f, 535, 24, kGreen);
        DrawVectorTextCentered(m_renderer, "ARROWS TURN THRUST", kCanvasWidth * 0.5f, 650, 16, kGreen);
        DrawVectorTextCentered(m_renderer, "SPACE FIRE  P PAUSE", kCanvasWidth * 0.5f, 685, 16, kGreen);
    } else if (m_state == State::WaveIntro) {
        std::snprintf(text, sizeof(text), "WAVE %d", m_level);
        DrawVectorTextCentered(m_renderer, text, kCanvasWidth * 0.5f, 450, 34, kBright);
    } else if (m_state == State::Respawning) {
        DrawVectorTextCentered(m_renderer, "GET READY", kCanvasWidth * 0.5f, 450, 27, kBright);
    } else if (m_state == State::GameOver) {
        DrawVectorTextCentered(m_renderer, "GAME OVER", kCanvasWidth * 0.5f, 430, 40, kBright);
        DrawVectorTextCentered(m_renderer, "PRESS SPACE", kCanvasWidth * 0.5f, 505, 23, kGreen);
    }
    if (m_paused)
        DrawVectorTextCentered(m_renderer, "PAUSED", kCanvasWidth * 0.5f, 450, 38, kBright);
}

void Game::Render() {
    // Draw every luminous element once at full resolution. This texture is
    // reused both as the crisp vector core and as the source of the glow.
    SDL_SetRenderTarget(m_renderer, m_bloom);
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
    SDL_RenderClear(m_renderer);
    RenderWorld();

    // Downsampling the vector layer and filtering it back up produces a soft,
    // even halo around each line without displacing the scene geometry.
    SDL_RenderSetLogicalSize(m_renderer, 0, 0);
    SDL_SetRenderTarget(m_renderer, m_glow);
    // Keep this intermediate layer opaque. With a transparent background,
    // downsampling reduces both RGB and alpha, causing the glow intensity to
    // be multiplied twice when it is blended back into the scene.
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);
    SDL_SetTextureAlphaMod(m_bloom, 255);
    SDL_SetTextureBlendMode(m_bloom, SDL_BLENDMODE_ADD);
    SDL_RenderCopy(m_renderer, m_bloom, nullptr, nullptr);

    SDL_SetRenderTarget(m_renderer, m_canvas);
    SDL_SetRenderDrawColor(m_renderer, 1, 6, 3, 255);
    SDL_RenderClear(m_renderer);
    for (int i = 0; i < 75; ++i) {
        const uint32_t h = (uint32_t)i * 2654435761u + 0x9e3779b9u;
        SDL_SetRenderDrawColor(m_renderer, 5, 22, 10, 255);
        SDL_RenderDrawPoint(m_renderer, h % kCanvasWidth, (h >> 12) % kCanvasHeight);
    }

    // Two low-intensity passes add a restrained neon bloom. The expanded pass
    // creates the outer aura; the regular pass reinforces the glow near lines.
    SDL_SetTextureAlphaMod(m_glow, 72);
    const SDL_Rect outerGlow{-5, -5, kCanvasWidth + 10, kCanvasHeight + 10};
    SDL_RenderCopy(m_renderer, m_glow, nullptr, &outerGlow);
    SDL_SetTextureAlphaMod(m_glow, 140);
    SDL_RenderCopy(m_renderer, m_glow, nullptr, nullptr);

    // A faint one-pixel ring keeps the neon effect visible on software
    // renderers where texture filtering may be limited or disabled.
    SDL_SetTextureAlphaMod(m_bloom, 30);
    const SDL_Rect nearGlow[] = {
        {-2, 0, kCanvasWidth, kCanvasHeight},
        { 2, 0, kCanvasWidth, kCanvasHeight},
        { 0,-2, kCanvasWidth, kCanvasHeight},
        { 0, 2, kCanvasWidth, kCanvasHeight},
        {-1,-1, kCanvasWidth, kCanvasHeight},
        { 1,-1, kCanvasWidth, kCanvasHeight},
        {-1, 1, kCanvasWidth, kCanvasHeight},
        { 1, 1, kCanvasWidth, kCanvasHeight}
    };
    for (const SDL_Rect& rect : nearGlow)
        SDL_RenderCopy(m_renderer, m_bloom, nullptr, &rect);

    SDL_SetTextureAlphaMod(m_bloom, 255);
    SDL_RenderCopy(m_renderer, m_bloom, nullptr, nullptr);

    SDL_SetRenderTarget(m_renderer, nullptr);
    SDL_RenderSetLogicalSize(m_renderer, kCanvasWidth, kCanvasHeight);
    SDL_RenderCopy(m_renderer, m_canvas, nullptr, nullptr);
    SDL_RenderPresent(m_renderer);
}

void Game::RenderParticles() {
    for (const Particle& particle : m_particles) particle.Draw(m_renderer);
}

void Game::Run() { EventLoop(); }

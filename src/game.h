#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <cstdint>
#include <vector>

#include "vector.h"

// Main game class: owns the loop, state, entities, and rendering.
class Game {
public:
    Game();
    ~Game();

    void Run();

private:
    enum class State { Title, Playing, WaveIntro, Respawning, GameOver };

    void EventLoop();
    void Update(float dt);
    void Render();
    void RenderWorld();
    void RenderHud();
    void StartGame();

    void SpawnWave(int level);
    void SpawnAsteroidAt(float x, float y, int size);
    void Reship();
    void DestroyShip();
    bool CenterIsSafe() const;
    void SpawnPirate();
    void FirePirateShot();

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture* m_canvas = nullptr;   // final render buffer
    SDL_Texture* m_bloom = nullptr;    // blurred phosphor glow layer

    bool m_running = false;

    // Game state
    int m_score = 0;
    int m_lives = 3;
    int m_level = 0;
    int m_hiScore = 0;
    bool m_paused = false;
    State m_state = State::Title;

    // Timers
    float m_fireCooldown = 0.0f;
    float m_invulnerable = 0.0f;
    float m_spawnTimer = 0.0f;
    float m_flashTimer = 0.0f;
    float m_stateTimer = 0.0f;
    float m_pirateTimer = 10.0f;
    int m_nextExtraLife = 10000;

    // Entities
    class Ship* m_ship = nullptr;
    class Pirate* m_pirate = nullptr;
    std::vector<class Asteroid> m_asteroids;
    std::vector<class Bullet> m_bullets;
    std::vector<class Particle> m_particles;

    // Input state
    bool m_thrusting = false;
    bool m_left = false;
    bool m_right = false;

    void UpdateParticles(float dt);
    void RenderParticles();
};

#endif // GAME_H

#include <SFML/Graphics.hpp>
#include <optional>
#include <iostream>
#include <vector>
#include <ctime>   // Required for system clock (random seed)
#include <cstdlib> // Required for rand()
#include <cmath>

struct SimulationConfig {
    const float bounceY = 0.7f;
    const float bounceX = 0.9f;
    const float groundFriction = 0.95f;
    const float gravity = 9.81f;
    const float explosionForce = 3000.0f;
    const float friction = 0.999f;
    const float explosionRadius = 100.f;
    const float obstacleBounce = 0.8f;
};

// Define the particle structure
struct Particle {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float lifeTime{};
    float maxLifeTime{};

    Particle(sf::Vector2f pos, sf::Vector2f vel, float life) {
        float radius = 2.f;
        shape.setRadius(radius);
        shape.setOrigin({ radius,radius });
        shape.setFillColor(sf::Color::Cyan);
        shape.setPosition(pos);
        velocity = vel;
        lifeTime = life;
        maxLifeTime = life;
    }

    void update(float deltaTime, sf::Vector2u windowSize, const SimulationConfig& config) {

        velocity.y += (config.gravity * deltaTime);

        // Displacement based on delta time
        shape.move(velocity * deltaTime);

        // 2. Boundary bounds checking (Reflection/Bounce)
        sf::Vector2f pos = shape.getPosition();
        float radius = shape.getRadius();

        // --- HORIZONTAL BOUNDS CHECK (X) ---
        sf::Vector2f currentPos = shape.getPosition(); // Get updated position

        if (currentPos.x < 0) {
            shape.setPosition({ 0.f, currentPos.y }); // Clamp to left border, maintain current Y
            velocity.x = std::abs(velocity.x) * config.bounceX;
        }
        else if (currentPos.x + (radius * 2) > windowSize.x) {
            shape.setPosition({ windowSize.x - (radius * 2), currentPos.y }); // Clamp to right border
            velocity.x = -std::abs(velocity.x) * config.bounceX;
        }

        // --- VERTICAL BOUNDS CHECK (Y) ---
        currentPos = shape.getPosition(); // Re-sync position to prevent jitter/clipping

        if (currentPos.y < 0) {
            shape.setPosition({ currentPos.x, 0.f }); // Clamp to top border
            velocity.y = std::abs(velocity.y);
        }
        else if (currentPos.y + (radius * 2) > windowSize.y) {
            shape.setPosition({ currentPos.x, windowSize.y - (radius * 2) }); // Clamp to bottom border

            if (std::abs(velocity.y) > 15.0f) {
                velocity.y = -std::abs(velocity.y) * config.bounceY;
            }
            else {
                velocity.y = 0;
            }
            velocity.x *= config.groundFriction;
        }

        // Air resistance / Drag force
        velocity *= config.friction;

        // Obstacle collision response
        sf::Vector2f obsPos = { 400.f, 450.f };
        float obsRadius = 50.f;

        float dxObs = shape.getPosition().x - obsPos.x;
        float dyObs = shape.getPosition().y - obsPos.y;
        float distObs = std::sqrt(dxObs * dxObs + dyObs * dyObs);

        if (distObs < obsRadius + shape.getRadius()) {
            float nx = dxObs / distObs;
            float ny = dyObs / distObs;

            // Resolve penetration (positional correction)
            shape.setPosition({ obsPos.x + nx * (obsRadius + shape.getRadius()),
                                obsPos.y + ny * (obsRadius + shape.getRadius()) });

            // Calculate impulse vector using dot product
            float dotProduct = velocity.x * nx + velocity.y * ny;

            // Reflect velocity vector across the collision normal
            velocity.x -= 2 * dotProduct * nx;
            velocity.y -= 2 * dotProduct * ny;

            velocity *= config.obstacleBounce;
        }

        lifeTime -= deltaTime;

        float lifeRatio = lifeTime / maxLifeTime;
        if (lifeRatio < 0) lifeRatio = 0;

        // Calculate current speed magnitude
        float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);

        // Normalize speed (0.0 = stationary, 1.0 = maximum velocity)
        float maxSpeed = 300.0f;
        float ratio = speed / maxSpeed;

        if (ratio > 1.0f) ratio = 1.0f; // Cap threshold to prevent color interpolation overflow

        // Compute dynamic color shifting (blue = slow, red = fast)
        unsigned char r = static_cast<unsigned char>(255 * ratio);          // Direct correlation with speed scale
        unsigned char b = static_cast<unsigned char>(255 * (1.0f - ratio)); // Inverse correlation with speed scale
        unsigned char g = 100;                                              // Constant green bias to maintain visibility
        unsigned char alpha = static_cast<unsigned char>(255 * lifeRatio);

        shape.setFillColor(sf::Color(r, g, b, alpha));
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Simulatore Particellare v1.0");
    window.setFramerateLimit(144);  // Cap framework tick rate

    SimulationConfig config;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 1. Initialize clock
    sf::Clock clock;

    // 2. Initialize contiguous container for memory pooling
    std::vector<Particle> particles;

    // Populate particle container
    for (int i = 0; i < 500; i++) {

        float vx = (std::rand() % 400) - 200.0f;
        float vy = (std::rand() % 400) - 200.0f;

        // Generate randomized lifespan between 2.0 and 5.0 seconds
        float randomLife = static_cast<float>(std::rand() % 1000) / 100.f + 5.0f;

        particles.push_back(Particle({ 400.f, 300.f }, { vx,vy }, randomLife));
    }

    sf::CircleShape obstacle(50.0f);
    obstacle.setFillColor(sf::Color(100, 100, 100));
    obstacle.setOrigin({ 50.f,50.f });
    obstacle.setPosition({ 400.f, 450.f });

    // Motion blur / Accumulation buffer trail effect
    sf::RectangleShape fade(sf::Vector2f(800.0f, 600.0f));
    fade.setFillColor(sf::Color(0, 0, 0, 40));

    while (window.isOpen()) {
        // 3. Compute DeltaTime (elapsed time since last frame step)
        sf::Time elapsed = clock.restart();
        float dt = elapsed.asSeconds();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            // Get mouse position relative to the application window
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);

            // Cast coordinates to Vector2f for floating-point physics synchronization
            sf::Vector2f mPos(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

            for (auto& p : particles) {
                float dx = p.shape.getPosition().x - mPos.x;
                float dy = p.shape.getPosition().y - (mPos.y + 20.0f);

                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance < config.explosionRadius) {

                    if (distance < 1.0f) distance = 1.0f;

                    // Calculate direction normal (normalized unit vector)
                    float dirX = dx / distance;
                    float dirY = dy / distance;

                    // Apply distance-attenuated shockwave impulse
                    float fattoreVicinanza = (100.0f - distance) / 100.0f;

                    // Radial impulse addition + linear upward lift factor
                    p.velocity.x += dirX * config.explosionForce * fattoreVicinanza * dt;
                    p.velocity.y += (dirY * config.explosionForce - 200.0f) * fattoreVicinanza * dt;
                }
            }
        }

        // Note: window.clear(sf::Color::Black) is omitted to allow accumulation-based trail drawing
        window.draw(fade);
        window.draw(obstacle);

        // 4. Engine logic update loop
        for (auto& p : particles) {
            p.update(dt, window.getSize(), config); // Pass window size context for boundary evaluations

            // Recycle dead particles (Object Pooling reset pattern)
            if (p.lifeTime <= 0) {
                p.shape.setPosition({ 400.0f, 200.0f });

                float vx = (std::rand() % 400) - 200.0f;
                float vy = (std::rand() % 400) - 200.0f;
                p.velocity = { vx,vy };

                p.lifeTime = static_cast<float>(std::rand() % 10 + 5);
                p.maxLifeTime = p.lifeTime;
            }

            window.draw(p.shape);
        }

        window.display();   // Swap buffers to display current frame output
    }

    return 0;
}
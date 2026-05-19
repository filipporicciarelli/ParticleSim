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

        if (ratio > 1.0f) ratio = 1.
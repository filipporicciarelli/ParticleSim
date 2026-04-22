#include <SFML/Graphics.hpp>
#include <optional>
#include <iostream>
#include <vector>
#include <ctime>   // Per l'orologio di sistema (numeri casuali)
#include <cstdlib> // Per rand()
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

// Definisco la struttura della particella
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

        // Spostamento basato sul tempo reale
        shape.move(velocity * deltaTime);

        // 2. Controllo bordi (Rimbalzo)
        sf::Vector2f pos = shape.getPosition();
        float radius = shape.getRadius();

        // --- CONTROLLO ORIZZONTALE(X) ---
        sf::Vector2f currentPos = shape.getPosition(); // Prendo la posizione aggiornata

        if (currentPos.x < 0) {
            shape.setPosition({ 0.f, currentPos.y }); // Blocca a sinistra, mantieni la Y attuale
            velocity.x = std::abs(velocity.x) * config.bounceX;
        }
        else if (currentPos.x + (radius * 2) > windowSize.x) {
            shape.setPosition({ windowSize.x - (radius * 2), currentPos.y }); // Blocca a destra
            velocity.x = -std::abs(velocity.x) * config.bounceX;
        }

        // --- CONTROLLO VERTICALE (Y) ---
        currentPos = shape.getPosition(); // Aggiorno di nuovo la posizione per sicurezza

        if (currentPos.y < 0) {
            shape.setPosition({ currentPos.x, 0.f }); // Blocca in alto
            velocity.y = std::abs(velocity.y);
        }
        else if (currentPos.y + (radius * 2) > windowSize.y) {
            shape.setPosition({ currentPos.x, windowSize.y - (radius * 2) }); // Blocca in basso

            if (std::abs(velocity.y) > 15.0f) {
                velocity.y = -std::abs(velocity.y) * config.bounceY;
            }
            else {
                velocity.y = 0;
            }
            velocity.x *= config.groundFriction;
        }

        //attrito dell'aria
        velocity *= config.friction;

        //interazione con ostacolo
        sf::Vector2f obsPos = { 400.f, 450.f };
        float obsRadius = 50.f;

        float dxObs = shape.getPosition().x - obsPos.x;
        float dyObs = shape.getPosition().y - obsPos.y;
        float distObs = std::sqrt(dxObs * dxObs + dyObs * dyObs);

        if (distObs < obsRadius + shape.getRadius()) {
            float nx = dxObs / distObs;
            float ny = dyObs / distObs;

            shape.setPosition({ obsPos.x + nx * (obsRadius + shape.getRadius()),
                                obsPos.y + ny * (obsRadius + shape.getRadius()) });

            float dotProduct = velocity.x * nx + velocity.y * ny;

            velocity.x -= 2 * dotProduct * nx;
            velocity.y -= 2 * dotProduct * ny;

            velocity *= config.obstacleBounce;
        }

        lifeTime -= deltaTime;

        float lifeRatio = lifeTime / maxLifeTime;
        if (lifeRatio < 0)lifeRatio = 0;

        //calcolo velocità attuale
        float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);

        //normalizzo la velocità (0.0 = fermo, 1.0 = velocissimo)
        float maxSpeed = 300.0f;
        float ratio = speed / maxSpeed;

        if (ratio > 1.0f) ratio = 1.0f; //evito di superare il limite di colore

        //creo colore dinamico (blu = lento, rosso = veloce)
        unsigned char r = static_cast<unsigned char>(255 * ratio);          // Più è veloce, più c'è rosso
        unsigned char b = static_cast<unsigned char>(255 * (1.0f - ratio)); // Più è veloce, meno c'è blu
        unsigned char g = 100;                                              // Un po' di verde costante per renderlo meno spento
        unsigned char alpha = static_cast<unsigned char>(255 * lifeRatio);

        shape.setFillColor(sf::Color(r, g, b, alpha));
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Simulatore Particellare v1.0");
    window.setFramerateLimit(144);  //setta il limite di framerate

    SimulationConfig config;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 1. Creo l'orologio
    sf::Clock clock;

    // 2. creo un contenitore particles che conterrà tutte le particelle 
    std::vector<Particle> particles;

    //riempio il contenitore particles
    for (int i = 0; i < 500; i++) {

        float vx = (std::rand() % 400) - 200.0f;
        float vy = (std::rand() % 400) - 200.0f;

        // Genero una vita casuale tra 2.0 e 5.0 secondi
        float randomLife = static_cast<float>(std::rand() % 1000) / 100.f + 5.0f;

        particles.push_back(Particle({ 400.f, 300.f }, { vx,vy }, randomLife));
    }

    sf::CircleShape obstacle(50.0f);
    obstacle.setFillColor(sf::Color(100,100,100));
    obstacle.setOrigin({ 50.f,50.f });
    obstacle.setPosition({ 400.f, 450.f });

    //effetto scia
    sf::RectangleShape fade(sf::Vector2f(800.0f, 600.0f));
    fade.setFillColor(sf::Color(0, 0, 0, 40));

    while (window.isOpen()) {
        // 3. Calcolo il DeltaTime (tempo trascorso dall'ultimo frame)
        sf::Time elapsed = clock.restart();
        float dt = elapsed.asSeconds();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            //ottiene la posizione del mouse rispetto la finestra
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);

            // Converte in Vector2f (perché le particelle usano i float)
            sf::Vector2f mPos(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

            for (auto& p : particles) {
                float dx = p.shape.getPosition().x - mPos.x;
                float dy = p.shape.getPosition().y - (mPos.y+20.0f);

                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance < config.explosionRadius) {

                    if (distance < 1.0f) distance = 1.0f;

                    // Calcolo la direzione (vettore normalizzato)
                    // Dividendo dx e dy per la distanza, otteniamo valori tra -1 e 1
                    float dirX = dx / distance;
                    float dirY = dy / distance;

                    // Applichiamo una forza d'urto costante
                                        
                    float fattoreVicinanza = (100.0f - distance) / 100.0f;

                    // Spinta radiale + un piccolo bonus verso l'alto per "staccarle" dal suolo
                    p.velocity.x += dirX * config.explosionForce * fattoreVicinanza * dt;
                    p.velocity.y += (dirY * config.explosionForce - 200.0f) * fattoreVicinanza * dt;
                }
            }
        }

        //window.clear(sf::Color::Black); // Cancella il frame precedente se metto questo devo togliere l'effetto scia
        
        window.draw(fade);
        window.draw(obstacle);

        // 4. Aggiorno la logica
        for (auto& p : particles) {
            p.update(dt, window.getSize(), config); // Passo la dimensione attuale della finestra

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

        window.display();   // Mostra tutto quello che abbiamo disegnato finora

        
        
    }

    return 0;
}
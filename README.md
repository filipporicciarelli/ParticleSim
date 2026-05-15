# Particle Physics Sandbox (SFML)

An interactive particle simulator built with **C++** and **SFML**. This project serves as a demonstration of vector mathematics and applied physics in the context of game development.

## 🚀 Key Features

- **Realistic Physics:** Accurate simulation of gravity, air friction (drag), and elastic bounces.
- **Advanced Collisions:** Particles bounce off circular obstacles using vector reflection based on the **Dot Product**.
- **User Interaction:** Click anywhere to trigger radial explosions that realistically repel nearby particles.
- **Optimization:** Implements an **Object Pooling** system to recycle particles, ensuring high performance even with a large number of entities.
- **Visuals:** Dynamic color shifting based on particle velocity and smooth trail effects.

## 🎮 Controls

- **Left Click:** Trigger a radial explosion at the cursor position to push particles away.

## 🛠 Tech Stack

- **Language:** C++ (C++17 or higher)
- **Graphics Library:** SFML 3.0+
- **Mathematics:** Vector Algebra (Reflection, Normalization, Dot Product)

## 🔧 Installation & Usage

1. Ensure **SFML 3.0+** is installed and correctly linked in your project settings.
2. Compile the source files using a C++17 compatible compiler.
3. Run the executable to start the sandbox.

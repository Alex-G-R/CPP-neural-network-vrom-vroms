#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

// Function to generate a random float between min and max
float getRandomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

// Function to generate a random circle
sf::CircleShape generateRandomCircle(const sf::RenderWindow& window) {
    float radius = getRandomFloat(10.f, 50.f);
    float x = getRandomFloat(radius, window.getSize().x - radius);
    float y = getRandomFloat(radius, window.getSize().y - radius);
    sf::Color color(rand() % 256, rand() % 256, rand() % 256);
    sf::CircleShape circle(radius);
    circle.setPosition(x, y);
    circle.setFillColor(color);
    return circle;
}

// Simple neural network representation
struct NeuralNetwork {
    std::vector<float> weights;

    NeuralNetwork(int numInputs, int numOutputs) : weights(numInputs * numOutputs) {
        std::generate(weights.begin(), weights.end(), [&]() { return getRandomFloat(-1.f, 1.f); });
    }

    // Mutation function to adjust weights slightly
    void mutate() {
        for (auto& weight : weights) {
            weight += getRandomFloat(-0.1f, 0.1f); // Small mutation
        }
    }

    // Predicts outputs for given inputs
    std::vector<float> predict(const std::vector<float>& inputs) {
        int numOutputs = weights.size() / inputs.size();
        std::vector<float> outputs(numOutputs, 0.0f);

        for (int i = 0; i < numOutputs; ++i) {
            for (size_t j = 0; j < inputs.size(); ++j) {
                outputs[i] += inputs[j] * weights[i * inputs.size() + j];
            }
        }

        return outputs;
    }
};

// Agent with a car sprite and a neural network
struct Agent {
    sf::Sprite carSprite;
    NeuralNetwork brain;
    float reward = 0.0f;

    Agent(const sf::Texture& texture) : brain(2, 4) {  // 2 inputs (x and y diffs), 4 outputs (up, down, left, right)
        carSprite.setTexture(texture);
        carSprite.setScale(0.1f, 0.1f); // Scale the car to fit the screen
        resetPosition();
    }

    void resetPosition() {
        carSprite.setPosition(400.f, 500.f); // Start position
    }
};

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    sf::RenderWindow window(sf::VideoMode(800, 600), "Simple Racing Game");

    sf::Texture carTexture;
    if (!carTexture.loadFromFile("../assets/car.png")) {
        return EXIT_FAILURE;
    }

    std::vector<Agent> agents;
    const int numAgents = 3000;
    for (int i = 0; i < numAgents; ++i) {
        agents.emplace_back(carTexture);
    }

    sf::Clock generationClock;

    float generationTime = 5.0f;
    int currentGeneration = 1;



    sf::CircleShape circle = generateRandomCircle(window);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        if (generationClock.getElapsedTime().asSeconds() >= generationTime) {
            // Sort agents by reward
            std::sort(agents.begin(), agents.end(), [](const Agent& a, const Agent& b) {
                return a.reward > b.reward;
            });

            // Keep the best agent and mutate
            Agent bestAgent = agents[0];
            bestAgent.brain.mutate();
            bestAgent.reward = 0;
            bestAgent.resetPosition();

            // Replace all agents with mutations of the best one
            agents.clear();
            agents.push_back(bestAgent);

            // Move the circle to a new random position
            circle = generateRandomCircle(window);

            // Reset the clock for the new generation
            generationClock.restart();
            currentGeneration++;
        }

        // Game logic (move agents, calculate rewards, etc.)
        for (auto& agent : agents) {
            float reward = -0.01f; // Penalize taking time

            // Calculate the distance between agent and circle
            sf::Vector2f agentPosition = agent.carSprite.getPosition();
            sf::Vector2f circlePosition = circle.getPosition();
            float dx = circlePosition.x - agentPosition.x;
            float dy = circlePosition.y - agentPosition.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            // Normalize distance to be in the range [0, 1]
            float normalizedDistance = 1.0f - std::min(distance / window.getSize().x, 1.0f);

            // Calculate inputs for the neural network
            std::vector<float> inputs = {dx / window.getSize().x, dy / window.getSize().y};

            // Predict the best action to take
            std::vector<float> actions = agent.brain.predict(inputs);
            float maxAction = *std::max_element(actions.begin(), actions.end());

            // Apply the action
            if (maxAction == actions[0])
                agent.carSprite.move(0, -1);
            else if (maxAction == actions[1])
                agent.carSprite.move(0, 1);
            else if (maxAction == actions[2])
                agent.carSprite.move(-1, 0);
            else if (maxAction == actions[3])
                agent.carSprite.move(1, 0);

            // Ensure the car stays within the window boundaries
            sf::FloatRect bounds = agent.carSprite.getGlobalBounds();
            if (bounds.left < 0 || bounds.left + bounds.width > window.getSize().x ||
                bounds.top < 0 || bounds.top + bounds.height > window.getSize().y) {
                agent.reward -= 2.0f; // Penalize for hitting the boundary
                agent.resetPosition();
            }

            // Reward for getting closer to the circle
            reward += normalizedDistance;
            agent.reward += reward;
        }

        // Clear the screen
        window.clear();

        // Draw agents and circle
        for (auto& agent : agents) {
            window.draw(agent.carSprite);
        }
        window.draw(circle);

        // Display generation info
        sf::Font font;
        if (!font.loadFromFile("../assets/arial.ttf")) {
            return EXIT_FAILURE;
        }
        sf::Text genText("Generation: " + std::to_string(currentGeneration), font, 20);
        genText.setFillColor(sf::Color::White);
        genText.setPosition(10, 10);
        window.draw(genText);

        // Update the window
        window.display();
    }

    return EXIT_SUCCESS;
}

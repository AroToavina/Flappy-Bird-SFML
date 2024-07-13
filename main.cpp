#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <sstream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <memory>

using namespace sf;
using namespace std;

class Pipe {
public:
    Sprite topPipe, bottomPipe;
    float speed;
    bool passed;

    Pipe(Texture &topTexture, Texture &bottomTexture, float posX, float posY, float pipeGap, float pipeSpeed) {
        topPipe.setTexture(topTexture);
        bottomPipe.setTexture(bottomTexture);

        topPipe.setPosition(posX, posY - topTexture.getSize().y);
        bottomPipe.setPosition(posX, posY + pipeGap);

        speed = pipeSpeed;
        passed = false;
    }

    void update(float deltaTime) {
        topPipe.move(-speed * deltaTime, 0);
        bottomPipe.move(-speed * deltaTime, 0);
    }

    void draw(RenderWindow &window) {
        window.draw(topPipe);
        window.draw(bottomPipe);
    }

    bool isOffScreen() {
        return topPipe.getPosition().x + topPipe.getGlobalBounds().width < 0;
    }

    float getXPosition() {
        return topPipe.getPosition().x;
    }

    bool collidesWithBird(Sprite &bird) {
        return bird.getGlobalBounds().intersects(topPipe.getGlobalBounds()) ||
               bird.getGlobalBounds().intersects(bottomPipe.getGlobalBounds());
    }

    bool hasCollided() const {
        return collided;
    }

    void setCollided(bool value) {
        collided = value;
    }

private:
    bool collided = false;
};

enum GameState { StartScreen, Playing, GameOver };

void resetGame(int &life, int &score, float &bird_y, float &velocity, vector<Pipe> &pipes, stringstream &str_score, Text &score_set, stringstream &str_life, Text &life_set) {
    life = 3;
    score = 0;
    bird_y = 250;
    velocity = 0;
    pipes.clear();
    str_score.str("");
    str_score << score;
    score_set.setString("score: " + str_score.str());
    str_life.str("");
    str_life << life;
    life_set.setString("x" + str_life.str());
}

int main() {
    srand(static_cast<unsigned>(time(0)));

    const int screen_width = 800;
    const int screen_height = 600;

    int life = 3;
    float velocity = 0;
    float bird_y = 250;
    const float jump_velocity = -10;
    const float gravity = 0.5;
    int score = 0;

    Event event;
    Texture bird, background, heart, pipeUp, pipeDown;
    Font font;
    SoundBuffer jump_buffer, background_buffer, pipe_passed_buffer, game_over_buffer;
    RectangleShape board_right, board_left;

    RenderWindow window(VideoMode(screen_width, screen_height), "Flappy Bird");
    window.setFramerateLimit(60);

    if (!bird.loadFromFile("src/images/grim_bird.png") ||
        !background.loadFromFile("src/images/background.png") ||
        !heart.loadFromFile("src/images/heart.png") ||
        !pipeUp.loadFromFile("src/images/pipeUp.png") ||
        !pipeDown.loadFromFile("src/images/pipeDown.png") ||
        !font.loadFromFile("src/fonts/JosefinSans-SemiBold.ttf") ||
        !jump_buffer.loadFromFile("src/sounds/jump_sound.wav") ||
        !background_buffer.loadFromFile("src/sounds/background_sound.wav") ||
        !pipe_passed_buffer.loadFromFile("src/sounds/pipe_passed_sound.wav") ||
        !game_over_buffer.loadFromFile("src/sounds/game_over.wav")) {
        cerr << "Erreur lors du chargement des ressources." << endl;
        return -1;
    }

    Sprite birdSprite(bird);
    birdSprite.setScale(0.2f, 0.2f);

    Sprite backgroundSprite(background);
    Sprite heartSprite(heart);
    heartSprite.setScale(0.05f, 0.05f);

    stringstream str_score;
    str_score << score;
    Text score_set("score: " + str_score.str(), font, 20);
    score_set.setFillColor(Color::White);
    score_set.setPosition(23, 20);

    stringstream str_life;
    str_life << life;
    Text life_set("x" + str_life.str(), font, 20);
    life_set.setFillColor(Color::White);
    life_set.setPosition(740, 20);

    Sound jump_sound(jump_buffer);
    Sound background_sound(background_buffer);
    Sound pipe_passed_sound(pipe_passed_buffer);
    Sound game_over_sound(game_over_buffer);
    
    bool gameOver = false;

    board_right.setSize(Vector2f(68, 30));
    board_right.setOutlineColor(Color(0, 0, 0));
    board_right.setOutlineThickness(3);
    board_right.setPosition(700, 20);
    board_right.setFillColor(Color(115, 141, 179));

    board_left.setSize(Vector2f(100, 30));
    board_left.setOutlineColor(Color(0, 0, 0));
    board_left.setOutlineThickness(3);
    board_left.setPosition(20, 20);
    board_left.setFillColor(Color(115, 141, 179));

    vector<Pipe> pipes;
    const float pipeGap = 200;
    const float pipeSpeed = 200;
    const float spawnInterval = 1.5f;
    float spawnTimer = 0;

    GameState gameState = StartScreen;

    Text startText("Press Enter to Start", font, 30);
    startText.setFillColor(Color::White);
    startText.setPosition(screen_width / 2 - startText.getGlobalBounds().width / 2, screen_height / 2 - startText.getGlobalBounds().height / 2);

    Text gameOverText("Game Over! Press Enter to Restart", font, 30);
    gameOverText.setFillColor(Color::White);
    gameOverText.setPosition(screen_width / 2 - gameOverText.getGlobalBounds().width / 2, screen_height / 2 - gameOverText.getGlobalBounds().height / 2);

    Clock clock;

    while (window.isOpen()) {
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Enter) {
                    if (gameState == StartScreen || gameState == GameOver) {
                        gameState = Playing;
                        resetGame(life, score, bird_y, velocity, pipes, str_score, score_set, str_life, life_set);
                        gameOver = false;
                        background_sound.setLoop(true);
                        background_sound.play();
                    }
                }
                if (event.key.code == Keyboard::Space && gameState == Playing) {
                    velocity = jump_velocity;
                    jump_sound.setVolume(100);
                    jump_sound.play();
                }
            }
        }

        float deltaTime = clock.restart().asSeconds();

        if (gameState == Playing) {
            velocity += gravity;
            bird_y += velocity;

            if (bird_y > screen_height - birdSprite.getGlobalBounds().height) {
                bird_y = screen_height - birdSprite.getGlobalBounds().height;
                velocity = 0;
            }
            if (bird_y < 40) {
                bird_y = 40;
                velocity = 0;
            }

            birdSprite.setPosition(50, bird_y);
            heartSprite.setPosition(700, 18);

            spawnTimer += deltaTime;
            if (spawnTimer >= spawnInterval) {
                float posY = rand() % 200 + 150;
                pipes.emplace_back(pipeUp, pipeDown, screen_width, posY, pipeGap, pipeSpeed);
                spawnTimer = -1;
            }

            for (auto &pipe : pipes) {
                pipe.update(deltaTime);
            }

            pipes.erase(remove_if(pipes.begin(), pipes.end(), [](Pipe &pipe) {
                return pipe.isOffScreen();
            }), pipes.end());

            for (auto &pipe : pipes) {
                if (!pipe.passed && pipe.getXPosition() + pipe.topPipe.getGlobalBounds().width < birdSprite.getPosition().x) {
                    score++;
                    pipe.passed = true;
                    str_score.str("");
                    str_score << score;
                    score_set.setString("score: " + str_score.str());
                    pipe_passed_sound.play();
                }
            }

            for (auto &pipe : pipes) {
                if (pipe.collidesWithBird(birdSprite) && !pipe.hasCollided()) {
                    life--;
                    pipe.setCollided(true);
                    str_life.str("");
                    str_life << life;
                    life_set.setString("x" + str_life.str());

                    if (life <= 0) {
                        gameState = GameOver;
                        break;
                    }

                    bird_y = 250;
                    velocity = 0;
                    pipes.clear();
                    spawnTimer = 0;
                    break;
                }
            }
        }

        window.clear();
        window.draw(backgroundSprite);

        if (gameState == StartScreen) {
            window.draw(startText);
        } else if (gameState == Playing) {
            window.draw(birdSprite);

            for (auto &pipe : pipes) {
                pipe.draw(window);
            }

            window.draw(board_right);
            window.draw(board_left);
            window.draw(score_set);
            window.draw(heartSprite);
            window.draw(life_set);
        } else if (gameState == GameOver) {
            window.draw(gameOverText);
            window.draw(board_left);
            window.draw(score_set);
            background_sound.pause();
            if (!gameOver) {
            game_over_sound.play();
            gameOver = true;
        }
        }

        window.display();
    }

    return 0;
}

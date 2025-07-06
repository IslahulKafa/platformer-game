#include <iostream>
#include <SFML/Graphics.hpp>
#include  <vector>
#include <fstream>

struct Platform {
    std::vector<sf::Sprite> tiles;

    Platform(sf::Texture &tileset, int tileCol, int tileRow, float x, float y, int tileCount) {
        const int TILE_WIDTH = 32;
        const int TILE_HEIGHT = 32;
        const int TILE_SPACING = 1;

        int tileX = tileCol * (TILE_WIDTH + TILE_SPACING);
        int tileY = tileRow * (TILE_HEIGHT + TILE_SPACING);

        sf::IntRect tileRect(tileX, tileY, TILE_WIDTH, TILE_HEIGHT);

        for (int i = 0; i < tileCount; ++i) {
            sf::Sprite tile;
            tile.setTexture(tileset);
            tile.setTextureRect(tileRect);
            tile.setPosition(x + i * TILE_WIDTH, y);
            tiles.push_back(tile);
        }
    }

    void draw(sf::RenderWindow &window) const {
        for (const auto &tile: tiles)
            window.draw(tile);
    }

    std::vector<sf::FloatRect> getBounds() const {
        std::vector<sf::FloatRect> bounds;
        for (const auto &tile: tiles)
            bounds.push_back(tile.getGlobalBounds());
        return bounds;
    }
};

struct Collectible {
    sf::Sprite sprite;
    bool collected = false;
    int frameIndex = 0;
    float animationTimer = 0.f;
    const float animationSpeed = 0.1f;
    static constexpr int frameCount = 17;

    Collectible(sf::Texture &texture, float x, float y) {
        sprite.setTexture(texture);
        sprite.setPosition(x, y);
        sprite.setScale(1.f, 1.f);
        sprite.setTextureRect(sf::IntRect(0, 0, 32, 32));
    }

    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }

    void update(float deltaTime) {
        if (collected) return;

        animationTimer += deltaTime;
        if (animationTimer >= animationSpeed) {
            animationTimer = 0.f;
            frameIndex = (frameIndex + 1) % frameCount;
            sprite.setTextureRect(sf::IntRect(frameIndex * 32, 0, 32, 32));
        }
    }

    void draw(sf::RenderWindow &window) const {
        if (!collected) {
            window.draw(sprite);
        }
    }
};

struct Explosion {
    sf::Sprite sprite;
    float frameTime = 0.f;
    int currentFrame = 0;
    bool finished = false;

    static constexpr float animationSpeed = 0.07f;
    static constexpr int frameSize = 32;
    static constexpr int totalFrames = 6;

    Explosion(sf::Texture &texture, sf::Vector2f pos) {
        sprite.setTexture(texture);
        sprite.setTextureRect(sf::IntRect(0, 0, frameSize, frameSize));
        sprite.setOrigin(frameSize / 2.f, frameSize / 2.f);
        sprite.setPosition(pos);
        sprite.setScale(1.5f, 1.5f);
    }

    void update(float dt) {
        if (finished) return;

        frameTime += dt;
        if (frameTime >= animationSpeed) {
            frameTime = 0.f;
            currentFrame++;
            if (currentFrame >= totalFrames) {
                finished = true;
            } else {
                sprite.setTextureRect(sf::IntRect(currentFrame * frameSize, 0, frameSize, frameSize));
            }
        }
    }

    void draw(sf::RenderWindow &window) const {
        if (!finished) {
            window.draw(sprite);
        }
    }

    bool isFinished() const {
        return finished;
    }
};

struct FireLauncher {
    sf::Sprite sprite;
    sf::Texture &offTexture;
    sf::Texture &onTexture;

    float timer = 0.f;
    float stateDuration = 1.5f;
    bool isActive = false;
    float x;
    float y;

    float animationTimer = 0.f;
    const float frameDuration = 0.1f;
    int currentFrame = 0;
    const int frameCount = 3;
    const int frameWidth = 16;
    const int frameHeight = 32;

    FireLauncher(sf::Texture &offTex, sf::Texture &onTex, float x, float y)
        : offTexture(offTex), onTexture(onTex), x(x), y(y) {
        sprite.setTexture(offTexture);
        sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));

        sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
        sprite.setPosition(x + frameWidth / 2.f, y +frameHeight / 2.f);

        sprite.setRotation(180.f);
    }

    void update(float dt) {
        timer += dt;

        if (timer >= stateDuration) {
            timer = 0.f;
            isActive = !isActive;
            sprite.setTexture(isActive ? onTexture : offTexture);
            currentFrame = 0;
            animationTimer = 0.f;
        }

        if (isActive) {
            animationTimer += dt;
            if (animationTimer >= frameDuration) {
                animationTimer = 0.f;
                currentFrame = (currentFrame + 1) % frameCount;
                sprite.setTextureRect(sf::IntRect(currentFrame * frameWidth, 0, frameWidth, frameHeight));
            }
        } else {
            sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
        }
    }

    void draw(sf::RenderWindow &window) const {
        window.draw(sprite);
    }

    sf::FloatRect getBounds() const {
        sf::FloatRect bounds = sprite.getGlobalBounds();
        bounds.top +=32;
        return bounds;
    }
};

void loadMap(const std::string &filePath, std::vector<Platform> &platforms, sf::Texture &tileset,
             sf::Vector2f &playerStartPos, std::vector<Collectible> *collectibles = nullptr,
             sf::Texture *collectibleTexture = nullptr,
             std::vector<FireLauncher> *fireLaunchers = nullptr,
             sf::Texture *fireOffTex = nullptr,
             sf::Texture *fireOnTex = nullptr,
             const int TILE_WIDTH = 32, const int TILE_HEIGHT = 32) {
    std::ifstream file(filePath);
    std::string line;
    int row = 0;

    while (std::getline(file, line)) {
        for (int col = 0; col < line.length(); ++col) {
            char tile = line[col];
            float x = col * TILE_WIDTH;
            float y = row * TILE_HEIGHT;

            if (tile == '#') {
                Platform p(tileset, 3, 0, x, y, 1);
                platforms.push_back(p);
            } else if (tile == 'P') {
                playerStartPos = {x + TILE_WIDTH / 2.f, y + TILE_HEIGHT};
            } else if (tile == 'C') {
                collectibles->emplace_back(*collectibleTexture, x, y);
            } else if (tile == 'F' && fireLaunchers && fireOffTex && fireOnTex) {
                fireLaunchers->emplace_back(*fireOffTex, *fireOnTex, x, y);
            }
        }
        row++;
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Platformer Game");
    window.setFramerateLimit(60);

    sf::View camera(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
    sf::Vector2f cameraCenter(400.f, 300.f);
    camera.zoom(0.75f);
    const float cameraSmoothness = 0.1f;

    enum class PlayerState { Idle, Running, Jumping };
    PlayerState currentState = PlayerState::Idle;
    PlayerState lastState = PlayerState::Idle;

    sf::Texture idleTexture, runTexture, jumpTexture;
    if (!idleTexture.loadFromFile("/mnt/disk-2/Media/Projects/C++/platformer-game/player_idle.png") ||
        !runTexture.loadFromFile("/mnt/disk-2/Media/Projects/C++/platformer-game/player_run.png") ||
        !jumpTexture.loadFromFile("/mnt/disk-2/Media/Projects/C++/platformer-game/player_jump.png")) {
        std::cerr << "Error loading player1.png" << std::endl;
        return -1;
    }

    const int frameWidth = 32;
    const int frameHeight = 32;
    int frameIndex = 0;
    float animationTimer = 0.f;
    const float animationSpeed = 0.3f;

    // Create the player sprite
    sf::Sprite player;
    player.setTexture(idleTexture);
    player.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
    player.setOrigin(frameWidth / 2.f, frameHeight);

    // Tileset
    sf::Texture tileset;
    if (!tileset.loadFromFile("/mnt/disk-2/Media/Projects/C++/platformer-game/terrain.png")) {
        std::cerr << "Error loading tileset.png" << std::endl;
        return -1;
    }

    std::vector<Platform> platforms;

    sf::Texture appleTexture;
    if (!appleTexture.loadFromFile("/mnt/disk-2/Media/Projects/C++/platformer-game/apple.png")) {
        std::cerr << "Error loading apple.png" << std::endl;
        return -1;
    }

    sf::Texture particleTexture;
    if (!particleTexture.loadFromFile("/mnt/disk-2/Media/Projects/C++/platformer-game/particle.png")) {
        std::cerr << "Error loading particle.png" << std::endl;
        return -1;
    }

    std::vector<Explosion> explosions;

    sf::Texture fireOffTexture, fireOnTexture;
    if (!fireOffTexture.loadFromFile("/mnt/disk-2/Media/Projects/C++/platformer-game/sprite/fire-launcher/off.png") ||
        !fireOnTexture.loadFromFile("/mnt/disk-2/Media/Projects/C++/platformer-game/sprite/fire-launcher/on.png")) {
        std::cerr << "Error loading fire launcher texture" << std::endl;
        return -1;
    }

    std::vector<FireLauncher> fireLaunchers;

    // Physics
    sf::Vector2f velocity(0.f, 0.f);
    const float gravity = 0.5f;
    const float jumpStrength = -11.f;
    const float moveSpeed = 4.0f;

    bool isOnGround = false;
    float jumpCooldown = 0.f;
    const float jumpCooldownDuration = 0.5f;
    bool facingRight = true;

    sf::Clock clock;

    std::vector<Collectible> collectibles;
    sf::Vector2f playerStart;
    loadMap("/mnt/disk-2/Media/Projects/C++/platformer-game/map.txt", platforms, tileset, playerStart, &collectibles,
            &appleTexture, &fireLaunchers, &fireOffTexture, &fireOnTexture);
    player.setPosition(playerStart);
    sf::Vector2f playerSpawnPos = player.getPosition();

    int score = 0;
    sf::Font font;
    if (!font.loadFromFile("/mnt/disk-2/Media/Projects/C++/platformer-game/font.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
        return -1;
    }

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(20);
    scoreText.setFillColor(sf::Color::Black);
    scoreText.setPosition(10, 10);
    scoreText.setString("Score: 0");

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        sf::Event event;
        while (window.pollEvent(event))
            if (event.type == sf::Event::Closed)
                window.close();

        if (jumpCooldown > 0.f) {
            jumpCooldown -= deltaTime;
        }

        // Input
        bool movingLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
        bool movingRight = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
        bool isMoving = movingLeft || movingRight;

        // Direction
        if (movingLeft) facingRight = false;
        if (movingRight) facingRight = true;

        // Horizontal movement
        float dx = 0.f;
        if (movingLeft) dx -= moveSpeed;
        if (movingRight) dx += moveSpeed;

        sf::Vector2f pos = player.getPosition();
        sf::FloatRect bounds(
            pos.x - frameWidth / 2.f + dx,
            pos.y - frameHeight,
            frameWidth,
            frameHeight
        );

        for (const auto &platform: platforms) {
            for (const auto &tileBounds: platform.getBounds()) {
                if (bounds.intersects(tileBounds)) {
                    if (dx > 0)
                        dx = tileBounds.getPosition().x - (pos.x - frameWidth / 2.f + frameWidth);
                    else if (dx < 0)
                        dx = (tileBounds.getPosition().x + tileBounds.getSize().x) - (pos.x - frameWidth / 2.f);
                }
            }
        }
        player.move(dx, 0.f);

        if (player.getPosition().y > 1000.f) {
            player.setPosition(playerSpawnPos);
            velocity = {0.f, 0.f};
        }

        // Apply gravity
        velocity.y += gravity;

        // Vertical movement
        pos = player.getPosition();
        sf::FloatRect verticalBounds(
            pos.x - frameWidth / 2.f,
            pos.y - frameHeight + velocity.y,
            frameWidth,
            frameHeight
        );

        isOnGround = false;
        std::cout << player.getPosition().y << std::endl;
        // Tiles
        for (const auto &platform: platforms) {
            for (const auto &tileBounds: platform.getBounds()) {
                if (verticalBounds.intersects(tileBounds)) {
                    float playerBottom = verticalBounds.top + verticalBounds.height;
                    float playerTop = verticalBounds.top;
                    float tileTop = tileBounds.top;
                    float tileBottom = tileBounds.top + tileBounds.height;

                    if (velocity.y > 0.f && playerBottom - velocity.y <= tileTop + 2.f) {
                        // Land on top
                        velocity.y = 0.f;
                        player.setPosition(pos.x, tileTop);
                        isOnGround = true;
                    } else if (velocity.y < 0.f && playerTop - velocity.y >= tileBottom - 2.f) {
                        // Hit from below
                        velocity.y = 0.f;
                        player.setPosition(pos.x, tileBottom + frameHeight);
                    }
                }
            }
        }
        player.move(0.f, velocity.y);

        // Jump
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && isOnGround && jumpCooldown <= 0.f) {
            velocity.y = jumpStrength;
            isOnGround = false;
            jumpCooldown = jumpCooldownDuration;
        }

        // Animation
        if (!isOnGround) {
            currentState = PlayerState::Jumping;
        } else if (isMoving) {
            currentState = PlayerState::Running;
        } else {
            currentState = PlayerState::Idle;
        }

        if (currentState != lastState) {
            frameIndex = 0;
            animationTimer = 0.f;
            switch (currentState) {
                case PlayerState::Idle:
                    player.setTexture(idleTexture);
                    break;
                case PlayerState::Running:
                    player.setTexture(runTexture);
                    break;
                case PlayerState::Jumping:
                    player.setTexture(jumpTexture);
                    break;
            }
            lastState = currentState;
        }

        if (currentState == PlayerState::Idle) {
            animationTimer += animationSpeed;
            if (animationTimer >= 1.f) {
                animationTimer = 0;
                frameIndex = (frameIndex + 1) % 11;
            }
        } else if (currentState == PlayerState::Running) {
            animationTimer += animationSpeed;
            if (animationTimer >= 1.f) {
                animationTimer = 0;
                frameIndex = (frameIndex + 1) % 12;
            }
        } else {
            frameIndex = 0;
        }

        int textureX = frameIndex * frameWidth;
        if (facingRight) {
            player.setTextureRect(sf::IntRect(textureX, 0, frameWidth, frameHeight));
        } else {
            player.setTextureRect(sf::IntRect(textureX + frameWidth, 0, -frameWidth, frameHeight));
        }

        float mapWidth = 800.f * 5;
        float viewHalfWidth = camera.getSize().x / 2.f;
        float viewHalfHeight = camera.getSize().y / 2.f;

        cameraCenter.x = std::max(viewHalfWidth, cameraCenter.x);
        cameraCenter.x = std::min(mapWidth - viewHalfWidth, cameraCenter.x);

        cameraCenter.y = std::max(viewHalfHeight, cameraCenter.y);

        pos.y = std::max(pos.y, 150.f);
        sf::Vector2f target = {player.getPosition().x, 350.f};
        cameraCenter += (target - cameraCenter) * cameraSmoothness;
        camera.setCenter(cameraCenter);

        for (auto &e: explosions) {
            e.update(deltaTime);
        }
        explosions.erase(std::remove_if(explosions.begin(), explosions.end(),
                                        [](const Explosion &e) { return e.isFinished(); }),
                         explosions.end());

        for (auto &apple: collectibles) {
            apple.update(deltaTime);
            if (!apple.collected && player.getGlobalBounds().intersects(apple.getBounds())) {
                apple.collected = true;
                score++;
                explosions.emplace_back(particleTexture, player.getPosition());
                scoreText.setString("Score: " + std::to_string(score));
            }
        }

        for (auto &fire: fireLaunchers) {
            fire.update(deltaTime);

            if (fire.isActive && fire.getBounds().intersects(player.getGlobalBounds())) {
                player.setPosition(playerSpawnPos);
                velocity = {0.f, 0.f};
            }
        }

        // Draw
        window.clear(sf::Color(135, 206, 235));
        window.setView(camera);
        for (const auto &platform: platforms) {
            platform.draw(window);
        }
        for (const auto &apple: collectibles) {
            apple.draw(window);
        }
        for (const auto &e: explosions) {
            e.draw(window);
        }
        for (const auto &fire: fireLaunchers)
            fire.draw(window);
        window.draw(player);
        window.setView(window.getDefaultView());
        window.draw(scoreText);
        window.display();
    }

    return 0;
}

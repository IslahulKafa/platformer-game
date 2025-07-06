#include "Platform.h"

Platform::Platform(sf::Vector2f position, sf::Texture &texture) {
    sprite.setTexture(texture);
    sprite.setPosition(position);
    sprite.setTextureRect(sf::IntRect(0, 0, 32, 32));

    bounds.push_back(sprite.getGlobalBounds());
}

void Platform::draw(sf::RenderWindow &window) const {
    window.draw(sprite);
}

const std::vector<sf::FloatRect> &Platform::getBounds() const {
    return bounds;
}

#pragma once

#include <SFML/Graphics.hpp>


class Platform {
public:
    Platform(sf::Vector2f position, sf::Texture &texture);

    void draw(sf::RenderWindow &window) const;

    const std::vector<sf::FloatRect> &getBounds() const;

private:
    sf::Sprite sprite;
    std::vector<sf::FloatRect> bounds;
};

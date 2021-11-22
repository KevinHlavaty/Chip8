
#include <SFML/Graphics.hpp>
#include <iostream>

#include "Chip8.h"

#define SCALE 10

void drawScreen(sf::RenderWindow& window, bool** pixels) {
    for(int row = 0; row < HEIGHT; row++) {
        for(int col = 0; col < WIDTH; col++) {
            sf::RectangleShape pixel(sf::Vector2<float>(SCALE, SCALE));
            pixel.setPosition(col * SCALE, row * SCALE);
            if(pixels[row][col]) {
                pixel.setFillColor(sf::Color::Black);
                pixel.setOutlineColor(sf::Color::Black);
            } else {
                pixel.setFillColor(sf::Color::White);
                pixel.setOutlineColor(sf::Color::White);
            }
            window.draw(pixel);
        }
    }
}

int main(int argc, char* argv[]) {
    Chip8* chip8 = new Chip8(argv[1]);
    sf::RenderWindow window(sf::VideoMode(WIDTH * SCALE, HEIGHT * SCALE), "Chip8", sf::Style::Titlebar | sf::Style::Close);

    while(window.isOpen()) {
        sf::Event event;
        while(window.pollEvent(event)) {
            // Window closed or escape key pressed: exit
            if((event.type == sf::Event::Closed) ||
              ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
            {
                window.close();
                break;
            }
        }
        chip8->emulateCycle();
        if(chip8->getDrawFlag()) {
            drawScreen(window, chip8->getGraphics());
            window.display();
        }
    }
    return 0;
}

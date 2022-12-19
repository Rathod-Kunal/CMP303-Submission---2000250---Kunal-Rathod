#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <time.h>

const int width = 600, height = 600;
int speed = 2;
sf::TcpSocket socket;   //  creates a TCP socket
sf::Packet serverPacket;
sf::Packet clientPacket;


struct player
{
    int x, y, direction;
    sf::Color playerColor;
    player(sf::Color color) {
        x = width / 6;
        y = height / 6;
        playerColor = color;
        direction = 1;
    }
    void tick() {
        switch (direction)  //  tick function makes sure that both client and server are in sync as players are constantly moving, based on direction at a steady speed
        {
        case 0:
            x -= 1;
            break;
        case 1:
            x += 1;
            break;
        case 2:
            y -= 1;
            break;
        case 3:
            y += 1;
            break;
        }

        if (x > width) {
            x = 0;
        }
        else if (x < 0) {
            x = width - 1;
        }
        if (y > height) {
            y = 0;
        }
        else if (y < 0) {
            y = height - 1;
        }
    }

    sf::Vector3f getColor() {
        return sf::Vector3f(playerColor.r, playerColor.g, playerColor.b);
    }
};

void serverListen() {   //  function listens for client connection attempts and allows the client to connect
    std::cout << "Listening for Client" << std::endl;
    sf::TcpListener listener;   //  Creates a listener for listening to connections
    if (listener.listen(53000) != sf::Socket::Done) {   //  listens for connections at port 53000
        std::cout << "Listen failed..." << std::endl;
    }

    if (listener.accept(socket) != sf::Socket::Done) {  //  accepts client connection
        std::cout << "Connection failed" << std::endl;
        return;
    }

    std::cout << "Client Connected" << std::endl;
}

void clientListen() {

}

int main() {
    srand(time(0));

    char typeInput = 'n';   //  creates variable for user input
    std::string windowType; //  creates variable to store whether the window acts as a server or a client
    std::string windowName;
    socket.setBlocking(false);

    std::cout << "Do you want a server (s) or client (c)?" << std::endl;    //  asks user for their input
    std::cin >> typeInput;

    while (!(typeInput == 's' || typeInput == 'c'))    //  while the input is invalid it will keep asking the user to enter s or c
    {
        std::cout << "Do you want a server (s) or client (c)?" << std::endl;
        std::cin >> typeInput;
    }

    if (typeInput == 's') { //  if the user entered s windowType is set to Server
        windowType = "Server";
        serverListen();
    }
    else if (typeInput == 'c') {    //  if the user entered s windowType is set to Client
        windowType = "Client";
        sf::Socket::Status status = socket.connect("127.0.0.1", 53000); //  attempts a connection to local host address through port 53000
        if (status != sf::Socket::Done) {
            std::cout << "Client connection failed" << std::endl;
        }

        std::cout << "Client connection successful" << std::endl;
    }

    if (windowType == "Server") {
        windowName = "Tron (Server Window)";
    }
    else {
        windowName = "Tron (Client Window)";
    }

    sf::RenderWindow window(sf::VideoMode(width, height), windowName);    // Creates window for game graphics
    window.setFramerateLimit(60);

    sf::Clock sendClock;
    sf::Clock disconnectClock;

    int sendCount = 0;

    sf::Texture texture;
    texture.loadFromFile("assets/tronBackground.png");
    sf::Sprite spriteBackground(texture);

    player p1(sf::Color::Red), p2(sf::Color::Blue);
    p2.x = p2.x * 5;
    p2.y = p2.y * 5;
    p2.direction = 0;

    sf::Sprite sprite;
    sf::RenderTexture tex;
    tex.create(width, height);
    tex.setSmooth(true);
    sprite.setTexture(tex.getTexture());
    tex.clear();
    tex.draw(spriteBackground);

    sf::Font font;
    font.loadFromFile("assets/ethnocentricrg.otf");
    sf::Text text("You Win!", font, 30);
    text.setPosition(width / 2 - 100, 20);

    bool Game = 1;
    sf::String endMessage = "End";

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        if (windowType == "Server") {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {  //  player input
                if (p1.direction != 1) {
                    p1.direction = 0;
                }
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                if (p1.direction != 0) {
                    p1.direction = 1;
                }
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
                if (p1.direction != 3) {
                    p1.direction = 2;
                }
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                if (p1.direction != 2) {
                    p1.direction = 3;
                }
            }
            //receive client player position and update client player
            socket.receive(clientPacket);
            if (clientPacket >> p2.x >> p2.y >> p2.direction) {
                std::cout << "Client Packet read successful" << std::endl;
                std::cout << "Client player position : " << p2.x << ", " << p2.y << std::endl;
                clientPacket.clear();
                disconnectClock.restart();
            }
            else {
                std::cout << "Client Packet read failed" << std::endl;
                if (disconnectClock.getElapsedTime().asSeconds() >= 5) {    //  handles disconnection
                    std::cout << "Closing Client connection" << std::endl;  //  if there has been no message received for at least 5 seconds, the connection is terminated and the window is closed
                    socket.disconnect();    //  terminates connection
                    disconnectClock.restart();
                    return 0;
                }
            }
        }
        else {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {   //  player input
                if (p2.direction != 1) {
                    p2.direction = 0;
                }
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                if (p2.direction != 0) {
                    p2.direction = 1;
                }
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                if (p2.direction != 3) {
                    p2.direction = 2;
                }
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                if (p2.direction != 2) {
                    p2.direction = 3;
                }
            }
            //receive server player position and update server player
            socket.receive(serverPacket);
            if (serverPacket >> p1.x >> p1.y >> p1.direction) {
                std::cout << "Server Packet read successful" << std::endl;
                std::cout << "Server player position : " << p1.x << ", " << p1.y << std::endl;
                serverPacket.clear();
                disconnectClock.restart();  //  resets disconnect clock
            }
            else {
                std::cout << "Server Packet read failed" << std::endl;
                if (disconnectClock.getElapsedTime().asSeconds() >= 5) {
                    std::cout << "Disconnecting from server" << std::endl;
                    socket.disconnect();
                    disconnectClock.restart();
                    return 0;
                }
            }
        }

        for (int i = 0; i < speed; i++) {

            p1.tick();  //  updates player movement
            p2.tick();

            sf::CircleShape circle(3);  //  sets up texture for the player trails
            circle.setPosition(p1.x, p1.y);
            circle.setFillColor(p1.playerColor);
            tex.draw(circle);
            circle.setPosition(p2.x, p2.y);
            circle.setFillColor(p2.playerColor);
            tex.draw(circle);
            tex.display();
        }

        window.clear(); //  displays game objects
        window.draw(sprite);
        window.display();

        if (windowType == "Server") {
            if (sendClock.getElapsedTime().asSeconds() >= 1) {  //  if the time is greater than 1 second, reset the timer
                sendClock.restart();
                sendCount = 0;
            }
            else if ((sendClock.getElapsedTime().asMilliseconds() / 10) > sendCount){   //  makes sure packets are sent every tenth of a second
                //send server player position to client
                serverPacket << p1.x << p1.y << p1.direction;   //  synchronises positions between client and server
                socket.send(serverPacket);
                serverPacket.clear();   //  empties packet
                sendCount += 1;
            }
        }
        else {
            if (sendClock.getElapsedTime().asSeconds() >= 1) {
                sendClock.restart();
                sendCount = 0;
            }
            else if ((sendClock.getElapsedTime().asMilliseconds() / 10) > sendCount) {
                //send server player position to client
                clientPacket << p2.x << p2.y << p2.direction;
                socket.send(clientPacket);
                clientPacket.clear();   //  empties packet
                sendCount += 1;
            }
        }

    }

    return 0;
}
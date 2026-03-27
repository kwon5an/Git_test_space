#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

using namespace std;

// 총알
struct Bullet {
    sf::CircleShape shape;
    sf::Vector2f velocity;
};

// 맵
class Map {
private:
    vector<vector<int>> grid;
    int tileSize;

public:
    Map() {
        tileSize = 50;

        grid = {
            {1,1,1,1,1,1,1,1,1,1},
            {1,0,0,0,1,0,0,0,0,1},
            {1,0,1,0,1,0,1,1,0,1},
            {1,0,1,0,0,0,0,1,0,1},
            {1,0,1,1,1,1,0,1,0,1},
            {1,0,0,0,0,1,0,1,0,1},
            {1,1,1,1,0,1,0,1,0,1},
            {1,0,0,1,0,0,0,0,0,1},
            {1,0,0,0,0,1,1,1,0,1},
            {1,1,1,1,1,1,1,1,1,1}
        };
    }

    int getTileSize() { return tileSize; }

    bool isWall(int x, int y) {
        if (x < 0 || y < 0 || y >= grid.size() || x >= grid[0].size())
            return true;
        return grid[y][x] == 1;
    }

    void draw(sf::RenderWindow& window) {
        sf::RectangleShape tile;
        tile.setSize(sf::Vector2f(tileSize, tileSize));

        for (int y = 0; y < grid.size(); y++) {
            for (int x = 0; x < grid[y].size(); x++) {

                tile.setPosition(x * tileSize, y * tileSize);

                if (grid[y][x] == 1)
                    tile.setFillColor(sf::Color(90, 90, 90));
                else
                    tile.setFillColor(sf::Color(30, 30, 30));

                window.draw(tile);
            }
        }
    }
};

// 부드러운 충돌 (핵심 수정)
bool canMove(sf::Vector2f pos, float radius, Map& map) {
    int t = map.getTileSize();

    // 중심 기준으로 약간 안쪽만 검사 (여유)
    float checkRadius = radius * 0.7f;

    int gridX = pos.x / t;
    int gridY = pos.y / t;

    // 주변 4칸만 검사 (부드럽게)
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {

            int nx = gridX + x;
            int ny = gridY + y;

            if (map.isWall(nx, ny)) {
                float tileLeft = nx * t;
                float tileTop = ny * t;

                // 원 vs 사각형 충돌
                float closestX = max(tileLeft, min(pos.x, tileLeft + t));
                float closestY = max(tileTop, min(pos.y, tileTop + t));

                float dx = pos.x - closestX;
                float dy = pos.y - closestY;

                if (dx * dx + dy * dy < checkRadius * checkRadius)
                    return false;
            }
        }
    }

    return true;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(500, 500), "FPS FIXED");
    window.setFramerateLimit(60);

    sf::Clock clock;
    Map map;

    float radius = 15;

    // 플레이어 (안 막히는 위치)
    sf::CircleShape player(radius);
    player.setFillColor(sf::Color::Blue);
    player.setOrigin(radius, radius);
    player.setPosition(75, 75); // 안전 위치

    // 총구
    sf::RectangleShape gun(sf::Vector2f(25, 5));
    gun.setFillColor(sf::Color::White);
    gun.setOrigin(0, 2.5);

    vector<Bullet> bullets;

    float playerSpeed = 180.0f;
    float bulletSpeed = 400.0f;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // 발사
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {

                    sf::Vector2f mouse = window.mapPixelToCoords(
                        sf::Mouse::getPosition(window));

                    sf::Vector2f dir = mouse - player.getPosition();
                    float len = sqrt(dir.x * dir.x + dir.y * dir.y);
                    dir /= len;

                    Bullet b;
                    b.shape = sf::CircleShape(5);
                    b.shape.setFillColor(sf::Color::Yellow);
                    b.shape.setOrigin(5, 5);
                    b.shape.setPosition(player.getPosition());
                    b.velocity = dir * bulletSpeed;

                    bullets.push_back(b);
                }
            }
        }

        // 이동
        sf::Vector2f move(0, 0);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) move.y -= playerSpeed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) move.y += playerSpeed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) move.x -= playerSpeed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) move.x += playerSpeed * dt;

        sf::Vector2f pos = player.getPosition();

        // X
        sf::Vector2f newX = pos + sf::Vector2f(move.x, 0);
        if (canMove(newX, radius, map))
            player.move(move.x, 0);

        // Y
        sf::Vector2f newY = player.getPosition() + sf::Vector2f(0, move.y);
        if (canMove(newY, radius, map))
            player.move(0, move.y);

        // 마우스 방향
        sf::Vector2f mouse = window.mapPixelToCoords(
            sf::Mouse::getPosition(window));

        sf::Vector2f dir = mouse - player.getPosition();
        float angle = atan2(dir.y, dir.x) * 180 / 3.141592;

        gun.setPosition(player.getPosition());
        gun.setRotation(angle);

        // 총알 이동
        for (int i = 0; i < bullets.size(); i++) {
            bullets[i].shape.move(bullets[i].velocity * dt);

            int bx = bullets[i].shape.getPosition().x / map.getTileSize();
            int by = bullets[i].shape.getPosition().y / map.getTileSize();

            if (map.isWall(bx, by)) {
                bullets.erase(bullets.begin() + i);
                i--;
            }
        }

        window.clear();

        map.draw(window);
        window.draw(player);
        window.draw(gun);

        for (auto& b : bullets)
            window.draw(b.shape);

        window.display();
    }

    return 0;
}
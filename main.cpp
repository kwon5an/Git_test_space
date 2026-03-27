#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <algorithm> // max, min 사용을 위해 추가

using namespace std;

// 총알 구조체
struct Bullet {
    sf::CircleShape shape;
    sf::Vector2f velocity;
};

// 맵 클래스
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
        if (x < 0 || y < 0 || y >= (int)grid.size() || x >= (int)grid[0].size())
            return true;
        return grid[y][x] == 1;
    }

    void draw(sf::RenderWindow& window) {
        sf::RectangleShape tile;
        tile.setSize({(float)tileSize, (float)tileSize});

        for (int y = 0; y < (int)grid.size(); y++) {
            for (int x = 0; x < (int)grid[y].size(); x++) {
                tile.setPosition({(float)x * tileSize, (float)y * tileSize});

                if (grid[y][x] == 1)
                    tile.setFillColor(sf::Color(90, 90, 90));
                else
                    tile.setFillColor(sf::Color(30, 30, 30));

                window.draw(tile);
            }
        }
    }
};

// 충돌 감지 함수
bool canMove(sf::Vector2f pos, float radius, Map& map) {
    int t = map.getTileSize();
    float checkRadius = radius * 0.7f;

    int gridX = static_cast<int>(pos.x / t);
    int gridY = static_cast<int>(pos.y / t);

    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            int nx = gridX + x;
            int ny = gridY + y;

            if (map.isWall(nx, ny)) {
                float tileLeft = (float)nx * t;
                float tileTop = (float)ny * t;

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
    // SFML 3.0: sf::VideoMode({width, height}) 형식 사용
    sf::RenderWindow window(sf::VideoMode({500, 500}), "FPS FIXED");
    window.setFramerateLimit(60);

    sf::Clock clock;
    Map map;

    float radius = 15.f;

    // 플레이어 설정
    sf::CircleShape player(radius);
    player.setFillColor(sf::Color::Blue);
    player.setOrigin({radius, radius});
    player.setPosition({75.f, 75.f});

    // 총구 설정
    sf::RectangleShape gun({25.f, 5.f});
    gun.setFillColor(sf::Color::White);
    gun.setOrigin({0.f, 2.5f});

    vector<Bullet> bullets;

    float playerSpeed = 180.0f;
    float bulletSpeed = 400.0f;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // SFML 3.0: pollEvent()는 std::optional<sf::Event>를 반환할 수 있으나 
        // 기존 루프 방식도 호환성을 위해 유지되거나 getEvent로 대체 가능합니다.
        // 여기서는 가장 안정적인 3.0 스타일로 작성합니다.
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            // 마우스 클릭 발사
            if (const auto* mouseButtonEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseButtonEvent->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    sf::Vector2f dir = mousePos - player.getPosition();
                    float len = sqrt(dir.x * dir.x + dir.y * dir.y);
                    if (len != 0) dir /= len;

                    Bullet b;
                    b.shape = sf::CircleShape(5.f);
                    b.shape.setFillColor(sf::Color::Yellow);
                    b.shape.setOrigin({5.f, 5.f});
                    b.shape.setPosition(player.getPosition());
                    b.velocity = dir * bulletSpeed;

                    bullets.push_back(b);
                }
            }
        }

        // 이동 로직
        sf::Vector2f moveAmount(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) moveAmount.y -= playerSpeed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) moveAmount.y += playerSpeed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) moveAmount.x -= playerSpeed * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) moveAmount.x += playerSpeed * dt;

        sf::Vector2f currentPos = player.getPosition();

        // X축 이동 검사
        sf::Vector2f nextX = currentPos + sf::Vector2f(moveAmount.x, 0.f);
        if (canMove(nextX, radius, map))
            player.move({moveAmount.x, 0.f});

        // Y축 이동 검사
        sf::Vector2f nextY = player.getPosition() + sf::Vector2f(0.f, moveAmount.y);
        if (canMove(nextY, radius, map))
            player.move({0.f, moveAmount.y});

        // 마우스 방향 계산
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        sf::Vector2f lookDir = mousePos - player.getPosition();
        float angle = atan2(lookDir.y, lookDir.x) * 180.f / 3.141592f;

        gun.setPosition(player.getPosition());
        gun.setRotation(sf::degrees(angle)); // SFML 3.0: sf::degrees() 사용 권장

        // 총알 업데이트
        for (int i = 0; i < (int)bullets.size(); i++) {
            bullets[i].shape.move(bullets[i].velocity * dt);

            int bx = (int)(bullets[i].shape.getPosition().x / map.getTileSize());
            int by = (int)(bullets[i].shape.getPosition().y / map.getTileSize());

            if (map.isWall(bx, by)) {
                bullets.erase(bullets.begin() + i);
                i--;
            }
        }

        window.clear();
        map.draw(window);
        window.draw(player);
        window.draw(gun);
        for (auto& b : bullets) {
            window.draw(b.shape);
        }
        window.display();
    }

    return 0;
}
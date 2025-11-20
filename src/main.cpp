#include <algorithm>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "DeltaClock.h"

#include <iostream>
#include <numbers>
#include <cmath>
#include <array>

namespace
{
    // Ensure a 2:1 aspect ratio!
    constexpr Uint16 SCREEN_WIDTH = 2000;
    constexpr Uint16 SCREEN_HEIGHT = 1000;

    SDL_Window* window{nullptr};
    SDL_Renderer* renderer{nullptr};

    const bool* keyStates{nullptr};

    bool IS_RUNNING{true};

    util::DeltaClock deltaClock;
    double deltaTime{};

    // Ray casting config.
    float FOV = 60.0f;

    constexpr int GRID_WIDTH = 10;
    constexpr int GRID_HEIGHT = 10;

    constexpr float TILE_SIZE = static_cast<float>(SCREEN_WIDTH / 2) / GRID_WIDTH;

    constexpr Uint8 RAY_RES = 4;
    constexpr Uint16 NUMBER_OF_RAYS = (SCREEN_WIDTH / 2) / RAY_RES;

    // Map.
    constexpr int MAP_SIZE = GRID_WIDTH * GRID_HEIGHT;
    const int map[GRID_HEIGHT][GRID_WIDTH] =
    {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 1, 1, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };

    // Player.
    float playerX{TILE_SIZE * 2};
    float playerY{TILE_SIZE * 2};
    float playerDeltaX{};
    float playerDeltaY{};
    float playerAngle{std::numbers::pi * 0.5f};

    constexpr float rotationSpeed{3.0f};
    constexpr float moveSpeed{3.0f};
}

float normaliseAngle(const float angle)
{
    if (angle <= 0.0f)
        return angle + (2.0f * std::numbers::pi);
    else if (angle > (2.0f * std::numbers::pi))
        return angle - (2.0f * std::numbers::pi);

    return angle;
}

float distanceBetween(const float x1, const float y1, const float x2, const float y2)
{
    const float x = x2 - x1;
    const float y = y2 - y1;

    return std::sqrt((x * x) + (y * y));
}

void handleMovement()
{
    if (keyStates[SDL_SCANCODE_W])
    {
        playerX += playerDeltaX * moveSpeed * deltaTime;
        playerY += playerDeltaY * moveSpeed * deltaTime;
    }

    if (keyStates[SDL_SCANCODE_S])
    {
        playerX -= playerDeltaX * moveSpeed * deltaTime;
        playerY -= playerDeltaY * moveSpeed * deltaTime;
    }

    if (keyStates[SDL_SCANCODE_A])
    {
        playerAngle -= rotationSpeed * deltaTime;
        playerAngle = normaliseAngle(playerAngle);

        playerDeltaX = std::cos(playerAngle) * TILE_SIZE;// * (SCREEN_WIDTH / GRID_WIDTH);
        playerDeltaY = std::sin(playerAngle) * TILE_SIZE;// * (SCREEN_HEIGHT / GRID_WIDTH);
    }

    if (keyStates[SDL_SCANCODE_D])
    {
        playerAngle += rotationSpeed * deltaTime;
        playerAngle = normaliseAngle(playerAngle);

        playerDeltaX = std::cos(playerAngle) * TILE_SIZE;// * (SCREEN_WIDTH / GRID_WIDTH);
        playerDeltaY = std::sin(playerAngle) * TILE_SIZE;// * (SCREEN_HEIGHT / GRID_HEIGHT);
    }
}

void handleInput(const SDL_Event& event)
{
    switch (event.key.key)
    {
    case SDLK_ESCAPE:
        IS_RUNNING = false;
        break;
    default:
        break;
    }
}

void handleEvent(SDL_Event& event)
{
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            IS_RUNNING = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            handleInput(event);
            break;
        default:
            break;
        }
    }
}

bool hasWallAt(const float pixelX, const float pixelY)
{
    if (pixelX < 0 || pixelY < 0)
        return false;

    const int tileX = std::floor(pixelX / TILE_SIZE);
    const int tileY = std::floor(pixelY / TILE_SIZE);

    if (tileX + tileY > MAP_SIZE)
        return false;

    return map[tileY][tileX] == 1;
}

void drawMap()
{
    SDL_FRect rect{0.0f, 0.0f, TILE_SIZE - 2, TILE_SIZE - 2};

    for (Uint16 y = 0; y < GRID_HEIGHT; y++)
    {
        for (Uint16 x = 0; x < GRID_WIDTH; x++)
        {
            rect.x = (SCREEN_WIDTH / 2) + x * TILE_SIZE;
            rect.y = y * TILE_SIZE;

            if (map[y][x] == 0)
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            else
                SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);

            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void drawFilledCircle(SDL_Renderer* renderer, const int centreX, const int centreY, const int radius)
{
    int x = 0;
    int y = radius;
    int p = 1 - radius; // Midpoint decision variable

    // Draw the initial vertical line at the center of the circle
    while (x <= y)
    {
        // Draw horizontal lines (symmetry in the circle)
        SDL_RenderLine(renderer, centreX - x, centreY + y, centreX + x, centreY + y); // Top part
        SDL_RenderLine(renderer, centreX - x, centreY - y, centreX + x, centreY - y); // Bottom part
        SDL_RenderLine(renderer, centreX - y, centreY + x, centreX + y, centreY + x); // Left part
        SDL_RenderLine(renderer, centreX - y, centreY - x, centreX + y, centreY - x); // Right part

        x++;

        // Midpoint circle decision logic
        if (p < 0)
        {
            p = p + 2 * x + 1;
        }
        else
        {
            y--;
            p = p + 2 * (x - y) + 1;
        }
    }
}

struct Ray
{
    float distance;
    int colour;
};

int main(int argc, char* argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL failed to initialise. Error: %s", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("Raycaster", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

    if (!window)
    {
        SDL_Log("SDL couldn't create a window. Error: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer)
    {
        SDL_Log("Failed to create a renderer. Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    keyStates = SDL_GetKeyboardState(nullptr);

    // Convert the field of view into radians.
    FOV = FOV * (std::numbers::pi / 180.0f);

    playerDeltaX = std::cos(playerAngle) * 150.0f;
    playerDeltaY = std::sin(playerAngle) * 150.0f;

    // Create and populate the ray distances array.
    std::array<Ray, NUMBER_OF_RAYS> rays{};
    rays.fill({std::numeric_limits<float>::max(), 255});

    // Calculate the distance to the projection plane.
    const float distanceToProjectionPlane = (SCREEN_WIDTH * 0.25f) / std::tan(FOV * 0.5f);

    while (IS_RUNNING)
    {
        SDL_Event event;
        SDL_zero(event);

        handleEvent(event);

        deltaTime = deltaClock.tick();

        handleMovement();

        // Render.
        SDL_SetRenderDrawColorFloat(renderer, 0.0f, 0.0f, 0.0f, 0.0f);
        SDL_RenderClear(renderer);

        drawMap();

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        drawFilledCircle(renderer, SCREEN_WIDTH / 2 + playerX, playerY, 30);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderLine(renderer, SCREEN_WIDTH / 2 + playerX, playerY, SCREEN_WIDTH / 2 + playerX + playerDeltaX, playerY + playerDeltaY);

        const float degreePerRay = FOV / NUMBER_OF_RAYS;
        const float rayStartAngle = playerAngle - (FOV * 0.5f);

        // for (int i = 0; i < NUMBER_OF_RAYS; i++)
        // {
        //     float rayAngle = rayStartAngle + (i * degreePerRay);
        //     rayAngle = normaliseAngle(rayAngle);
        //
        //     const float rayEndX = playerX + (std::cos(rayAngle) * 300.0f);
        //     const float rayEndY = playerY + (std::sin(rayAngle) * 300.0f);
        //
        //     SDL_RenderLine(renderer, playerX, playerY, rayEndX, rayEndY);
        // }

        //float rayAngle = playerAngle;
        constexpr int maximumDepth = 8;

        float rayAngle = normaliseAngle(rayStartAngle);

        // for (int i = 0; i < 1; i++)
        for (int i = 0; i < NUMBER_OF_RAYS; i++)
        {
            int depth = 0;

            float rayX{0};
            float rayY{0};

            // Horizontal hit check.
            float horizontalHitX{};
            float horizontalHitY{};
            float horizontalDistance = std::numeric_limits<float>::max();

            const bool isFacingUp = rayAngle > std::numbers::pi;
            const bool isFacingDown = !isFacingUp;
            const bool isFacingLeft = rayAngle > 0.5f * std::numbers::pi && rayAngle < 1.5f * std::numbers::pi;
            const bool isFacingRight = !isFacingLeft;

            // The ray is looking 'up'.
            // if (rayAngle > std::numbers::pi)
            if (isFacingUp)
            {
                rayY = (std::floor(playerY / TILE_SIZE) * TILE_SIZE) - 0.0001f;
            }
            // The ray is looking 'down'.
            //else if (rayAngle < std::numbers::pi)
            else if (isFacingDown)
            {
                rayY = (std::floor(playerY / TILE_SIZE) * TILE_SIZE) + TILE_SIZE;
            }

            rayX = ((rayY - playerY) / std::tan(rayAngle)) + playerX;

            if (rayAngle == 0.0f || rayAngle == std::numbers::pi)
            {
                rayX = playerX;
                rayY = playerY;
                depth = maximumDepth;
            }

            //float rayYOffset = (rayAngle > std::numbers::pi) ? -TILE_SIZE : TILE_SIZE;
            float rayYOffset = isFacingUp ? -TILE_SIZE : TILE_SIZE;
            float rayXOffset = rayYOffset / std::tan(rayAngle);

            while (depth < maximumDepth)
            {
                if (hasWallAt(rayX, rayY)) // the ray hit a wall
                {
                    horizontalHitX = rayX;
                    horizontalHitY = rayY;
                    horizontalDistance = distanceBetween(playerX, playerY, horizontalHitX, horizontalHitY);
                    horizontalDistance *= std::cos(playerAngle - rayAngle);

                    break;
                }

                rayX += rayXOffset;
                rayY += rayYOffset;
                depth++;
            }

            // Vertical hit check.
            float verticalHitX{};
            float verticalHitY{};
            float verticalDistance = std::numeric_limits<float>::max();

            // The ray is looking 'right'.
            //if (rayAngle < 0.5f * std::numbers::pi || rayAngle > 1.5f * std::numbers::pi)
            if (isFacingRight)
            {
                rayX = (std::floor(playerX / TILE_SIZE) * TILE_SIZE) + TILE_SIZE;
            }
            // The ray is looking 'left'.
            //else if (rayAngle > 0.5f * std::numbers::pi && rayAngle < 1.5f * std::numbers::pi)
            else if (isFacingLeft)
            {
                rayX = (std::floor(playerX / TILE_SIZE) * TILE_SIZE) - 0.0001f;
            }

            rayY = playerY + (rayX - playerX) * std::tan(rayAngle);

            //rayXOffset = (rayAngle < std::numbers::pi / 2 || rayAngle > (3.0f * std::numbers::pi) / 2) ? TILE_SIZE : -TILE_SIZE;
            rayXOffset = isFacingRight ? TILE_SIZE : -TILE_SIZE;
            rayYOffset = rayXOffset * std::tan(rayAngle);

            // Reset the current ray depth.
            depth = 0;

            while (depth < maximumDepth)
            {
                if (hasWallAt(rayX, rayY)) // the ray hit a wall
                {
                    verticalHitX = rayX;
                    verticalHitY = rayY;
                    verticalDistance = distanceBetween(playerX, playerY, verticalHitX, verticalHitY);
                    verticalDistance *= std::cos(playerAngle - rayAngle);

                    break;
                }

                rayX += rayXOffset;
                rayY += rayYOffset;
                depth++;
            }

            if (horizontalDistance < verticalDistance)
            {
                rayX = horizontalHitX;
                rayY = horizontalHitY;
                rays.at(i).distance = horizontalDistance;
                rays.at(i).colour = 255;
            }
            else
            {
                rayX = verticalHitX;
                rayY = verticalHitY;
                rays.at(i).distance = verticalDistance;
                rays.at(i).colour = 180;
            }

            // Draw the line.
            SDL_RenderLine(renderer, SCREEN_WIDTH / 2 + playerX, playerY, SCREEN_WIDTH / 2 + rayX, rayY);

            rayAngle = normaliseAngle(rayAngle + degreePerRay);
        }

        // Render the background
        /* SDL_FRect background{0.0f, 0.0f, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
        SDL_SetRenderDrawColor(renderer, 180, 0, 0, 255);
        SDL_RenderFillRect(renderer, &background);

        background.y = SCREEN_HEIGHT / 2;
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderFillRect(renderer, &background); */

        SDL_FRect wallRect{0.0f, 0.0f, RAY_RES, 0.0f};

        // Calculate and display the walls.
        for (int i = 0; i < NUMBER_OF_RAYS; i++)
        {
            constexpr float wallWidth = (SCREEN_WIDTH / 2) / NUMBER_OF_RAYS;
            const float wallHeight = (TILE_SIZE / rays.at(i).distance) * distanceToProjectionPlane;

            wallRect.x = i * RAY_RES;
            wallRect.y = (SCREEN_HEIGHT * 0.5f) - (wallHeight * 0.5f);
            wallRect.w = wallWidth;
            wallRect.h = wallHeight;

            int colour = rays.at(i).colour * (180 / rays.at(i).distance);
            colour = std::clamp(colour, 0, 255);

            SDL_SetRenderDrawColor(renderer, colour, colour, colour, 255);

            SDL_RenderFillRect(renderer, &wallRect);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

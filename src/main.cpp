#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>

#include <iostream>
#include <algorithm>
#include <numbers>
#include <cmath>
#include <array>
#include <string>

#include "DeltaClock.h"

float degreesToRadians(const float degrees);

namespace
{
    // Ensure a 2:1 aspect ratio!
    constexpr Uint16 SCREEN_WIDTH = 2560;
    constexpr Uint16 SCREEN_HEIGHT = 1440;

    SDL_Window* window{nullptr};
    SDL_Renderer* renderer{nullptr};

    const bool* keyStates{nullptr};

    bool IS_RUNNING{true};

    util::DeltaClock deltaClock;
    double deltaTime{};

    // Ray casting config.
    const float HFOV = degreesToRadians(90.0f);

    constexpr int GRID_WIDTH = 13;
    constexpr int GRID_HEIGHT = 13;

    constexpr Uint8 RAY_RES = 1;
    constexpr Uint16 NUMBER_OF_RAYS = SCREEN_WIDTH / RAY_RES;

    // Map.
    constexpr int MAP_SIZE = GRID_WIDTH * GRID_HEIGHT;
    const int map[GRID_HEIGHT][GRID_WIDTH] =
    {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };

    // Player.
    float playerX{1.5f};
    float playerY{1.5f};
    float playerDeltaX{};
    float playerDeltaY{};
    float playerAngle{std::numbers::pi * 0.5f};

    constexpr float rotationSpeed{3.0f};
    constexpr float moveSpeed{2.0f};
}

float degreesToRadians(const float degrees)
{
    return degrees * (std::numbers::pi_v<float> / 180.0f);
}

float radiansToDegrees(const float radians)
{
    return radians * (180.0f / std::numbers::pi_v<float>);
}

Uint16 worldToGridCoordinate(const float worldPosition)
{
    return std::floor(worldPosition);
}

bool hasWallAt(const float worldX, const float worldY)
{
    if (worldX < 0 || worldY < 0)
        return false;

    const int tileX = worldToGridCoordinate(worldX);
    const int tileY = worldToGridCoordinate(worldY);

    if (tileX + tileY > MAP_SIZE)
        return false;

    return map[tileY][tileX] == 1;
}

float normaliseAngle(const float angle)
{
    if (angle <= 0.0f)
        return angle + (2.0f * std::numbers::pi_v<float>);
    else if (angle > (2.0f * std::numbers::pi))
        return angle - (2.0f * std::numbers::pi_v<float>);

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
        const float xOffset = playerDeltaX < 0 ? -0.25f : 0.25f;
        const float yOffset = playerDeltaY < 0 ? -0.25f : 0.25f;

        const float xOffsetPosition = playerX + xOffset;
        const float yOffsetPosition = playerY + yOffset;

        if (!hasWallAt(xOffsetPosition, playerY))
            playerX += playerDeltaX * moveSpeed * static_cast<float>(deltaTime);

        if (!hasWallAt(playerX, yOffsetPosition))
            playerY += playerDeltaY * moveSpeed * static_cast<float>(deltaTime);
    }

    if (keyStates[SDL_SCANCODE_S])
    {
        const float xOffset = playerDeltaX < 0 ? -0.25f : 0.25f;
        const float yOffset = playerDeltaY < 0 ? -0.25f : 0.25f;

        const float xOffsetPosition = playerX - xOffset;
        const float yOffsetPosition = playerY - yOffset;

        if (!hasWallAt(xOffsetPosition, playerY))
            playerX -= playerDeltaX * moveSpeed * static_cast<float>(deltaTime);

        if (!hasWallAt(playerX, yOffsetPosition))
            playerY -= playerDeltaY * moveSpeed * static_cast<float>(deltaTime);
    }

    if (keyStates[SDL_SCANCODE_A])
    {
        playerAngle -= rotationSpeed * static_cast<float>(deltaTime);
        playerAngle = normaliseAngle(playerAngle);

        playerDeltaX = std::cos(playerAngle);
        playerDeltaY = std::sin(playerAngle);
    }

    if (keyStates[SDL_SCANCODE_D])
    {
        playerAngle += rotationSpeed * static_cast<float>(deltaTime);
        playerAngle = normaliseAngle(playerAngle);

        playerDeltaX = std::cos(playerAngle);
        playerDeltaY = std::sin(playerAngle);
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

    window = SDL_CreateWindow("Raycaster", SCREEN_WIDTH, SCREEN_HEIGHT, NULL);

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

    playerDeltaX = std::cos(playerAngle);
    playerDeltaY = std::sin(playerAngle);

    // Create and populate the ray distances array.
    std::array<Ray, NUMBER_OF_RAYS> rays{};
    rays.fill({std::numeric_limits<float>::max(), 255});

    // Calculate the distance to the projection plane.
    const float distanceToProjectionPlane = (SCREEN_WIDTH * 0.5f) / std::tan(HFOV * 0.5f);
    const float projectionPlaneWidth = distanceToProjectionPlane * std::tan(HFOV * 0.5f) * 2.0f;
    const float projectionPlaneHalfWidth = projectionPlaneWidth * 0.5f;
    constexpr int maxX = SCREEN_WIDTH - 1;

    const float VFOV = 2 * std::atan(std::tan(HFOV * 0.5f) * (static_cast<float>(SCREEN_HEIGHT) / static_cast<float>(SCREEN_WIDTH)));

    const float projectionPlaneHeight = distanceToProjectionPlane * std::tan(VFOV * 0.5f) * 2.0f;

    while (IS_RUNNING)
    {
        SDL_Event event;
        SDL_zero(event);

        handleEvent(event);

        deltaTime = deltaClock.tick();

       handleMovement();

        std::string title = "X: " + std::to_string(playerX) + " Y: " + std::to_string(playerY);
        SDL_SetWindowTitle(window, title.c_str());

        // Render.
        SDL_SetRenderDrawColorFloat(renderer, 0.0f, 0.0f, 0.0f, 0.0f);
        SDL_RenderClear(renderer);

        const float rayStartAngle = playerAngle - (HFOV * 0.5f);
        float rayAngle = normaliseAngle(rayStartAngle);

        for (int i = 0; i < NUMBER_OF_RAYS; i++)
        {
            constexpr int maximumDepth = 20;
            int depth = 0;

            float rayX{0};
            float rayY{0};

            const bool isFacingUp = rayAngle > std::numbers::pi;
            const bool isFacingDown = !isFacingUp;
            const bool isFacingLeft = rayAngle > 0.5f * std::numbers::pi && rayAngle < 1.5f * std::numbers::pi;
            const bool isFacingRight = !isFacingLeft;

            // Horizontal hit check.
            float horizontalDistance = std::numeric_limits<float>::max();

            // The ray is looking 'up'.
            if (isFacingUp)
                rayY = std::floor(playerY) - 0.000001f;
            // The ray is looking 'down'.
            else if (isFacingDown)
                rayY = std::floor(playerY) + 1.0f;

            rayX = ((rayY - playerY) / std::tan(rayAngle)) + playerX;

            // if (rayAngle == 0.0f || rayAngle == std::numbers::pi)
            // {
            //     rayX = playerX;
            //     rayY = playerY;
            //     depth = maximumDepth;
            // }

            float rayYOffset = isFacingUp ? -1 : 1;
            float rayXOffset = rayYOffset / std::tan(rayAngle);

            while (depth < maximumDepth)
            {
                if (hasWallAt(rayX, rayY)) // the ray hit a wall
                {
                    horizontalDistance = distanceBetween(playerX, playerY, rayX, rayY);
                    horizontalDistance *= std::cos(playerAngle - rayAngle);

                    break;
                }

                rayX += rayXOffset;
                rayY += rayYOffset;
                depth++;
            }

            // Vertical hit check.
            float verticalDistance = std::numeric_limits<float>::max();

            // The ray is looking 'right'.
            if (isFacingRight)
                rayX = std::floor(playerX) + 1.0f;
            // The ray is looking 'left'.
            else if (isFacingLeft)
                rayX = std::floor(playerX) - 0.000001f;

            rayY = playerY + (rayX - playerX) * std::tan(rayAngle);

            rayXOffset = isFacingRight ? 1 : -1;
            rayYOffset = rayXOffset * std::tan(rayAngle);

            // Reset the current ray depth.
            depth = 0;

            while (depth < maximumDepth)
            {
                if (hasWallAt(rayX, rayY)) // the ray hit a wall
                {
                    verticalDistance = distanceBetween(playerX, playerY, rayX, rayY);
                    verticalDistance *= std::cos(playerAngle - rayAngle);

                    break;
                }

                rayX += rayXOffset;
                rayY += rayYOffset;
                depth++;
            }

            if (horizontalDistance < verticalDistance)
            {
                rays.at(i).distance = horizontalDistance;
                rays.at(i).colour = 255;
            }
            else
            {
                rays.at(i).distance = verticalDistance;
                rays.at(i).colour = 180;
            }

            // Get the current ray's screen X position.
            const int screenX = (i + 1) * RAY_RES;

            // Calculate the next ray's correct position on the projection plane for the next loop.
            const float projectionScreenX = ((static_cast<float>(screenX * 2) - maxX) / maxX) * projectionPlaneHalfWidth;
            const float castAngle = std::atan2f(projectionScreenX, distanceToProjectionPlane) + playerAngle;

            // Prepare the ray angle for the next loop.
            rayAngle = normaliseAngle(castAngle);
        }

        // Render the background
        SDL_FRect background{0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT * 0.5f};
        SDL_SetRenderDrawColor(renderer, 56, 56, 56, 255);
        SDL_RenderFillRect(renderer, &background);

        background.y = SCREEN_HEIGHT * 0.5f;
        SDL_SetRenderDrawColor(renderer, 112, 112, 112, 255);
        SDL_RenderFillRect(renderer, &background);

        SDL_FRect wallRect{0.0f, 0.0f, RAY_RES, 0.0f};

        // Calculate and display the walls.
        for (int i = 0; i < NUMBER_OF_RAYS; i++)
        {
            constexpr float wallWidth = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(NUMBER_OF_RAYS);

            constexpr float halfWall = 1 * 0.5f;
            const float projectionPlaneY = distanceToProjectionPlane * (halfWall / rays.at(i).distance);
            const float wallHeight = SCREEN_HEIGHT * ((projectionPlaneY * 2) / projectionPlaneHeight);

            wallRect.x = i * RAY_RES;
            wallRect.y = (SCREEN_HEIGHT * 0.5f) - (wallHeight * 0.5f);
            wallRect.w = wallWidth;
            wallRect.h = wallHeight;

            // int colour = rays.at(i).colour * (1 / rays.at(i).distance * 5);
            // colour = std::clamp(colour, 30, 255);

            const int colour = rays.at(i).colour;

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

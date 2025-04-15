#include "external/raylib-5.5/src/raylib.h"
#include "external/raylib-5.5/src/raymath.h"
#include "external/raylib-5.5/src/rlgl.h"
#include <math.h>
#include <stdio.h>
#include <threads.h>

#define TILE_SIZE (50)
#define MAX_TILE_X (30)
#define MAX_TILE_Y (15)
#define FPS (60)

typedef enum {
  ENTITY_IDLE,
  ENTITY_STATE_COUNT
} EntityState;

typedef enum LayerType {
  GROUND,
  FARM,
  LAYER_COUNT
} LayerType;

typedef enum TextureType {
  EMPTY,
  GRASS,
  DIRT,
  PLAYER,
  TEXTURE_TYPE_COUNT,
} TextureType;

typedef struct CameraState {
  float scaleFactor;
} CameraState;

typedef struct Tile {
  float posX;
  float posY;
  TextureType type;
} Tile;

typedef enum  {
  CENTER,
  NORTH,
  NORTHEAST,
  EAST,
  SOUTHEAST,
  SOUTH,
  SOUTHWEST,
  WEST,
  NORTHWEST,

  CENTER_END,
  NORTH_END,
  EAST_END,
  SOUTH_END,
  WEST_END,

  N_CORNER,
  NE_CORNER,
  E_CORNER,
  SE_CORNER,
  S_CORNER,
  SW_CORNER,
  W_CORNER,
  NW_CORNER,

  TILE_STATE_COUNT
} TileState;

typedef enum {
  TP_TILESET,
  TP_ENTITY,
  TEXTURE_PATHS_COUNT
} TexturePath;

typedef struct {
  Vector2 position;
  int height;
  int width;
  Vector2 direction;
  Vector2 velocity;
  int frames_counter;
  int current_frame;
  Rectangle frame_rect;
  int base_accel;
  int current_accel;
  int run_accel_modifier;
  Vector2 cell;
} Player;

typedef struct GameState {
  Tile tile_map[MAX_TILE_Y][MAX_TILE_X];
  Player player;
  int debug;
} GameState;


static const GameState DefaultGameState = {
  .player = (Player) {
    .position = (Vector2) { 0.0f, 0.0f },
    .velocity = (Vector2) { 0.f, 0.f },
    .base_accel = 200,
    .run_accel_modifier = 2,
    .cell = (Vector2) { 0.f, 0.f },
  },
  .debug = 0,
};

TextureType GetTileType(Tile tile_map[MAX_TILE_Y][MAX_TILE_X], int x, int y) {
  if (x < 0 || y < 0 || x >= MAX_TILE_X || y >= MAX_TILE_Y) {
    return 0; // out of map
  }
  return tile_map[y][x].type;
}

/*TileState GetTileState(Tile tileMap[MAX_TILE_Y][MAX_TILE_X], int x, int y, TextureType self) {*/
/*    // Get neighboring tile types (accounting for reversed rows/columns)*/
/*    TextureType top = GetTileType(tileMap, x, y - 1);    // Tile above*/
/*    TextureType bottom = GetTileType(tileMap, x, y + 1); // Tile below*/
/*    TextureType left = GetTileType(tileMap, x - 1, y);   // Tile to the left*/
/*    TextureType right = GetTileType(tileMap, x + 1, y);  // Tile to the right*/
/**/
/*    TextureType topLeft = GetTileType(tileMap, x - 1, y - 1);    // Top-left diagonal*/
/*    TextureType topRight = GetTileType(tileMap, x + 1, y - 1);   // Top-right diagonal*/
/*    TextureType bottomLeft = GetTileType(tileMap, x - 1, y + 1); // Bottom-left diagonal*/
/*    TextureType bottomRight = GetTileType(tileMap, x + 1, y + 1); // Bottom-right diagonal*/
/**/
/*    switch (self) {*/
/*    case GRASS: {*/
/*      // ======== PRIOR 1 ========*/
/**/
/*      // Case 4-Way Connect (center of 4 diagonal single tiles)*/
/*      if (!topLeft && !topRight && !bottomLeft && !bottomRight && top &&*/
/*          bottom && left && right) {*/
/*        return FOURWAY_CONNECT; // grassTileFourWayConnect; // Special texture*/
/*                                // for 4 diagonal tiles*/
/*      }*/
/**/
/*      // Case BottomLeft connected to TopRight*/
/*      if (!topLeft && topRight && bottomLeft && !bottomRight && top && bottom &&*/
/*          left && right) {*/
/*        return BOTTOM_LEFT_TOP_RIGHT; // grassTileBottomLeftTopRight;*/
/*      }*/
/*      // Case TopLeft connected to BottomRight*/
/*      if (topLeft && !topRight && !bottomLeft && bottomRight && top && bottom &&*/
/*          left && right) {*/
/*        return TOP_LEFT_BOTTOM_RIGHT; // grassTileTopLeftBottomRight;*/
/*      }*/
/*      // Case surrounded except TopLeft;*/
/*      if (!topLeft && topRight && bottomLeft && bottomRight && top && bottom &&*/
/*          left && right) {*/
/*        return CENTER_TOP_LEFT; // grassTileCenterTopLeft;*/
/*      }*/
/*      // Case surrounded except TopRight;*/
/*      if (topLeft && !topRight && bottomLeft && bottomRight && top && bottom &&*/
/*          left && right) {*/
/*        return CENTER_TOP_RIGHT; // grassTileCenterTopRight;*/
/*      }*/
/*      // Case surrounded except BottomLeft;*/
/*      if (topLeft && topRight && !bottomLeft && bottomRight && top && bottom &&*/
/*          left && right) {*/
/*        return CENTER_BOTTOM_LEFT; // grassTileCenterBottomLeft;*/
/*      }*/
/*      // Case surrounded except BottomRight;*/
/*      if (topLeft && topRight && bottomLeft && !bottomRight && top && bottom &&*/
/*          left && right) {*/
/*        return CENTER_BOTTOM_RIGHT; // grassTileCenterBottomRight;*/
/*      }*/
/*      // Case TopRight and BottomRight not connected*/
/*      if (topLeft && !topRight && bottomLeft && !bottomRight && top && bottom &&*/
/*          left && right) {*/
/*        return CONNECT_LEFT_SINGLE; // grassTileLeftSingleConnect;*/
/*      }*/
/*      // Case TopLeft and BottomLeft not connected*/
/*      if (!topLeft && topRight && !bottomLeft && bottomRight && top && bottom &&*/
/*          left && right) {*/
/*        return CONNECT_RIGHT_SINGLE;*/
/*        ; // grassTileRightSingleConnect;*/
/*      }*/
/*      // Case Topleft and TopRight not connected*/
/*      if (!topLeft && !topRight && bottomLeft && bottomRight && top && bottom &&*/
/*          left && right) {*/
/*        return CONNECT_TOP_SINGLE; // grassTileTopSingleConnect;*/
/*      }*/
/*      // Case Bottomleft and BottomRight not connected*/
/*      if (topLeft && topRight && !bottomLeft && !bottomRight && top && bottom &&*/
/*          left && right) {*/
/*        return CONNECT_BOTTOM_SINGLE; // grassTileBottomSingleConnect;*/
/*      }*/
/**/
/*      // Case BottomLeft tile connect single*/
/*      if (!topLeft && topRight && !bottomLeft && !bottomRight && top &&*/
/*          bottom && left && right) {*/
/*        return CONNECT_BOTTOM_LEFT_SINGLE; // grassTileBottomLeftConnectSingle;*/
/*      }*/
/*      // Case BottomRight tile connect single*/
/*      if (topLeft && !topRight && !bottomLeft && !bottomRight && top &&*/
/*          bottom && left && right) {*/
/*        return CONNECT_BOTTOM_RIGHT_SINGLE; // grassTileBottomRightConnectSingle;*/
/*      }*/
/*      // Case TopLeft tile connect single*/
/*      if (!topLeft && !topRight && !bottomLeft && bottomRight && top &&*/
/*          bottom && left && right) {*/
/*        return CONNECT_TOP_LEFT_SINGLE; // grassTileTopLeftConnectSingle;*/
/*      }*/
/*      // Case TopRight tile connect single*/
/*      if (!topLeft && !topRight && bottomLeft && !bottomRight && top &&*/
/*          bottom && left && right) {*/
/*        return CONNECT_TOP_RIGHT_SINGLE; // grassTileTopRightConnectSingle;*/
/*      }*/
/**/
/*      // ======== PRIOR 2 ========*/
/**/
/*      // Case TopTile connect SingleRight*/
/*      if (bottomLeft && !bottomRight && !top && bottom && left && right) {*/
/*        return CONNECT_TOP_TO_RIGHT_SINGLE; // grassTileTopToRightSingleConnect;*/
/*      }*/
/*      // Case TopTile connect SingleLeft*/
/*      if (!bottomLeft && bottomRight && !top && bottom && left && right) {*/
/*        return CONNECT_TOP_TO_LEFT_SINGLE; // grassTileTopToLeftSingleConnect;*/
/*      }*/
/*      // Case BottomTile connect SingleRight*/
/*      if (topLeft && !topRight && top && !bottom && left && right) {*/
/*        return CONNECT_BOTTOM_TO_RIGHT_SINGLE; // grassTileBottomToRightSingleConnect;*/
/*      }*/
/*      // Case BottomTile connect SingleLeft*/
/*      if (!topLeft && topRight && top && !bottom && left && right) {*/
/*        return CONNECT_BOTTOM_TO_LEFT_SINGLE; // grassTileBottomToLeftSingleConnect;*/
/*      }*/
/*      // Case LeftTile connect SingleTop*/
/*      if (bottomRight && !topRight && top && bottom && !left && right) {*/
/*        return CONNECT_LEFT_TO_TOP_SINGLE; // grassTileLeftToTopSingleConnect;*/
/*      }*/
/*      // Case LeftTile connect SingleBottom*/
/*      if (!bottomRight && topRight && top && bottom && !left && right) {*/
/*        return CONNECT_LEFT_TO_BOTTOM_SINGLE; // grassTileLeftToBottomSingleConnect;*/
/*      }*/
/*      // Case RightTile connect SingleTop*/
/*      if (bottomLeft && !topLeft && top && bottom && left && !right) {*/
/*        return CONNECT_RIGHT_TO_TOP_SINGLE; // grassTileRightToTopSingleConnect;*/
/*      }*/
/*      // Case RightTile connect SingleBottom*/
/*      if (!bottomLeft && topLeft && top && bottom && left && !right) {*/
/*        return CONNECT_RIGHT_TO_BOTTOM_SINGLE; // grassTileRightToBottomSingleConnect;*/
/*      }*/
/**/
/*      // Single Corners*/
/*      // Case SingleTopLeft*/
/*      if (!bottomRight && !top && bottom && !left && right) {*/
/*        return TOP_LEFT_SINGLE; // grassTileSingleTopLeft;*/
/*      }*/
/*      // Case SingleTopRight*/
/*      if (!bottomLeft && !top && bottom && left && !right) {*/
/*        return TOP_RIGHT_SINGLE; // grassTileSingleTopRight;*/
/*      }*/
/*      // Case SingleBottomLeft*/
/*      if (!topRight && top && !bottom && !left && right) {*/
/*        return BOTTOM_LEFT_SINGLE; // grassTileSingleBottomLeft;*/
/*      }*/
/*      // Case SingleBottomRight*/
/*      if (!topLeft && top && !bottom && left && !right) {*/
/*        return BOTTOM_RIGHT_SINGLE; // grassTileSingleBottomRight;*/
/*      }*/
/**/
/*      // Case SingleTop connect SingeBottom*/
/*      if (!bottomLeft && !bottomRight && !top && bottom && left && right) {*/
/*        return CONNECT_TOP_BOTTOM_SINGLE; // grassTileSingleTopConnectBottom;*/
/*      }*/
/*      // Case SingleBottom connect SingeTop*/
/*      if (!topLeft && !topRight && top && !bottom && left && right) {*/
/*        return CONNECT_BOTTOM_TOP_SINGLE; // grassTileSingleBottomConnectTop;*/
/*      }*/
/*      // Case SingleLeft connect SingeRight*/
/*      if (!topRight && !bottomRight && top && bottom && !left && right) {*/
/*        return CONNECT_LEFT_RIGHT_SINGLE; // grassTileSingleLeftConnectRight;*/
/*      }*/
/*      // Case SingleRight connect SingeLeft*/
/*      if (!topLeft && !bottomLeft && top && bottom && left && !right) {*/
/*        return CONNECT_RIGHT_LEFT_SINGLE; // grassTileSingleRightConnectLeft;*/
/*      }*/
/**/
/*      // ======== PRIOR 3 ========*/
/**/
/*      // Case 2: Corners*/
/*      if (!top && !left && right && bottom)*/
/*        return TOP_LEFT; // grassTileTopLeft;*/
/*      if (!top && !right && left && bottom)*/
/*        return TOP_RIGHT; // grassTileTopRight;*/
/*      if (!bottom && !left && right && top)*/
/*        return BOTTOM_LEFT; // grassTileBottomLeft;*/
/*      if (!bottom && !right && left && top)*/
/*        return BOTTOM_RIGHT; // grassTileBottomRight;*/
/**/
/**/
/*      // grass to dirt transitions*/
/*      if(bottomRight == DIRT && top && bottom && bottom != DIRT && left && right && right != DIRT) {*/
/*        return TRANSITION_CENTER_BOTTOM_RIGHT_DIRT;*/
/*      }*/
/*      if(bottomLeft == DIRT && top && bottom && bottom != DIRT && left && left != DIRT && right) {*/
/*        return TRANSITION_CENTER_BOTTOM_LEFT_DIRT;*/
/*      }*/
/*      if(topRight == DIRT && top&& top != DIRT && bottom && left && right && right != DIRT) {*/
/*        return TRANSITION_CENTER_TOP_RIGHT_DIRT;*/
/*      }*/
/*      if(topLeft == DIRT && top && top != DIRT && bottom && left && left != DIRT && right) {*/
/*        return TRANSITION_CENTER_TOP_LEFT_DIRT;*/
/*      }*/
/*      if(top && bottom && left && left == DIRT && right && right == DIRT) {*/
/*        return TRANSITION_CENTER_HORIZONTAL_DIRT;*/
/*      }*/
/*      if(top && top == DIRT && bottom && bottom == DIRT && left && right) {*/
/*        return TRANSITION_CENTER_VERTICAL_DIRT;*/
/*      }*/
/*      if(top && bottom == DIRT && left && right) {*/
/*        return TRANSITION_CENTER_BOTTOM_DIRT;*/
/*      }*/
/*      if(top == DIRT && bottom && left && right) {*/
/*        return TRANSITION_CENTER_TOP_DIRT;*/
/*      }*/
/*      if(top && bottom && left == DIRT && right) {*/
/*        return TRANSITION_CENTER_RIGHT_DIRT;*/
/*      }*/
/*      if(top && bottom && left && right == DIRT) {*/
/*        return TRANSITION_CENTER_LEFT_DIRT;*/
/*      }*/
/**/
/*      // Case 3: Edges*/
/*      if (!top && bottom && left && right)*/
/*        return TOP; // grassTileTop;*/
/*      if (!bottom && top && left && right)*/
/*        return BOTTOM; // grassTileBottom;*/
/*      if (!left && right && top && bottom)*/
/*        return LEFT; // grassTileLeft;*/
/*      if (!right && left && top && bottom)*/
/*        return RIGHT; /// grassTileRight;*/
/**/
/*      // Case 4: Single row (horizontal line)*/
/*      if (!top && !bottom && (left || right)) {*/
/*        // Leftmost of single row*/
/*        if (!left && right)*/
/*          return LEFT_SINGLE; // grassTileLeftSingle;*/
/*        // Rightmost of single row*/
/*        if (!right && left)*/
/*          return RIGHT_SINGLE; // grassTileRightSingle;*/
/*        // Middle of single row*/
/*        return HORIZONTAL_CENTER_SINGLE; // grassTileSingleHorizontalCenter;*/
/*      }*/
/**/
/*      // Case 5: Single column (vertical line)*/
/*      if (!left && !right && (top || bottom)) {*/
/*        // Topmost of single column*/
/*        if (!top && bottom)*/
/*          return TOP_SINGLE; // grassTileTopSingle;*/
/*        // Bottommost of single column*/
/*        if (!bottom && top)*/
/*          return BOTTOM_SINGLE; // grassTileBottomSingle;*/
/*        // Middle of single column*/
/*        return VERTICAL_CENTER_SINGLE; // grassTileSingleVerticalCenter;*/
/*      }*/
/**/
/*      //Single isolated tile*/
/*      if (!top && !bottom && !left && !right) {*/
/*        return SINGLE; // grassTileSingle;*/
/*      }*/
/*      break;*/
/*    }*/
/*    case DIRT: {*/
/**/
/*      if (!top && !left && right && bottom)*/
/*        return TOP_LEFT; // grassTileTopLeft;*/
/*      if (!top && !right && left && bottom)*/
/*        return TOP_RIGHT; // grassTileTopRight;*/
/*      if (!bottom && !left && right && top)*/
/*        return BOTTOM_LEFT; // grassTileBottomLeft;*/
/*      if (!bottom && !right && left && top)*/
/*        return BOTTOM_RIGHT; // grassTileBottomRight;*/
/**/
/*      if (!top && bottom && left && right)*/
/*        return TOP; // grassTileTop;*/
/*      if (!bottom && top && left && right)*/
/*        return BOTTOM; // grassTileBottom;*/
/*      if (!left && right && top && bottom)*/
/*        return LEFT; // grassTileLeft;*/
/*      if (!right && left && top && bottom)*/
/*        return RIGHT; /// grassTileRight;*/
/*      break;*/
/*    }*/
/*    default: {*/
/*      break;*/
/*    }*/
/*    }*/
/**/
/*    return CENTER;*/
/*}*/

TileState GetTileState(Tile tileMap[MAX_TILE_Y][MAX_TILE_X], int tileX, int tileY, TextureType self) {
  for(int i = -2; i > 2; i++) {
    for(int j = -2; j > 2; j++) {
      if(tileY + i < 0 || tileY + j < 0 || tileX + i >= MAX_TILE_X || tileX + j >= MAX_TILE_Y) {
        continue;
      }
      if(tileMap[tileY + i][tileX + j].type == GRASS) {
        return CENTER;
      }
    }
  }
}

static Rectangle EntityTextures[TEXTURE_TYPE_COUNT][ENTITY_STATE_COUNT] = {
  [PLAYER] = {
    [ENTITY_IDLE] = { 0.0f, 0.0f,  32.0f, 32.0f },
  }
};

static Rectangle TileTextures[TEXTURE_TYPE_COUNT][TILE_STATE_COUNT] = {
  [GRASS] = {
    [CENTER] = { 16.0f, 16.0f, 16.0f, 16.0f },
  },
};

static char* TexturePaths[][16] = {
  [TP_TILESET] = {
    [GRASS] = "Assets/Custom/GrassTile.png",
  },
  [TP_ENTITY] = {
    [PLAYER] = "Assets/Custom/Player.png",
  }
};

int main() {

  const int screenHeight = 1080;
  const int screenWidth = 1920;

  InitWindow(screenWidth, screenHeight, "ALLFARM");

  Texture2D grassTexture = LoadTexture("Assets/Custom/GrassTile.png");
  Texture2D playerTexture = LoadTexture("Assets/Custom/Player.png");

  Texture2D *textures[][16] = {
    [TP_TILESET] = {
      [GRASS] = &grassTexture,
    },
    [TP_ENTITY] = {
      [PLAYER] = &playerTexture,
    },
  };

  Texture2D atlas = LoadTexture("Assets/TextureAtlas.png");

  Camera2D camera = {0};
  CameraState cameraState = {.scaleFactor = 1.0f};

  GameState gameState = DefaultGameState;

  for (int y = 0; y < MAX_TILE_Y; y++) {
    for (int x = 0; x < MAX_TILE_X; x++) {
      gameState.tile_map[y][x] = (Tile){
        .posX = x * TILE_SIZE,
        .posY = y * TILE_SIZE,
        .type = 1
      };
    }
  }

  Player* player = &gameState.player;

  player->frame_rect = (Rectangle) {
    0.0f, 0.0f, playerTexture.width / 3.f, playerTexture.height
  };

  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  SetTargetFPS(FPS);
  ToggleFullscreen();

  while (!WindowShouldClose()) {
    player->frames_counter++;

    if(IsKeyPressed(KEY_G)) {
      gameState.debug = !gameState.debug;
    }

    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      cameraState.scaleFactor = 1.1f + (0.25f * fabsf(wheel));
      if (wheel < 0)
        cameraState.scaleFactor = 1.0f/cameraState.scaleFactor ;
      camera.zoom = Clamp(camera.zoom * cameraState.scaleFactor, 0.125f, 64.0f);
    }
    if (camera.zoom > 5.0f)
      camera.zoom = 5.0f;
    if (camera.zoom < 0.5f)
      camera.zoom = 0.5f;

    // Player
    Rectangle* player_rect = EntityTextures[PLAYER];

    player->height = 48.f;
    player->width = 48.f;

    player->current_accel = player->base_accel;

    if(IsKeyDown(KEY_LEFT_SHIFT))
      player->current_accel *= player->run_accel_modifier;

    int input_dirs[4] = {
      IsKeyDown(KEY_A),
      IsKeyDown(KEY_D),
      IsKeyDown(KEY_W),
      IsKeyDown(KEY_S)
    };
    player->direction.x = input_dirs[1] - input_dirs[0];
    player->direction.y = input_dirs[3] - input_dirs[2];

    player->velocity.x = Lerp(
        player->velocity.x,
        player->direction.x * player->current_accel,
        GetFrameTime() * 14.0f
    );
    player->velocity.y = Lerp(
        player->velocity.y,
        player->direction.y * player->current_accel,
        GetFrameTime() * 14.0f
    );

    player->position.x += player->velocity.x * GetFrameTime();
    player->position.y += player->velocity.y * GetFrameTime();

    camera.target = (Vector2) {
      player->position.x + (player->width / 2.f),
      player->position.y + (player->height / 2.f)
    };

    camera.offset = (Vector2) {
      .x = GetScreenWidth() / 2.f,
      .y = GetScreenHeight() / 2.f
    };

    // Set player cell
    player->cell.x = (player->position.x + (player->width / 2.f)) / TILE_SIZE;
    player->cell.y = (player->position.y + (player->height / 2.f)) / TILE_SIZE;

    // Calculate world coordinates of the top-left corner of the player hovered cell
    Vector2 player_world_pos = (Vector2){
      player->cell.x * TILE_SIZE,
      player->cell.y * TILE_SIZE
    };

    BeginDrawing();
    ClearBackground(DARKGRAY);
    BeginMode2D(camera);


    for (int tileY = 0; tileY < MAX_TILE_Y; tileY++) {
      for (int tileX = 0; tileX < MAX_TILE_X; tileX++) {
        Tile* curTile = &gameState.tile_map[tileY][tileX];

        // nothing to draw
        if (curTile->type == 0) continue;

        TileState tile_state = GetTileState(gameState.tile_map, tileX, tileY, curTile->type);
        Rectangle src_rect = TileTextures[curTile->type][tile_state];

        DrawTexturePro(
            *textures[TP_TILESET][curTile->type],
            src_rect,
            (Rectangle){
                .x = curTile->posX, .y = curTile->posY, TILE_SIZE, TILE_SIZE},
            (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
      }
    }

    if(gameState.debug) {
      for (int gridIdx = 0; gridIdx <= MAX_TILE_X * TILE_SIZE;
           gridIdx += TILE_SIZE) {
        DrawLine(gridIdx, 0, gridIdx, MAX_TILE_Y * TILE_SIZE, RAYWHITE);
      }
      for (int gridIdx = 0; gridIdx <= MAX_TILE_Y * TILE_SIZE;
           gridIdx += TILE_SIZE) {
        DrawLine(0, gridIdx, MAX_TILE_X * TILE_SIZE, gridIdx, RAYWHITE);
      }
    }

    // Animate player sprite
    if(
        fabsf(player->velocity.x) > 100.f ||
        fabsf(player->velocity.y) > 100.f)
    {
      Texture2D player_texture = *textures[TP_ENTITY][PLAYER];

      int max_velocity = player->base_accel * player->run_accel_modifier;
      int sprite_fps = 10;
      if(player->frames_counter >= (
          FPS /
          (sprite_fps *
           (fabsf(player->velocity.x) + fabsf(player->velocity.y)) / max_velocity)
        )
      ) {
        player->frames_counter = 0;
        player->current_frame++;
        if(player->current_frame > 2) {
          player->current_frame = 0;
        }
        if(player->velocity.x < 0.f) {
          player->frame_rect.width = -player_texture.width / 3.f;
        }
        else {
          player->frame_rect.width = player_texture.width / 3.f;
        }
        player->frame_rect.x = player->current_frame * (player_texture.width / 3.f);
      }
    }
    else {
      player->frame_rect.x = 0.f;
    }

    DrawTexturePro(*textures[TP_ENTITY][PLAYER],
        player->frame_rect,
        (Rectangle) {
        .x = player->position.x,
        .y = player->position.y,
        player->width,
        player->height
        },
        (Vector2) { 0.0f, 0.0f },
        0.0f,
        WHITE
    );

    // PLAYER POS TILE
    if ((player_world_pos.x < MAX_TILE_X * TILE_SIZE && player_world_pos.x >= 0) &&
        (player_world_pos.y < MAX_TILE_Y * TILE_SIZE && player_world_pos.y >= 0)) {
      Tile *tile = &gameState.tile_map[(int)player->cell.y][(int)player->cell.x];
      if(gameState.debug) {
      char buffer[5000];
        DrawRectangle(tile->posX, tile->posY, TILE_SIZE, TILE_SIZE, (Color) { 255, 255 ,255, 50 });
      }
    }

    EndMode2D();


    if(gameState.debug) {
      DrawRectangle(0, 0, 300, 400, (Color) { 0, 0 ,0, 50 });
      // top left text
      char buffer[5000];
      sprintf(buffer, "player world pos: %.f, %.f", player_world_pos.x,
              player_world_pos.y);
      DrawText(buffer, 10, 50, 20, WHITE);
      sprintf(buffer, "player cell: %d, %d", (int)player->cell.x,
          (int)player->cell.y);
      DrawText(buffer, 10, 100, 20, WHITE);
      sprintf(buffer, "%f", cameraState.scaleFactor);
      DrawText(buffer, 10, 150, 20, WHITE);
      sprintf(buffer, "pvx: %f", player->velocity.x);
      DrawText(buffer, 10, 200, 20, WHITE);
      sprintf(buffer, "pvy: %f", player->velocity.y);
      DrawText(buffer, 10, 250, 20, WHITE);
      sprintf(buffer, "player: current_frame: %d", player->current_frame);
      DrawText(buffer, 10, 300, 20, WHITE);
      sprintf(buffer, "player: frames_counter: %d", player->frames_counter);
      DrawText(buffer, 10, 350, 20, WHITE);
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}

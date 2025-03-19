#include "external/raylib-5.5/src/raylib.h"
#include "external/raylib-5.5/src/raymath.h"
#include "external/raylib-5.5/src/rlgl.h"
#include <math.h>
#include <stdio.h>
#include <threads.h>

#define TILE_SIZE (50)
#define MAX_TILE_X (30)
#define MAX_TILE_Y (15)

typedef enum TileState {
  CENTER,
  TOP,
  RIGHT,
  BOTTOM,
  LEFT,
  TOP_LEFT,
  TOP_RIGHT,
  BOTTOM_LEFT,
  BOTTOM_RIGHT,
  SINGLE,
  TOP_SINGLE,
  BOTTOM_SINGLE,
  LEFT_SINGLE,
  RIGHT_SINGLE,
  HORIZONTAL_CENTER_SINGLE,
  VERTICAL_CENTER_SINGLE,
  FOURWAY_CONNECT,
  BOTTOM_LEFT_TOP_RIGHT,
  TOP_LEFT_BOTTOM_RIGHT,
  CENTER_TOP_RIGHT,
  CENTER_TOP_LEFT,
  CENTER_BOTTOM_RIGHT,
  CENTER_BOTTOM_LEFT,
  TOP_LEFT_SINGLE,
  TOP_RIGHT_SINGLE,
  BOTTOM_LEFT_SINGLE,
  BOTTOM_RIGHT_SINGLE,
  CONNECT_LEFT_SINGLE,
  CONNECT_RIGHT_SINGLE,
  CONNECT_TOP_SINGLE,
  CONNECT_BOTTOM_SINGLE,
  CONNECT_TOP_BOTTOM_SINGLE,
  CONNECT_BOTTOM_TOP_SINGLE,
  CONNECT_LEFT_RIGHT_SINGLE,
  CONNECT_RIGHT_LEFT_SINGLE,
  CONNECT_TOP_TO_RIGHT_SINGLE,
  CONNECT_TOP_TO_LEFT_SINGLE,
  CONNECT_BOTTOM_TO_RIGHT_SINGLE,
  CONNECT_BOTTOM_TO_LEFT_SINGLE,
  CONNECT_LEFT_TO_TOP_SINGLE,
  CONNECT_LEFT_TO_BOTTOM_SINGLE,
  CONNECT_RIGHT_TO_TOP_SINGLE,
  CONNECT_RIGHT_TO_BOTTOM_SINGLE,
  CONNECT_BOTTOM_LEFT_SINGLE,
  CONNECT_BOTTOM_RIGHT_SINGLE,
  CONNECT_TOP_RIGHT_SINGLE,
  CONNECT_TOP_LEFT_SINGLE,
  TRANSITION_CENTER_BOTTOM_DIRT,
  TRANSITION_CENTER_TOP_DIRT,
  TRANSITION_CENTER_LEFT_DIRT,
  TRANSITION_CENTER_RIGHT_DIRT,
  TRANSITION_CENTER_TOP_LEFT_DIRT,
  TRANSITION_CENTER_TOP_RIGHT_DIRT,
  TRANSITION_CENTER_BOTTOM_LEFT_DIRT,
  TRANSITION_CENTER_BOTTOM_RIGHT_DIRT,
  TRANSITION_CENTER_HORIZONTAL_DIRT,
  TRANSITION_CENTER_VERTICAL_DIRT,
  TILE_STATE_COUNT
} TileState;

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

typedef enum {
  NORTH,
  NORTHEAST,
  EAST,
  SOUTHEAST,
  SOUTH,
  SOUTHWEST,
  WEST,
  NORTHWEST,
} FacingDirection;

typedef struct {
  float posX;
  float posY;
  Vector2 velocity;
  FacingDirection facing;
} Player;

typedef struct GameState {
  Tile tile_map[MAX_TILE_Y][MAX_TILE_X];
  Player player;
} GameState;


static const GameState DefaultGameState = {
  .player = (Player) {
    .posX = 0.0f,
    .posY = 0.0f,
    .facing = EAST,
  }
};

TextureType GetTileType(Tile tile_map[MAX_TILE_Y][MAX_TILE_X], int x, int y) {
  if (x < 0 || y < 0 || x >= MAX_TILE_X || y >= MAX_TILE_Y) {
    return 0; // out of map
  }
  return tile_map[y][x].type;
}

TileState GetTileState(Tile tileMap[MAX_TILE_Y][MAX_TILE_X], int x, int y, TextureType self) {
    // Get neighboring tile types (accounting for reversed rows/columns)
    TextureType top = GetTileType(tileMap, x, y - 1);    // Tile above
    TextureType bottom = GetTileType(tileMap, x, y + 1); // Tile below
    TextureType left = GetTileType(tileMap, x - 1, y);   // Tile to the left
    TextureType right = GetTileType(tileMap, x + 1, y);  // Tile to the right

    TextureType topLeft = GetTileType(tileMap, x - 1, y - 1);    // Top-left diagonal
    TextureType topRight = GetTileType(tileMap, x + 1, y - 1);   // Top-right diagonal
    TextureType bottomLeft = GetTileType(tileMap, x - 1, y + 1); // Bottom-left diagonal
    TextureType bottomRight = GetTileType(tileMap, x + 1, y + 1); // Bottom-right diagonal

    switch (self) {
    case GRASS: {
      // ======== PRIOR 1 ========

      // Case 4-Way Connect (center of 4 diagonal single tiles)
      if (!topLeft && !topRight && !bottomLeft && !bottomRight && top &&
          bottom && left && right) {
        return FOURWAY_CONNECT; // grassTileFourWayConnect; // Special texture
                                // for 4 diagonal tiles
      }

      // Case BottomLeft connected to TopRight
      if (!topLeft && topRight && bottomLeft && !bottomRight && top && bottom &&
          left && right) {
        return BOTTOM_LEFT_TOP_RIGHT; // grassTileBottomLeftTopRight;
      }
      // Case TopLeft connected to BottomRight
      if (topLeft && !topRight && !bottomLeft && bottomRight && top && bottom &&
          left && right) {
        return TOP_LEFT_BOTTOM_RIGHT; // grassTileTopLeftBottomRight;
      }
      // Case surrounded except TopLeft;
      if (!topLeft && topRight && bottomLeft && bottomRight && top && bottom &&
          left && right) {
        return CENTER_TOP_LEFT; // grassTileCenterTopLeft;
      }
      // Case surrounded except TopRight;
      if (topLeft && !topRight && bottomLeft && bottomRight && top && bottom &&
          left && right) {
        return CENTER_TOP_RIGHT; // grassTileCenterTopRight;
      }
      // Case surrounded except BottomLeft;
      if (topLeft && topRight && !bottomLeft && bottomRight && top && bottom &&
          left && right) {
        return CENTER_BOTTOM_LEFT; // grassTileCenterBottomLeft;
      }
      // Case surrounded except BottomRight;
      if (topLeft && topRight && bottomLeft && !bottomRight && top && bottom &&
          left && right) {
        return CENTER_BOTTOM_RIGHT; // grassTileCenterBottomRight;
      }
      // Case TopRight and BottomRight not connected
      if (topLeft && !topRight && bottomLeft && !bottomRight && top && bottom &&
          left && right) {
        return CONNECT_LEFT_SINGLE; // grassTileLeftSingleConnect;
      }
      // Case TopLeft and BottomLeft not connected
      if (!topLeft && topRight && !bottomLeft && bottomRight && top && bottom &&
          left && right) {
        return CONNECT_RIGHT_SINGLE;
        ; // grassTileRightSingleConnect;
      }
      // Case Topleft and TopRight not connected
      if (!topLeft && !topRight && bottomLeft && bottomRight && top && bottom &&
          left && right) {
        return CONNECT_TOP_SINGLE; // grassTileTopSingleConnect;
      }
      // Case Bottomleft and BottomRight not connected
      if (topLeft && topRight && !bottomLeft && !bottomRight && top && bottom &&
          left && right) {
        return CONNECT_BOTTOM_SINGLE; // grassTileBottomSingleConnect;
      }

      // Case BottomLeft tile connect single
      if (!topLeft && topRight && !bottomLeft && !bottomRight && top &&
          bottom && left && right) {
        return CONNECT_BOTTOM_LEFT_SINGLE; // grassTileBottomLeftConnectSingle;
      }
      // Case BottomRight tile connect single
      if (topLeft && !topRight && !bottomLeft && !bottomRight && top &&
          bottom && left && right) {
        return CONNECT_BOTTOM_RIGHT_SINGLE; // grassTileBottomRightConnectSingle;
      }
      // Case TopLeft tile connect single
      if (!topLeft && !topRight && !bottomLeft && bottomRight && top &&
          bottom && left && right) {
        return CONNECT_TOP_LEFT_SINGLE; // grassTileTopLeftConnectSingle;
      }
      // Case TopRight tile connect single
      if (!topLeft && !topRight && bottomLeft && !bottomRight && top &&
          bottom && left && right) {
        return CONNECT_TOP_RIGHT_SINGLE; // grassTileTopRightConnectSingle;
      }

      // ======== PRIOR 2 ========

      // Case TopTile connect SingleRight
      if (bottomLeft && !bottomRight && !top && bottom && left && right) {
        return CONNECT_TOP_TO_RIGHT_SINGLE; // grassTileTopToRightSingleConnect;
      }
      // Case TopTile connect SingleLeft
      if (!bottomLeft && bottomRight && !top && bottom && left && right) {
        return CONNECT_TOP_TO_LEFT_SINGLE; // grassTileTopToLeftSingleConnect;
      }
      // Case BottomTile connect SingleRight
      if (topLeft && !topRight && top && !bottom && left && right) {
        return CONNECT_BOTTOM_TO_RIGHT_SINGLE; // grassTileBottomToRightSingleConnect;
      }
      // Case BottomTile connect SingleLeft
      if (!topLeft && topRight && top && !bottom && left && right) {
        return CONNECT_BOTTOM_TO_LEFT_SINGLE; // grassTileBottomToLeftSingleConnect;
      }
      // Case LeftTile connect SingleTop
      if (bottomRight && !topRight && top && bottom && !left && right) {
        return CONNECT_LEFT_TO_TOP_SINGLE; // grassTileLeftToTopSingleConnect;
      }
      // Case LeftTile connect SingleBottom
      if (!bottomRight && topRight && top && bottom && !left && right) {
        return CONNECT_LEFT_TO_BOTTOM_SINGLE; // grassTileLeftToBottomSingleConnect;
      }
      // Case RightTile connect SingleTop
      if (bottomLeft && !topLeft && top && bottom && left && !right) {
        return CONNECT_RIGHT_TO_TOP_SINGLE; // grassTileRightToTopSingleConnect;
      }
      // Case RightTile connect SingleBottom
      if (!bottomLeft && topLeft && top && bottom && left && !right) {
        return CONNECT_RIGHT_TO_BOTTOM_SINGLE; // grassTileRightToBottomSingleConnect;
      }

      // Single Corners
      // Case SingleTopLeft
      if (!bottomRight && !top && bottom && !left && right) {
        return TOP_LEFT_SINGLE; // grassTileSingleTopLeft;
      }
      // Case SingleTopRight
      if (!bottomLeft && !top && bottom && left && !right) {
        return TOP_RIGHT_SINGLE; // grassTileSingleTopRight;
      }
      // Case SingleBottomLeft
      if (!topRight && top && !bottom && !left && right) {
        return BOTTOM_LEFT_SINGLE; // grassTileSingleBottomLeft;
      }
      // Case SingleBottomRight
      if (!topLeft && top && !bottom && left && !right) {
        return BOTTOM_RIGHT_SINGLE; // grassTileSingleBottomRight;
      }

      // Case SingleTop connect SingeBottom
      if (!bottomLeft && !bottomRight && !top && bottom && left && right) {
        return CONNECT_TOP_BOTTOM_SINGLE; // grassTileSingleTopConnectBottom;
      }
      // Case SingleBottom connect SingeTop
      if (!topLeft && !topRight && top && !bottom && left && right) {
        return CONNECT_BOTTOM_TOP_SINGLE; // grassTileSingleBottomConnectTop;
      }
      // Case SingleLeft connect SingeRight
      if (!topRight && !bottomRight && top && bottom && !left && right) {
        return CONNECT_LEFT_RIGHT_SINGLE; // grassTileSingleLeftConnectRight;
      }
      // Case SingleRight connect SingeLeft
      if (!topLeft && !bottomLeft && top && bottom && left && !right) {
        return CONNECT_RIGHT_LEFT_SINGLE; // grassTileSingleRightConnectLeft;
      }

      // ======== PRIOR 3 ========

      // Case 2: Corners
      if (!top && !left && right && bottom)
        return TOP_LEFT; // grassTileTopLeft;
      if (!top && !right && left && bottom)
        return TOP_RIGHT; // grassTileTopRight;
      if (!bottom && !left && right && top)
        return BOTTOM_LEFT; // grassTileBottomLeft;
      if (!bottom && !right && left && top)
        return BOTTOM_RIGHT; // grassTileBottomRight;


      // grass to dirt transitions
      if(bottomRight == DIRT && top && bottom && bottom != DIRT && left && right && right != DIRT) {
        return TRANSITION_CENTER_BOTTOM_RIGHT_DIRT;
      }
      if(bottomLeft == DIRT && top && bottom && bottom != DIRT && left && left != DIRT && right) {
        return TRANSITION_CENTER_BOTTOM_LEFT_DIRT;
      }
      if(topRight == DIRT && top&& top != DIRT && bottom && left && right && right != DIRT) {
        return TRANSITION_CENTER_TOP_RIGHT_DIRT;
      }
      if(topLeft == DIRT && top && top != DIRT && bottom && left && left != DIRT && right) {
        return TRANSITION_CENTER_TOP_LEFT_DIRT;
      }
      if(top && bottom && left && left == DIRT && right && right == DIRT) {
        return TRANSITION_CENTER_HORIZONTAL_DIRT;
      }
      if(top && top == DIRT && bottom && bottom == DIRT && left && right) {
        return TRANSITION_CENTER_VERTICAL_DIRT;
      }
      if(top && bottom == DIRT && left && right) {
        return TRANSITION_CENTER_BOTTOM_DIRT;
      }
      if(top == DIRT && bottom && left && right) {
        return TRANSITION_CENTER_TOP_DIRT;
      }
      if(top && bottom && left == DIRT && right) {
        return TRANSITION_CENTER_RIGHT_DIRT;
      }
      if(top && bottom && left && right == DIRT) {
        return TRANSITION_CENTER_LEFT_DIRT;
      }

      // Case 3: Edges
      if (!top && bottom && left && right)
        return TOP; // grassTileTop;
      if (!bottom && top && left && right)
        return BOTTOM; // grassTileBottom;
      if (!left && right && top && bottom)
        return LEFT; // grassTileLeft;
      if (!right && left && top && bottom)
        return RIGHT; /// grassTileRight;

      // Case 4: Single row (horizontal line)
      if (!top && !bottom && (left || right)) {
        // Leftmost of single row
        if (!left && right)
          return LEFT_SINGLE; // grassTileLeftSingle;
        // Rightmost of single row
        if (!right && left)
          return RIGHT_SINGLE; // grassTileRightSingle;
        // Middle of single row
        return HORIZONTAL_CENTER_SINGLE; // grassTileSingleHorizontalCenter;
      }

      // Case 5: Single column (vertical line)
      if (!left && !right && (top || bottom)) {
        // Topmost of single column
        if (!top && bottom)
          return TOP_SINGLE; // grassTileTopSingle;
        // Bottommost of single column
        if (!bottom && top)
          return BOTTOM_SINGLE; // grassTileBottomSingle;
        // Middle of single column
        return VERTICAL_CENTER_SINGLE; // grassTileSingleVerticalCenter;
      }

      //Single isolated tile
      if (!top && !bottom && !left && !right) {
        return SINGLE; // grassTileSingle;
      }
      break;
    }
    case DIRT: {

      if (!top && !left && right && bottom)
        return TOP_LEFT; // grassTileTopLeft;
      if (!top && !right && left && bottom)
        return TOP_RIGHT; // grassTileTopRight;
      if (!bottom && !left && right && top)
        return BOTTOM_LEFT; // grassTileBottomLeft;
      if (!bottom && !right && left && top)
        return BOTTOM_RIGHT; // grassTileBottomRight;

      if (!top && bottom && left && right)
        return TOP; // grassTileTop;
      if (!bottom && top && left && right)
        return BOTTOM; // grassTileBottom;
      if (!left && right && top && bottom)
        return LEFT; // grassTileLeft;
      if (!right && left && top && bottom)
        return RIGHT; /// grassTileRight;
      break;
    }
    default: {
      break;
    }
    }

    return CENTER;
}


static Rectangle EntityTextures[TEXTURE_TYPE_COUNT][ENTITY_STATE_COUNT] = {
  [PLAYER] = {
    [ENTITY_IDLE] = { 1008.0f, 1040.0f,  16.0f, 16.0f },
  }
};

static Rectangle TileTextures[TEXTURE_TYPE_COUNT][TILE_STATE_COUNT] = {
  [GRASS] = {
    [CENTER] = {16.0f, 16.0f, 16.0f, 16.0f},
    [TOP] = {16.0f, 0.0f, 16.0f, 16.0f},
    [LEFT] = {0.0f, 16.0f, 16.0f, 16.0f},
    [RIGHT] = {32.0f, 16.0f, 16.0f, 16.0f},
    [TOP_LEFT] = {0.0f, 0.0f, 16.0f, 16.0f},
    [TOP_RIGHT] = {32.0f, 0.0f, 16.0f, 16.0f},
    [BOTTOM_LEFT] = {0.0f, 32.0f, 16.0f, 16.0f},
    [BOTTOM] = {16.0f, 32.0f, 16.0f, 16.0f},
    [BOTTOM_RIGHT] = {32.0f, 32.0f, 16.0f, 16.0f},
    [SINGLE] = { 48.0f, 48.0f, 16.0f, 16.0f },
    [TOP_SINGLE] = { 48.0f, 0.0f, 16.0f, 16.0f },
    [BOTTOM_SINGLE] = { 48.0f, 32.0f, 16.0f, 16.0f },
    [LEFT_SINGLE] = { 0.0f, 48.0f, 16.0f, 16.0f },
    [RIGHT_SINGLE] = { 32.0f, 48.0f, 16.0f, 16.0f },
    [HORIZONTAL_CENTER_SINGLE] = { 16.0f, 48.0f, 16.0f, 16.0f },
    [VERTICAL_CENTER_SINGLE] = { 48.0f, 16.0f, 16.0f, 16.0f },
    [FOURWAY_CONNECT] = { 128.0f, 64.0f, 16.0f, 16.0f },
    [BOTTOM_LEFT_TOP_RIGHT] = { 144.0f, 0.0f, 16.0f, 16.0f },
    [TOP_LEFT_BOTTOM_RIGHT] = { 144.0f, 16.0f, 16.0f, 16.0f },
    [CENTER_BOTTOM_RIGHT] = { 80.0f, 16.0f, 16.0f, 16.0f },
    [CENTER_BOTTOM_LEFT]= { 96.0f, 16.0f, 16.0f, 16.0f },
    [CENTER_TOP_RIGHT] = { 80.0f, 32.0f, 16.0f, 16.0f },
    [CENTER_TOP_LEFT] = { 96.0f, 32.0f, 16.0f, 16.0f },
    [CONNECT_LEFT_SINGLE] = { 80.0f, 64.0f, 16.0f, 16.0f },
    [CONNECT_RIGHT_SINGLE] = { 96.0f, 64.0f, 16.0f, 16.0f },
    [CONNECT_TOP_SINGLE] = { 128.0f, 32.0f, 16.0f, 16.0f },
    [CONNECT_BOTTOM_SINGLE] = { 128.0f, 16.0f, 16.0f, 16.0f },
    [CONNECT_TOP_BOTTOM_SINGLE] = { 128.0f, 0.0f, 16.0f, 16.0f },
    [CONNECT_BOTTOM_TOP_SINGLE] = { 128.0f, 48.0f, 16.0f, 16.0f },
    [CONNECT_LEFT_RIGHT_SINGLE] = { 64.0f, 64.0f, 16.0f, 16.0f },
    [CONNECT_RIGHT_LEFT_SINGLE] = { 112.0f, 64.0f, 16.0f, 16.0f },
    [TOP_LEFT_SINGLE] = { 64.0f, 0.0f, 16.0f, 16.0f },
    [TOP_RIGHT_SINGLE] = { 112.0f, 0.0f, 16.0f, 16.0f },
    [BOTTOM_LEFT_SINGLE] = { 64.0f, 48.0f, 16.0f, 16.0f },
    [BOTTOM_RIGHT_SINGLE] = { 112.0f, 48.0f, 16.0f, 16.0f },
    [CONNECT_TOP_TO_RIGHT_SINGLE] = { 80.0f, 0.0f, 16.0f, 16.0f },
    [CONNECT_TOP_TO_LEFT_SINGLE] = { 96.0f, 0.0f, 16.0f, 16.0f },
    [CONNECT_BOTTOM_TO_RIGHT_SINGLE] = { 80.0f, 48.0f, 16.0f, 16.0f },
    [CONNECT_BOTTOM_TO_LEFT_SINGLE] = { 96.0f, 48.0f, 16.0f, 16.0f },
    [CONNECT_LEFT_TO_TOP_SINGLE] = { 64.0f, 32.0f, 16.0f, 16.0f },
    [CONNECT_LEFT_TO_BOTTOM_SINGLE] = { 64.0f, 16.0f, 16.0f, 16.0f },
    [CONNECT_RIGHT_TO_TOP_SINGLE] = { 112.0f, 32.0f, 16.0f, 16.0f },
    [CONNECT_RIGHT_TO_BOTTOM_SINGLE] = { 112.0f, 16.0f, 16.0f, 16.0f },
    [CONNECT_BOTTOM_LEFT_SINGLE]= { 144.0f, 48.0f, 16.0f, 16.0f },
    [CONNECT_TOP_RIGHT_SINGLE] = { 160.0f, 32.0f, 16.0f, 16.0f },
    [CONNECT_TOP_LEFT_SINGLE] = { 144.0f, 32.0f, 16.0f, 16.0f },
    [CONNECT_BOTTOM_RIGHT_SINGLE] = { 160.0f, 48.0f, 16.0f, 16.0f },
    [TRANSITION_CENTER_BOTTOM_DIRT] = { 16.0f, 112.0f, 16.0f, 16.0f },
    [TRANSITION_CENTER_TOP_DIRT] = { 16.0f, 144.0f, 16.0f, 16.0f },
    [TRANSITION_CENTER_LEFT_DIRT] = { 0.0f, 128.0f, 16.0f, 16.0f },
    [TRANSITION_CENTER_RIGHT_DIRT] = { 32.0f, 128.0f, 16.0f, 16.0f },
    [TRANSITION_CENTER_TOP_RIGHT_DIRT] = { 0.0f, 144.0f, 16.0f, 16.0f },
    [TRANSITION_CENTER_TOP_LEFT_DIRT] = { 32.0f, 144.0f, 16.0f, 16.0f },
    [TRANSITION_CENTER_BOTTOM_RIGHT_DIRT] = { 0.0f, 112.0f, 16.0f, 16.0f },
    [TRANSITION_CENTER_BOTTOM_LEFT_DIRT] = { 32.0f, 112.0f, 16.0f, 16.0f },
    [TRANSITION_CENTER_HORIZONTAL_DIRT] = { 0.0f, 160.0f, 16.0f, 16.0f },
    [TRANSITION_CENTER_VERTICAL_DIRT] = { 16.0f, 160.0f, 16.0f, 16.0f },
  },
  [DIRT] = {
    [CENTER] = { 192.0f, 16.0f, 16.0f, 16.0f },
    [TOP] = {192.0f, 0.0f, 16.0f, 16.0f},
    [LEFT] = {176.0f, 16.0f, 16.0f, 16.0f},
    [RIGHT] = {208.0f, 16.0f, 16.0f, 16.0f},
    [TOP_LEFT] = {176.0f, 0.0f, 16.0f, 16.0f},
    [TOP_RIGHT] = {208.0f, 0.0f, 16.0f, 16.0f},
    [BOTTOM_LEFT] = {176.0f, 32.0f, 16.0f, 16.0f},
    [BOTTOM] = {192.0f, 32.0f, 16.0f, 16.0f},
    [BOTTOM_RIGHT] = {208.0f, 32.0f, 16.0f, 16.0f},
    [SINGLE] = {224.0f, 48.0f, 16.0f, 16.0f},
    [TOP_SINGLE] ={224.0f, 0.0f, 16.0f, 16.0f},
    [BOTTOM_SINGLE] = {224.0f, 32.0f, 16.0f, 16.0f},
    [LEFT_SINGLE] ={176.0f, 48.0f, 16.0f, 16.0f},
    [RIGHT_SINGLE] = {208.0f, 48.0f, 16.0f, 16.0f},
    [HORIZONTAL_CENTER_SINGLE] = {192.0f, 48.0f, 16.0f, 16.0f},
    [VERTICAL_CENTER_SINGLE] = {224.0f, 16.0f, 16.0f, 16.0f},
    [FOURWAY_CONNECT] = {304.0f, 64.0f, 16.0f, 16.0f},
    [BOTTOM_LEFT_TOP_RIGHT] = {0.0f, 0.0f, 16.0f, 16.0f},
    [TOP_LEFT_BOTTOM_RIGHT] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CENTER_BOTTOM_RIGHT] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CENTER_BOTTOM_LEFT]= {0.0f, 0.0f, 16.0f, 16.0f},
    [CENTER_TOP_RIGHT] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CENTER_TOP_LEFT] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_LEFT_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_RIGHT_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_TOP_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_BOTTOM_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_TOP_BOTTOM_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_BOTTOM_TOP_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_LEFT_RIGHT_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_RIGHT_LEFT_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [TOP_LEFT_SINGLE] ={0.0f, 0.0f, 16.0f, 16.0f},
    [TOP_RIGHT_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [BOTTOM_LEFT_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [BOTTOM_RIGHT_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_TOP_TO_RIGHT_SINGLE] ={0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_TOP_TO_LEFT_SINGLE] ={0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_BOTTOM_TO_RIGHT_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_BOTTOM_TO_LEFT_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_LEFT_TO_TOP_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_LEFT_TO_BOTTOM_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_RIGHT_TO_TOP_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_RIGHT_TO_BOTTOM_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_BOTTOM_LEFT_SINGLE]= {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_TOP_RIGHT_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_TOP_LEFT_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
    [CONNECT_BOTTOM_RIGHT_SINGLE] = {0.0f, 0.0f, 16.0f, 16.0f},
  }
};

int main() {

  const int screenHeight = 1080;
  const int screenWidth = 1920;

  InitWindow(screenWidth, screenHeight, "ALLFARM");

  Texture2D atlas = LoadTexture("Assets/TextureAtlas.png");

  Camera2D camera = {0};
  CameraState cameraState = {.scaleFactor = 1.0f};

  GameState gameState = DefaultGameState;

  int debugGrid = 0;

  for (int y = 0; y < MAX_TILE_Y; y++) {
    for (int x = 0; x < MAX_TILE_X; x++) {
      gameState.tile_map[y][x] = (Tile){.posX = x * TILE_SIZE,
                                        .posY = y * TILE_SIZE,
                                        .type = 1};
    }
  }

  Player* player = &gameState.player;

  camera.target =
      (Vector2){player->posX, player->posY}; // World-space point the camera focuses on
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  SetTargetFPS(60);
  ToggleFullscreen();

  while (!WindowShouldClose()) {
    /*if (IsKeyDown(KEY_SPACE)) {*/
    /*  camera.offset = (Vector2){0.0f, 0.0f};*/
    /*  camera.target = Vector2Subtract(*/
    /*      GetScreenToWorld2D((Vector2){0.0f, 0.0f}, camera), camera.target);*/
    /*}*/

    camera.target.x = player->posX + 20.0f;
    camera.target.y = player->posY + 20.f;

    camera.offset = (Vector2) { .x = screenWidth / 2.0f, .y = screenHeight / 2.0f };

    if(IsKeyPressed(KEY_G)) {
      debugGrid = !debugGrid;
    }

    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      /*camera.offset = GetMousePosition();*/
      /*camera.target = mouseWorldPos;*/
      cameraState.scaleFactor = 1.1f + (0.25f * fabsf(wheel));
      printf("%f\n", wheel);
      if (wheel < 0)
        cameraState.scaleFactor = 1.0f/cameraState.scaleFactor ;
      camera.zoom = Clamp(camera.zoom * cameraState.scaleFactor, 0.125f, 64.0f);
    }
    if (camera.zoom > 5.0f)
      camera.zoom = 5.0f;
    if (camera.zoom < 0.5f)
      camera.zoom = 0.5f;

    // Determine grid cell
    int cellX = (int)floor(player->posX / TILE_SIZE);
    int cellY = (int)floor(player->posY / TILE_SIZE);

    // Calculate world coordinates of the top-left corner of the hovered cell
    Vector2 cellWorldPos = (Vector2){cellX * TILE_SIZE, cellY * TILE_SIZE};

    BeginDrawing();
    ClearBackground(DARKGRAY);
    BeginMode2D(camera);

    // SELECT TILE
    if ((cellWorldPos.x < MAX_TILE_X * TILE_SIZE && cellWorldPos.x >= 0) &&
        (cellWorldPos.y < MAX_TILE_Y * TILE_SIZE && cellWorldPos.y >= 0)) {
      Tile *tile = &gameState.tile_map[cellY][cellX];
      // CHANGE TILE
      if (IsKeyDown(KEY_E)) {
        tile->type = 1;
      }
      if (IsKeyDown(KEY_F)) {
        tile->type = 2;
      }
      // REMOVE TILE
      if (IsKeyDown(KEY_Q)) {
        tile->type = 0;
      }
    }


    for (int tileY = 0; tileY < MAX_TILE_Y; tileY++) {
      for (int tileX = 0; tileX < MAX_TILE_X; tileX++) {
        Tile* curTile = &gameState.tile_map[tileY][tileX];

        // nothing to draw
        if (curTile->type == 0) continue;

        TileState tile_state = GetTileState(gameState.tile_map, tileX, tileY, curTile->type);
        Rectangle src_rect = TileTextures[curTile->type][tile_state];

        DrawTexturePro(
            atlas, src_rect,
            (Rectangle){
                .x = curTile->posX, .y = curTile->posY, TILE_SIZE, TILE_SIZE},
            (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
      }
    }


    if(debugGrid) {
      for (int gridIdx = 0; gridIdx <= MAX_TILE_X * TILE_SIZE;
           gridIdx += TILE_SIZE) {
        DrawLine(gridIdx, 0, gridIdx, MAX_TILE_Y * TILE_SIZE, RAYWHITE);
      }
      for (int gridIdx = 0; gridIdx <= MAX_TILE_Y * TILE_SIZE;
           gridIdx += TILE_SIZE) {
        DrawLine(0, gridIdx, MAX_TILE_X * TILE_SIZE, gridIdx, RAYWHITE);
      }
    }

    // Player
    Rectangle* player_rect = EntityTextures[PLAYER];

    int player_accel = 200;
    if(IsKeyDown(KEY_LEFT_SHIFT))
      player_accel *= 2;

    int input_dirs[4] = {
      IsKeyDown(KEY_A),
      IsKeyDown(KEY_D),
      IsKeyDown(KEY_W),
      IsKeyDown(KEY_S)
    };

    float velocity_x = (input_dirs[1] - input_dirs[0]) * player_accel;
    float velocity_y = (input_dirs[3] - input_dirs[2]) * player_accel;
    player->posX += velocity_x * GetFrameTime();
    player->posY += velocity_y * GetFrameTime();


    DrawTexturePro(atlas, player_rect[ENTITY_IDLE],
        (Rectangle) {
          .x = player->posX,
          .y = player->posY,
          64.0f,
          64.0f,
        },
        (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);

    DrawRectangle(cellWorldPos.x, cellWorldPos.y, TILE_SIZE, TILE_SIZE, (Color) { 255, 255 ,255, 50 });

    EndMode2D();

    // top left text
    char buffer[5000];
    sprintf(buffer, "%.f, %.f", round(cellWorldPos.x / TILE_SIZE),
            round(cellWorldPos.y / TILE_SIZE));
    DrawText(buffer, 10, 50, 20, WHITE);
    sprintf(buffer, "%.f, %.f", cellWorldPos.x, cellWorldPos.y);
    DrawText(buffer, 10, 100, 20, WHITE);
    sprintf(buffer, "%f", cameraState.scaleFactor);
    DrawText(buffer, 10, 150, 20, WHITE);
    sprintf(buffer, "%d", player->facing);
    DrawText(buffer, 10, 200, 20, WHITE);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define HOLES 5

// ------------------- Game States -------------------
typedef enum { STATE_MENU, STATE_GAME, STATE_PAUSE, STATE_VICTORY } GameState;

// ------------------- Mole Struct -------------------
typedef struct {
    Vector2 pos;
    bool isVisible;
    int type;       // 0=normal,1=golden,2=bomber,3=empty
    float timer;
    bool isHit;
} Mole;

typedef struct { Texture2D normal; Texture2D hit; } MoleSprites;

// ------------------- Hammer Struct -------------------
typedef struct {
    Texture2D normal;
    Texture2D hit;
    Vector2 pos;
    Vector2 targetPos;
    Vector2 startPos;
    bool isHitting;
    float animTime;
    float idleTime;
} Hammer;

// ------------------- Virtual Buttons -------------------
typedef struct { Rectangle rect; } VirtualButton;

// ------------------- Menu Buttons -------------------
typedef struct {
    Rectangle rect;
    const char *label;
    bool isHovered;
} GameButton;

// ------------------- Globals -------------------
GameState currentState = STATE_MENU;

// Backgrounds
Texture2D backgroundMenu;
Texture2D backgroundGame;

// Moles
Mole holes[HOLES];
MoleSprites moleNormal, moleGolden, moleBomber;

// Hammers
Hammer hammerRed;
Hammer hammerBlue;

// Star for victory
Texture2D starTexture;

// Sounds
Sound sndMolePop, sndHitNormal, sndHitGolden, sndHitBomber, sndHitEmpty;
Sound sndButton, sndVictory;
Music bgm;

// Font
Font myFont;

// Hole positions
Vector2 holePositions[HOLES] = {
    {707, 510}, 
    {1133, 510}, 
    {376, 640}, 
    {940, 700}, 
    {1490, 640}
};

// Scores and timer
int scoreRed = 0;
int scoreBlue = 0;
float roundTime = 101.0f;
float timer = 0.0f;

// Keys for hitting holes
KeyboardKey redKeys[HOLES] = { KEY_S, KEY_E, KEY_Z, KEY_R, KEY_G };
KeyboardKey blueKeys[HOLES] = { KEY_H, KEY_U, KEY_B, KEY_I, KEY_L };

// Gameplay buttons
VirtualButton redButtons[HOLES];
VirtualButton blueButtons[HOLES];

// Pause button
VirtualButton pauseButton;

// Menu buttons
GameButton mainMenuButtons[3] = {
    {{0,0,0,0}, "1. New Game", false},
    {{0,0,0,0}, "2. Resume", false},
    {{0,0,0,0}, "3. Exit", false}
};

GameButton pauseMenuButtons[3] = {
    {{0,0,0,0}, "1. Resume", false},
    {{0,0,0,0}, "2. Menu", false},
    {{0,0,0,0}, "3. Exit", false}
};

GameButton victoryMenuButtons[3] = {
    {{0,0,0,0}, "1. Replay", false},
    {{0,0,0,0}, "2. Menu", false},
    {{0,0,0,0}, "3. Exit", false}
};

// Edge/menu button textures
Texture2D redBoxTexture;
Texture2D blueBoxTexture;
Texture2D whiteBoxTexture;
Texture2D greenBoxTexture;
// ------------------- Load Assets -------------------
void LoadAssets() {
    // Backgrounds
    backgroundMenu = LoadTexture("assets/visual/menu_background.png");
    backgroundGame = LoadTexture("assets/visual/game_background.png");

    // Moles
    moleNormal.normal = LoadTexture("assets/visual/mole_normal.png");
    moleNormal.hit = LoadTexture("assets/visual/mole_normal_hit.png");
    moleGolden.normal = LoadTexture("assets/visual/mole_golden.png");
    moleGolden.hit = LoadTexture("assets/visual/mole_golden_hit.png");
    moleBomber.normal = LoadTexture("assets/visual/mole_bomber.png");
    moleBomber.hit = LoadTexture("assets/visual/mole_bomber_hit.png");

    // Hammers
    hammerRed.normal = LoadTexture("assets/visual/hammer_red.png");
    hammerRed.hit = LoadTexture("assets/visual/hammer_red_hit.png");
    hammerBlue.normal = LoadTexture("assets/visual/hammer_blue.png");
    hammerBlue.hit = LoadTexture("assets/visual/hammer_blue_hit.png");

    // Victory star
    starTexture = LoadTexture("assets/visual/star.png");

    // Edge/Menu buttons
    redBoxTexture = LoadTexture("assets/visual/red_box.png");
    blueBoxTexture = LoadTexture("assets/visual/blue_box.png");
    whiteBoxTexture = LoadTexture("assets/visual/black_box.png");
    greenBoxTexture = LoadTexture("assets/visual/green_box.png");

    // Sounds
    sndMolePop = LoadSound("assets/audio/mole_pop.wav");
    sndHitNormal = LoadSound("assets/audio/hit_normal.wav");
    sndHitGolden = LoadSound("assets/audio/hit_golden.wav");
    sndHitBomber = LoadSound("assets/audio/hit_bomber.wav");
    sndHitEmpty = LoadSound("assets/audio/hit_empty.wav");
    sndButton = LoadSound("assets/audio/button.wav");
    sndVictory = LoadSound("assets/audio/victory.ogg");

    bgm = LoadMusicStream("assets/audio/bgm.ogg");
    SetMusicVolume(bgm, 0.3f);
    PlayMusicStream(bgm);

    // Font
    myFont = LoadFont("assets/font/myfont.ttf");
}

// ------------------- Initialize Game -------------------
void InitGame() {
    scoreRed = 0;
    scoreBlue = 0;
    timer = roundTime;

    Vector2 redButtonPositions[HOLES] = {
    {50, 330},   // match hole 0 ->button:02
    {50, 480},   // match hole 1 ->button:03
    {50, 180},   // match hole 2 ->button:01
    {50, 630},   // match hole 3 ->button:4
    {50, 780}    // match hole 4 ->button:5
};

Vector2 blueButtonPositions[HOLES] = {
    {SCREEN_WIDTH-170, 630},    // match hole 0 ->button:04
    {SCREEN_WIDTH-170, 480},    // match hole 1 ->button:03
    {SCREEN_WIDTH-170, 780},    // match hole 2 ->button:05
    {SCREEN_WIDTH-170, 330},    // match hole 3 ->button:02
    {SCREEN_WIDTH-170, 180}     // match hole 4 ->button:01
};

for(int i=0;i<HOLES;i++){
    holes[i].isVisible = false;
    holes[i].isHit = false;
    holes[i].timer = 0.0f;

    // Assign new left/right edge button positions
    redButtons[i].rect = (Rectangle){redButtonPositions[i].x, redButtonPositions[i].y, 120, 120};
    blueButtons[i].rect = (Rectangle){blueButtonPositions[i].x, blueButtonPositions[i].y, 120, 120};
}

    hammerRed.pos = hammerRed.startPos = hammerRed.targetPos = (Vector2){SCREEN_WIDTH-1750, SCREEN_HEIGHT/2-100};
    hammerBlue.pos = hammerBlue.startPos = hammerBlue.targetPos = (Vector2){SCREEN_WIDTH-350, SCREEN_HEIGHT/2-100};
    hammerRed.isHitting = hammerBlue.isHitting = false;
    hammerRed.animTime = hammerBlue.animTime = 0;
    hammerRed.idleTime = hammerBlue.idleTime = 0;

    // Pause button
    pauseButton.rect = (Rectangle){SCREEN_WIDTH/2-100, SCREEN_HEIGHT-150, 200, 80};

    // Menu button rectangles
    for(int i=0;i<3;i++){
        mainMenuButtons[i].rect = (Rectangle){SCREEN_WIDTH/2-200, 300 + i*100, 400, 60};
        pauseMenuButtons[i].rect = (Rectangle){SCREEN_WIDTH/2-200, 300 + i*100, 400, 60};
        victoryMenuButtons[i].rect = (Rectangle){SCREEN_WIDTH/2-200, 300 + i*100, 400, 60};
    }
}

// ------------------- Mole & Hit Functions -------------------
// Spawn a mole in hole i
void SpawnMole(int i){
    int r = GetRandomValue(0,99);
    if(r<60) holes[i].type = 0;        // Normal
    else if(r<80) holes[i].type = 2;   // Bomber
    else if(r<85) holes[i].type = 1;   // Golden
    else holes[i].type = 3;             // Empty

    holes[i].isVisible = true;
    holes[i].isHit = false;
    holes[i].timer = 1.0f;
    PlaySound(sndMolePop);
}

// Update all moles
void UpdateMoles(){
    for(int i=0;i<HOLES;i++){
        if(holes[i].isVisible){
            holes[i].timer -= GetFrameTime();
            if(holes[i].timer <= 0) holes[i].isVisible = false;
        }else{
            if(GetRandomValue(0,1000) < 5) SpawnMole(i);
        }
    }
}

// Handle hammer hit for a hole
void HandleHit(int holeIndex, bool isRed){
    int *score = isRed ? &scoreRed : &scoreBlue;

    if(!holes[holeIndex].isVisible){
        *score -= 1;
        if(*score < 0) *score = 0;
        PlaySound(sndHitEmpty);
    }else{
        if(holes[holeIndex].isHit) return;
        holes[holeIndex].isHit = true;

        switch(holes[holeIndex].type){
            case 0: *score += 5; PlaySound(sndHitNormal); break;
            case 1: *score += 10; PlaySound(sndHitGolden); break;
            case 2: *score -= 5; if(*score < 0) *score = 0; PlaySound(sndHitBomber); break;
            case 3: *score -= 1; if(*score < 0) *score = 0; PlaySound(sndHitEmpty); break;
        }
    }

    // Move hammer animation
    if(isRed){
        hammerRed.targetPos = (Vector2){holePositions[holeIndex].x - hammerRed.normal.width/2,
                                        holePositions[holeIndex].y - hammerRed.normal.height/2};
        hammerRed.isHitting = true;
        hammerRed.idleTime = 0;
    }else{
        hammerBlue.targetPos = (Vector2){holePositions[holeIndex].x - hammerBlue.normal.width/2,
                                         holePositions[holeIndex].y - hammerBlue.normal.height/2};
        hammerBlue.isHitting = true;
        hammerBlue.idleTime = 0;
    }
}
    // ------------------- Main Game Loop Input & State -------------------
void UpdateGameState(float deltaTime, Vector2 mousePos, bool *gamePaused){
    // ------------------- Menu State -------------------
    if(currentState == STATE_MENU){
        for(int i=0;i<3;i++){
            mainMenuButtons[i].isHovered = CheckCollisionPointRec(mousePos, mainMenuButtons[i].rect);
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mainMenuButtons[i].isHovered) PlaySound(sndButton);
        }
        if(IsKeyPressed(KEY_ONE) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mainMenuButtons[0].isHovered)){
            InitGame(); currentState = STATE_GAME; *gamePaused=false;
        }
        if(IsKeyPressed(KEY_TWO) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mainMenuButtons[1].isHovered)){
            currentState = STATE_GAME; *gamePaused=false;
        }
        if(IsKeyPressed(KEY_THREE) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mainMenuButtons[2].isHovered)){
            CloseWindow(); exit(0);
        }
    }

    // ------------------- Game State -------------------
    else if(currentState == STATE_GAME){
        if(!(*gamePaused)) timer -= deltaTime;
        if(timer <= 0){
            currentState = STATE_VICTORY;
            PlaySound(sndVictory);
        }

        UpdateMoles();

        // Handle hammer hits with keys or virtual buttons
        for(int i=0;i<HOLES;i++){
            if(IsKeyPressed(redKeys[i]) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, redButtons[i].rect)))
                HandleHit(i,true);

            if(IsKeyPressed(blueKeys[i]) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, blueButtons[i].rect)))
                HandleHit(i,false);
        }

        // Pause
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, pauseButton.rect)){
            currentState = STATE_PAUSE;
            PlaySound(sndButton);
            *gamePaused = true;
        }
        if(IsKeyPressed(KEY_SPACE)){
            currentState = STATE_PAUSE;
            PlaySound(sndButton);
            *gamePaused = true;
        }

        // Hammer idle movement
        hammerRed.idleTime += deltaTime; 
        hammerBlue.idleTime += deltaTime;

        if(hammerRed.idleTime > 1.0f) hammerRed.targetPos = hammerRed.startPos;
        if(hammerBlue.idleTime > 1.0f) hammerBlue.targetPos = hammerBlue.startPos;

        float speed = 15.0f;
        hammerRed.pos.x += (hammerRed.targetPos.x - hammerRed.pos.x) * speed * deltaTime;
        hammerRed.pos.y += (hammerRed.targetPos.y - hammerRed.pos.y) * speed * deltaTime;
        hammerBlue.pos.x += (hammerBlue.targetPos.x - hammerBlue.pos.x) * speed * deltaTime;
        hammerBlue.pos.y += (hammerBlue.targetPos.y - hammerBlue.pos.y) * speed * deltaTime;

        if(hammerRed.isHitting){
            hammerRed.animTime += deltaTime; 
            if(hammerRed.animTime > 0.2f){ hammerRed.isHitting = false; hammerRed.animTime = 0; }
        }
        if(hammerBlue.isHitting){
            hammerBlue.animTime += deltaTime; 
            if(hammerBlue.animTime > 0.2f){ hammerBlue.isHitting = false; hammerBlue.animTime = 0; }
        }
    }

    // ------------------- Pause State -------------------
    else if(currentState == STATE_PAUSE){
        for(int i=0;i<3;i++){
            pauseMenuButtons[i].isHovered = CheckCollisionPointRec(mousePos, pauseMenuButtons[i].rect);
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && pauseMenuButtons[i].isHovered) PlaySound(sndButton);
        }

        if(IsKeyPressed(KEY_ONE) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && pauseMenuButtons[0].isHovered)){
            currentState = STATE_GAME; *gamePaused=false;
        }
        if(IsKeyPressed(KEY_TWO) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && pauseMenuButtons[1].isHovered)){
            currentState = STATE_MENU;
        }
        if(IsKeyPressed(KEY_THREE) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && pauseMenuButtons[2].isHovered)){
            CloseWindow(); exit(0);
        }
    }

    // ------------------- Victory State -------------------
    else if(currentState == STATE_VICTORY){
        for(int i=0;i<3;i++){
            victoryMenuButtons[i].isHovered = CheckCollisionPointRec(mousePos, victoryMenuButtons[i].rect);
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && victoryMenuButtons[i].isHovered) PlaySound(sndButton);
        }

        if(IsKeyPressed(KEY_ONE) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && victoryMenuButtons[0].isHovered)){
            InitGame(); currentState = STATE_GAME; *gamePaused=false;
        }
        if(IsKeyPressed(KEY_TWO) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && victoryMenuButtons[1].isHovered)){
            currentState = STATE_MENU;
        }
        if(IsKeyPressed(KEY_THREE) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && victoryMenuButtons[2].isHovered)){
            CloseWindow(); exit(0);
        }
    }
}
    // ------------------- Drawing Function -------------------
void DrawGame(Vector2 mousePos){
    BeginDrawing();
    ClearBackground(BLACK);

    // ------------------- Menu -------------------
    if(currentState == STATE_MENU){
        DrawTexture(backgroundMenu,0,0,WHITE);
        DrawTextEx(myFont, "Whac-A-Mole", (Vector2){SCREEN_WIDTH/2 - MeasureTextEx(myFont, "Whac-A-Mole", 70, 5).x/2, 150}, 70, 5, GOLD);

        for(int i=0;i<3;i++){
            // Draw menu buttons with white box texture
            DrawTexture(whiteBoxTexture, mainMenuButtons[i].rect.x, mainMenuButtons[i].rect.y, WHITE);

            Color textColor = mainMenuButtons[i].isHovered ? YELLOW : WHITE;
            DrawTextEx(myFont, mainMenuButtons[i].label, 
                (Vector2){mainMenuButtons[i].rect.x + 20, mainMenuButtons[i].rect.y + 10}, 50, 2, textColor);
        }
    }

    // ------------------- Game -------------------
    else if(currentState == STATE_GAME){
        DrawTexture(backgroundGame,0,0,WHITE);

        // Scores & Timer
        DrawTextEx(myFont, TextFormat("Red Team: %d", scoreRed), (Vector2){30,30}, 40, 2, RED);
        DrawTextEx(myFont, TextFormat("Blue Team: %d", scoreBlue), (Vector2){SCREEN_WIDTH-350,30}, 40, 2, BLUE);
        DrawTextEx(myFont, TextFormat("Time: %d", (int)timer), (Vector2){SCREEN_WIDTH/2-50,30}, 40, 2, YELLOW);

        // Draw Moles
        for(int i=0;i<HOLES;i++){
            if(holes[i].isVisible){
                Texture2D tex;
                if(holes[i].type==0) tex = holes[i].isHit ? moleNormal.hit : moleNormal.normal;
                else if(holes[i].type==1) tex = holes[i].isHit ? moleGolden.hit : moleGolden.normal;
                else if(holes[i].type==2) tex = holes[i].isHit ? moleBomber.hit : moleBomber.normal;
                else continue;

                DrawTexture(tex, holePositions[i].x - tex.width/2, holePositions[i].y - tex.height/2, WHITE);
            }

            // ------------------- Hole Indicators -------------------
            // Format: "Z - B", Z=red, B=blue, - = white
            char indicator[6];
            sprintf(indicator, "%c - %c", redKeys[i], blueKeys[i]);
            Vector2 textSize = MeasureTextEx(myFont, indicator, 30, 1);
            DrawTextEx(myFont, indicator, 
                (Vector2){holePositions[i].x - textSize.x/2, holePositions[i].y + 60}, 30, 1, WHITE);
        }

        // Draw left & right edge buttons
        for(int i=0;i<HOLES;i++){
            DrawTexture(redBoxTexture, redButtons[i].rect.x, redButtons[i].rect.y, WHITE);
            DrawTextEx(myFont, TextFormat("%c", redKeys[i]), 
                (Vector2){redButtons[i].rect.x+10, redButtons[i].rect.y+10}, 30, 1, WHITE);

            DrawTexture(blueBoxTexture, blueButtons[i].rect.x, blueButtons[i].rect.y, WHITE);
            DrawTextEx(myFont, TextFormat("%c", blueKeys[i]), 
                (Vector2){blueButtons[i].rect.x+10, blueButtons[i].rect.y+10}, 30, 1, WHITE);
        }

        // Draw hammers
        DrawTexture(hammerRed.isHitting ? hammerRed.hit : hammerRed.normal, hammerRed.pos.x, hammerRed.pos.y, WHITE);
        DrawTexture(hammerBlue.isHitting ? hammerBlue.hit : hammerBlue.normal, hammerBlue.pos.x, hammerBlue.pos.y, WHITE);

        // Draw pause button
        DrawTexture(greenBoxTexture, pauseButton.rect.x, pauseButton.rect.y, WHITE);
        DrawTextEx(myFont, "PAUSE", (Vector2){pauseButton.rect.x+20, pauseButton.rect.y+15}, 40, 2, WHITE);
    }

    // ------------------- Pause -------------------
    else if(currentState == STATE_PAUSE){
        DrawTexture(backgroundMenu,0,0,WHITE);
        DrawTextEx(myFont, "PAUSED", (Vector2){SCREEN_WIDTH/2 - MeasureTextEx(myFont, "PAUSED", 80, 5).x/2,150}, 80, 5, YELLOW);

        for(int i=0;i<3;i++){
            DrawTexture(whiteBoxTexture, pauseMenuButtons[i].rect.x, pauseMenuButtons[i].rect.y, WHITE);
            Color textColor = pauseMenuButtons[i].isHovered ? YELLOW : WHITE;
            DrawTextEx(myFont, pauseMenuButtons[i].label, 
                (Vector2){pauseMenuButtons[i].rect.x + 20, pauseMenuButtons[i].rect.y + 10}, 50, 2, textColor);
        }
    }

    // ------------------- Victory -------------------
    else if(currentState == STATE_VICTORY){
        DrawTexture(backgroundMenu,0,0,WHITE);

        const char* winnerText;
Color winnerColor;

if(scoreRed > scoreBlue){
    winnerText = "Red Team Wins!";
    winnerColor = RED;
}else if(scoreBlue > scoreRed){
    winnerText = "Blue Team Wins!";
    winnerColor = BLUE;
}else{
    winnerText = "Match Draw!";
    winnerColor = YELLOW;   // or WHITE/GOLD if you prefer
}

DrawTextEx(myFont, winnerText,
    (Vector2){SCREEN_WIDTH/2 - MeasureTextEx(myFont, winnerText, 70, 5).x/2, 150},
    70, 5, winnerColor);

        int startX = SCREEN_WIDTH/2 - (3 * starTexture.width)/2;
        for(int i=0;i<3;i++) DrawTexture(starTexture, startX + i*starTexture.width, 180, WHITE);

        for(int i=0;i<3;i++){
            DrawTexture(whiteBoxTexture, victoryMenuButtons[i].rect.x, victoryMenuButtons[i].rect.y, WHITE);
            Color textColor = victoryMenuButtons[i].isHovered ? YELLOW : WHITE;
            DrawTextEx(myFont, victoryMenuButtons[i].label,
                (Vector2){victoryMenuButtons[i].rect.x + 20, victoryMenuButtons[i].rect.y + 10},50,2,textColor);
        }
    }

    EndDrawing();
}
    int main(void){
    // ------------------- Window & Audio Setup -------------------
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Whac-A-Mole Multiplayer");
    InitAudioDevice();
    SetTargetFPS(60);
    srand(time(NULL));

    // ------------------- Load Assets -------------------
    LoadAssets(); // backgrounds, moles, hammers, sounds, font, star

    // Box textures already loaded in LoadAssets()
    // whiteBoxTexture, greenBoxTexture, redBoxTexture, blueBoxTexture

    // ------------------- Initialize Game -------------------
    InitGame();

    bool gamePaused = false;

    // ------------------- Main Loop -------------------
    while(!WindowShouldClose()){
        float deltaTime = GetFrameTime();
        UpdateMusicStream(bgm);
        Vector2 mousePos = GetMousePosition();

        // ------------------- Update Game State -------------------
        UpdateGameState(deltaTime, mousePos, &gamePaused);

        // ------------------- Draw -------------------
        DrawGame(mousePos);
    }

    // ------------------- Cleanup -------------------
    UnloadTexture(backgroundMenu);
    UnloadTexture(backgroundGame);
    UnloadTexture(moleNormal.normal);
    UnloadTexture(moleNormal.hit);
    UnloadTexture(moleGolden.normal);
    UnloadTexture(moleGolden.hit);
    UnloadTexture(moleBomber.normal);
    UnloadTexture(moleBomber.hit);
    UnloadTexture(hammerRed.normal);
    UnloadTexture(hammerRed.hit);
    UnloadTexture(hammerBlue.normal);
    UnloadTexture(hammerBlue.hit);
    UnloadTexture(starTexture);

    // Box textures
    UnloadTexture(whiteBoxTexture);
    UnloadTexture(greenBoxTexture);
    UnloadTexture(redBoxTexture);
    UnloadTexture(blueBoxTexture);

    // Sounds
    UnloadSound(sndMolePop);
    UnloadSound(sndHitNormal);
    UnloadSound(sndHitGolden);
    UnloadSound(sndHitBomber);
    UnloadSound(sndHitEmpty);
    UnloadSound(sndButton);
    UnloadSound(sndVictory);

    UnloadMusicStream(bgm);
    UnloadFont(myFont);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
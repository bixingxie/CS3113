#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include <vector>
#include "Matrix.h"
#include "ShaderProgram.h"
#include "FlareMap.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

// 60 FPS (1.0f/60.0f) (update sixty times a second)
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

SDL_Window* displayWindow;

GLuint SPRITE_COUNT_X = 16;
GLuint SPRITE_COUNT_Y = 8;
float TILE_SIZE = 1/8.0;

enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER, STATE_WIN};

GLuint LoadTexture(const char* filepath){
    int w,h,comp;
    unsigned char* image = stbi_load(filepath, &w, &h, &comp, STBI_rgb_alpha);
    
    if(image == NULL){
        std::cout << "Unable to load image. Make sure the path is corret\n";
        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    stbi_image_free(image);
    return retTexture;
}

class SheetSprite{
public:
    SheetSprite(){};
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
                size):textureID(textureID), u(u), v(v), width(width), height(height), size(size){ }
    
    void Draw(ShaderProgram *program) const {
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        GLfloat texCoords[] = {
            u, v+height,
            u+width, v,
            u, v,
            u+width, v,
            u, v+height,
            u+width, v+height
        };
        
        
        //        float aspect = width / height;
        //        float vertices[] = {
        //            -0.5f * size * aspect, -0.5f * size,
        //            0.5f * size * aspect, 0.5f * size,
        //            -0.5f * size * aspect, 0.5f * size,
        //            0.5f * size * aspect, 0.5f * size,
        //            -0.5f * size * aspect, -0.5f * size ,
        //            0.5f * size * aspect, -0.5f * size};
        
        //        float aspect = width / height;
        
        float vertices[] = {
            -0.5f , -0.5f,
            0.5f, 0.5f ,
            -0.5f , 0.5f ,
            0.5f, 0.5f ,
            -0.5f, -0.5f  ,
            0.5f , -0.5f };
        
        glUseProgram(program->programID);
        
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
    }
    
    
    
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
    float size;
};

class Vector3{
public:
    Vector3():x(0.0f), y(0.0f), z(0.0f){};
    Vector3(float x, float y, float z):x(x), y(y), z(z){};
    
    float x;
    float y;
    float z;
};

float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_COIN};

class Entity{
public:
    
    Entity(){};
    
    Entity(const SheetSprite& sprite, float positionX, float positionY, float sizeX, float sizeY, float velocityX, float velocityY, float accelerationX, float accelerationY, EntityType entityType):sprite(sprite), position(positionX, positionY, 0.0f), size(sizeX*sprite.size*sprite.width/sprite.height, sizeY*sprite.size, 0.0f), velocity(velocityX, velocityY, 0.0f), acceleration(accelerationX, accelerationY, 0.0f), entityType(entityType){};
    
    
    void UpdateX(float elapsed){
        
        if(entityType == ENTITY_PLAYER){
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            
            if(keys[SDL_SCANCODE_RIGHT]){
                acceleration.x = 2.5f;
            }else if(keys[SDL_SCANCODE_LEFT]){
                acceleration.x = -2.5f;
            }else{
                acceleration.x = 0.0f;
            }
            
            velocity.x = lerp(velocity.x, 0.0f, elapsed * 1.5f);
            velocity.x += acceleration.x * elapsed;
            position.x += velocity.x * elapsed;
            
        }
    }
    
    void UpdateY(float elapsed){
        if(entityType == ENTITY_PLAYER){
            acceleration.y = -2.0f;
            
            velocity.y += acceleration.y * elapsed;
            position.y += velocity.y * elapsed;
            
            if(position.y <= -2.0f+size.y*0.5){
                position.y = -2.0f+size.y*0.5;
                collideBottom = true;
            }
            
        }
    }
    
    void Render(ShaderProgram* program, Entity* player){
        
        Matrix projectionMatrix;
        projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        Matrix modelMatrix;
        modelMatrix.Translate(position.x, position.y, position.z);
        modelMatrix.Scale(size.x, size.y, size.z);
        
        Matrix viewMatrix;
        viewMatrix.Translate(-1.0f*player->position.x, -1.0f*player->position.y, 0.0f);
        
        glUseProgram(program->programID);
        
        program->SetProjectionMatrix(projectionMatrix);
        program->SetModelMatrix(modelMatrix);
        program->SetViewMatrix(viewMatrix);
        
        sprite.Draw(program);
    }
    
    
    SheetSprite sprite;
    
    Vector3 position;
    Vector3 size;
    Vector3 velocity;
    Vector3 acceleration;
    
    EntityType entityType;
    
    bool collideTop;
    bool collideBottom;
    bool collideLeft;
    bool collideRight;
    
};


class gameState{
public:
    
    gameState(){}
    
    Entity player;
};

void setup(ShaderProgram* program){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 1280, 720);
    //    glClearColor(254.0f/255.0f, 223.0f/255.0f, 225.0f/255.0f, 1.0f);
    
    program->Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void processGameInput(SDL_Event* event, bool& done, gameState* gameState){
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }else if(event->type == SDL_KEYDOWN){
            if(event->key.keysym.scancode == SDL_SCANCODE_SPACE && gameState->player.collideBottom == true){
                gameState->player.velocity.y = 2.0f;
            }
        }
    }
}

void updateGame(float elapsed, gameState* gameState){
    gameState->player.collideBottom = false;
    gameState->player.collideTop = false;
    gameState->player.collideRight = false;
    gameState->player.collideLeft = false;
    
    gameState->player.UpdateY(elapsed);
    //    player->CollidesWithY(coin);

    gameState->player.UpdateX(elapsed);
    //    player->CollidesWithX(coin);
    
}


void renderMap(ShaderProgram* program, FlareMap* map){
    
    Matrix projectionMatrix;
    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    Matrix modelMatrix;
    Matrix viewMatrix;
    
    glUseProgram(program->programID);
    
    
    program->SetModelMatrix(modelMatrix);
//    program->SetViewMatrix(viewMatrix);
    program->SetProjectionMatrix(projectionMatrix);
    
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    
    for(int y=0; y < map->mapHeight; y++) {
        for(int x=0; x < map->mapWidth; x++) {
            
            float u = (float)(((int)map->mapData[y][x]) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;
            float v = (float)(((int)map->mapData[y][x]) / SPRITE_COUNT_X) / (float) SPRITE_COUNT_Y;
            
            float spriteWidth = 1.0f/(float)SPRITE_COUNT_X;
            float spriteHeight = 1.0f/(float)SPRITE_COUNT_Y;
            
//            if ((int)map->mapData[y][x] == 0)
//            {
//                spriteWidth = 0.0f;
//                spriteHeight = 0.0f;
//            }
            
            

            vertexData.insert(vertexData.end(), {
                TILE_SIZE * x, -TILE_SIZE * y,
                TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
                (TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
                TILE_SIZE * x, -TILE_SIZE * y,
                (TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
                (TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
            });
            

            texCoordData.insert(texCoordData.end(), {
                u, v,
                u, v + (spriteHeight),
                u + spriteWidth, v + (spriteHeight),
                u, v,
                u + spriteWidth, v + (spriteHeight),
                u + spriteWidth, v
            });
            
        }
    }
    
    
    glUseProgram(program->programID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    
    glBindTexture(GL_TEXTURE_2D, map->textureID);
    glDrawArrays(GL_TRIANGLES, 0, vertexData.size()/2);
    
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
    
    
}

void placeEntity(std::string type, float x, float y, gameState* state){
    if(type == "Player"){
        state->player.position.x = x * TILE_SIZE;
        state->player.position.y = y * -TILE_SIZE;
    }
}

void renderGame(ShaderProgram* program, gameState* gameState, FlareMap* map){
    glClear(GL_COLOR_BUFFER_BIT);
    
    renderMap(program, map);
    
    gameState->player.Render(program, &gameState->player);

    SDL_GL_SwapWindow(displayWindow);
}

void cleanup(){
    SDL_Quit();
}

int main(int argc, char *argv[]){

    ShaderProgram program;
    float lastFrameTicks = 0.0f;
    float accumulator = 0.0f;
    
    setup(&program);
    
    gameState gameState;
    
    GLuint mapTexture = LoadTexture(RESOURCE_FOLDER"arne_sprites.png");
    
    
    GLuint spaceSpriteSheet = LoadTexture(RESOURCE_FOLDER"sheet.png");
    gameState.player.sprite = SheetSprite(spaceSpriteSheet, 247.0f/1024.0f, 84.0f/1024.0f, 99.0f/1024.0f, 75.0f/1024.0f, 0.2);
    gameState.player = Entity(gameState.player.sprite, 0.05, 0.05f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, -2.0f, ENTITY_PLAYER);
    
    placeEntity("Player", 6.0f, 24.0f, &gameState);
    
    FlareMap map;
    map.textureID = mapTexture;
    map.Load(RESOURCE_FOLDER"map.txt");
    
    SDL_Event event;
    bool done = false;
    
    while (!done) {
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue; }
        
        processGameInput(&event, done, &gameState);
        
        while(elapsed >= FIXED_TIMESTEP) {
            updateGame(FIXED_TIMESTEP, &gameState);
            elapsed -= FIXED_TIMESTEP;
        }
        
        accumulator = elapsed;
        renderGame(&program, &gameState, &map);
        
    
    }
    
    
    cleanup();
    return 0;
}



#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include <vector>
#include "Matrix.h"
#include "ShaderProgram.h"

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
        
        
        float aspect = width / height;
        float vertices[] = {
            -0.5f * size * aspect, -0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, 0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, -0.5f * size ,
            0.5f * size * aspect, -0.5f * size};
        
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
    Vector3(float x, float y, float z):x(x), y(y), z(z){};
    
    float x;
    float y;
    float z;
};

float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_COIN, ENTITY_STATIC};

class Entity{
public:
    
    Entity(float positionX, float positionY, float sizeX, float sizeY, float velocityX, float velocityY, float accelerationX, float accelerationY, EntityType entityType): position(positionX, positionY, 0.0f), size(sizeX, sizeY, 0.0f), velocity(velocityX, velocityY, 0.0f), acceleration(accelerationX, accelerationY, 0.0f), sprite(), entityType(entityType){};
    
    void Update(float elapsed){
        if(entityType == ENTITY_PLAYER){
        
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            
            if(keys[SDL_SCANCODE_RIGHT]){
                acceleration.x = 2.5f;
            }else if(keys[SDL_SCANCODE_LEFT]){
                acceleration.x = -2.5f;
            }else{
                acceleration.x = 0.0f;
            }
            
//            acceleration.y = -2.0f;
            
            //friction.x = 1.5f
            //friction.y = 1.5f
            velocity.x = lerp(velocity.x, 0.0f, elapsed * 1.5f);
//            velocity.y = lerp(velocity.y, 0.0f, elapsed * 1.5f);
            velocity.x += acceleration.x * elapsed;
            velocity.y += acceleration.y * elapsed;
            position.x += velocity.x * elapsed;
            position.y += velocity.y * elapsed;
            
            if(position.x >= 3.55f-size.x*0.2/3*2){
                position.x = 3.55f-size.x*0.2/3*2;
            }else if(position.x <= -3.55f+size.x*0.2/3*2){
                position.x = -3.55f+size.x*0.2/3*2;
            }
            
            
            if(position.y <= -2.0f+size.y*0.5*0.2){
                position.y = -2.0f+size.y*0.5*0.2;
            }else if(position.y >= 2.0f-size.y*0.5*0.2){
                position.y = 2.0f-size.y*0.5*0.2;
            }
            
        }
    }
    void Render(ShaderProgram* program){
        Matrix projectionMatrix;
        projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        Matrix modelMatrix;
        modelMatrix.Translate(position.x, position.y, position.z);
        modelMatrix.Scale(size.x, size.y, size.z);
        
        Matrix viewMatrix;
        
        glUseProgram(program->programID);
        
        program->SetProjectionMatrix(projectionMatrix);
        program->SetModelMatrix(modelMatrix);
        program->SetViewMatrix(viewMatrix);
        
        sprite.Draw(program);
    }
    bool CollidesWith(Entity* entity){

        if(position.x+size.x*0.2/3*2 < entity->position.x-entity->size.x*0.2*0.5 || position.x-size.x*0.2/3*2 > entity->position.x+entity->size.x*0.2*0.5|| position.y+size.y*0.2*0.5 < entity->position.y-entity->size.y*0.2*0.5 || position.y-size.y*0.2*0.5 > entity->position.y+entity->size.y*0.2*0.5){
            return false;
        }else{
            if(entity->entityType == ENTITY_COIN){
                entity->position.x = -2000.0f;
            }else if(entity->entityType == ENTITY_STATIC){
                double Ypenetration = 0.0f;
                double Xpenetration = 0.0f;
                
                
                if(position.y>entity->position.y){
                    Ypenetration = fabs((position.y-entity->position.y)-size.y*0.5*0.2-entity->size.y*0.5*0.2);
                    position.y = position.y + Ypenetration + 0.00001f;
                }else{
                    Ypenetration = fabs((entity->position.y-position.y)-size.y*0.5*0.2-entity->size.y*0.5*0.2);
                    position.y = position.y - Ypenetration + 0.00001f;
                }
                
                if(position.x>entity->position.x){
                    Xpenetration = fabs((position.x-entity->position.x)-size.x*0.2/3*2-entity->size.x*0.2*0.5);
                    position.x = position.x + Xpenetration + 0.00001f;
                }else{
                    
                    Xpenetration = fabs((entity->position.x-position.x)-size.x*0.2/3*2-entity->size.x*0.2*0.5);
                    position.x = position.x - Xpenetration + 0.00001f;
                }
//                std::cout << Ypenetration << std::endl;
                std::cout << Xpenetration << std::endl;
                velocity.y = 0.0f;
//                velocity.x = 0.0f;
            }
            return true;
        }
    }
    
    
    SheetSprite sprite;
    
    Vector3 position;
    Vector3 size;
    Vector3 velocity;
    Vector3 acceleration;
    
    bool isStatic;
    
    EntityType entityType;
    
    bool collideTop;
    bool collideBottom;
    bool collideLeft;
    bool collideRight;
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

void processGameInput(SDL_Event* event, bool& done, Entity* player){
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }else if(event->type == SDL_KEYDOWN){
            if(event->key.keysym.scancode == SDL_SCANCODE_SPACE){
                player->velocity.y = 2.0f;
            }
        }
    }
}

void updateGame(float elapsed, Entity* player, std::vector<Entity*> woods, Entity* coin ){
    player->Update(elapsed);
    for (Entity* woodPtr : woods){
        player->CollidesWith(woodPtr);
    }
    player->CollidesWith(coin);
    std::cout << coin << std::endl;
}

void renderGame(ShaderProgram* program, Entity* player, std::vector<Entity*> woods, Entity* coin){
    player->Render(program);
    for (Entity* woodPtr : woods){
        woodPtr->Render(program);
    }

    coin->Render(program);
}

void cleanup(){
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    ShaderProgram program;
    float lastFrameTicks = 0.0f;
    float accumulator = 0.0f;
    
    setup(&program);
    
    Entity player(-3.35f, -1.0f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, -2.0f, ENTITY_PLAYER);
    Entity coin(2.5f, 1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_COIN);
    
    GLuint spaceSpriteSheet = LoadTexture(RESOURCE_FOLDER"sheet.png");
    GLuint itemSpriteSheet = LoadTexture(RESOURCE_FOLDER"items_spritesheet.png");
    GLuint woodSpriteSheet = LoadTexture(RESOURCE_FOLDER"spritesheet_wood.png");
//    GLuint alienPinkSpriteSheet = LoadTexture(RESOURCE_FOLDER"alienPink.png");
    
    player.sprite = SheetSprite(spaceSpriteSheet, 247.0f/1024.0f, 84.0f/1024.0f, 99.0f/1024.0f, 75.0f/1024.0f, 0.2);
    coin.sprite = SheetSprite(itemSpriteSheet, 288.0f/1024.0f, 432.0f/1024.0f, 70.0f/1024.0f, 70.0f/1024.0f, 0.2);
//    player.sprite = SheetSprite(alienPinkSpriteSheet, 69.0f/512.0f, 379.0/512.0f, 66.0f/512.0f, 92.0f/512.0f, 0.2);
    
    float posX = -1.5f;
    float posY = -1.8f;
    
    std::vector<Entity*> woods;
    
    for (size_t i=0; i<5; i++){
        Entity* newWoodPtr = new Entity(posX, posY, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
        newWoodPtr->sprite = SheetSprite(woodSpriteSheet, 0.0f/1024.0f, 630.0f/1024.0f, 220.0f/1024.0f, 140.0f/1024.0f, 0.2);
        woods.push_back(newWoodPtr);
        posX += 0.8f;
        posY += 0.6f;
    }
    
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
        
        processGameInput(&event, done, &player);
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        while(elapsed >= FIXED_TIMESTEP) {
            updateGame(FIXED_TIMESTEP, &player, woods, &coin);
            elapsed -= FIXED_TIMESTEP;
        }
        
        accumulator = elapsed;
        renderGame(&program, &player, woods, &coin);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    cleanup();
    return 0;
}

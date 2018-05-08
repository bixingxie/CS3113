#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "SatCollision.h"
#include "Vector3.h"


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


//class Vector3{
//public:
//    Vector3(float x, float y, float z):x(x), y(y), z(z){};
//
//    float length(){ return sqrt(x*x+y*y+z*z);}
//
//    void normalize(){
//        float len = sqrt(x*x+y*y+z*z);
//
//        if(len!=0.0f){
//            x = x/len;
//            y = y/len;
//            z = z/len;
//        }
//    };
//
////    Vector3 operator* (const Matrix& rhs){
////        Vector3 newVec = Vector3(x, y, z);
////
////        newVec.x = rhs.m[0][0] * x + rhs.m[1][0] * y + rhs.m[2][0] * z;
////        newVec.y = rhs.m[0][1] * x + rhs.m[1][1] * y + rhs.m[2][1] * z;
////        newVec.z = rhs.m[0][2] * x + rhs.m[1][2] * y + rhs.m[2][2] * z;
////
////        return newVec;
////    }
//
//    float x;
//    float y;
//    float z;
//};



float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_COIN, ENTITY_STATIC, ENTITY_PLAYER2};

class Entity{
public:
    
    Entity(const SheetSprite& sprite, float positionX, float positionY, float sizeX, float sizeY, float velocityX, float velocityY, float accelerationX, float accelerationY, EntityType entityType):sprite(sprite), position(positionX, positionY, 0.0f), size(sizeX*sprite.size*sprite.width/sprite.height, sizeY*sprite.size, 0.0f), velocity(velocityX, velocityY, 0.0f), acceleration(accelerationX, accelerationY, 0.0f), entityType(entityType){
        
        
        
    };
    
    
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
            
        }else if(entityType == ENTITY_PLAYER2){
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            
            if(keys[SDL_SCANCODE_D]){
                acceleration.x = 2.5f;
            }else if(keys[SDL_SCANCODE_A]){
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
        if(entityType == ENTITY_PLAYER || entityType == ENTITY_PLAYER2){
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
        matrix.Identity();
        matrix.Translate(position.x, position.y, position.z);
        matrix.Scale(size.x, size.y, size.z);
        
        if(entityType == ENTITY_PLAYER){
            matrix.Scale(1.3f, 1.3f, 1.0f);
        }else if(entityType == ENTITY_STATIC){
            matrix.Scale(1.2f, 1.2f, 1.0f);
        }else if(entityType == ENTITY_COIN){
        }
        
        
        if(entityType == ENTITY_PLAYER){
            matrix.Rotate(0.8);
        }else if(entityType == ENTITY_STATIC){
            matrix.Rotate(1.5);
        }else if(entityType == ENTITY_COIN){
            matrix.Rotate(0.6);
        }else if(entityType == ENTITY_PLAYER2){
            matrix.Rotate(2.5);
        }
        
        Matrix viewMatrix;
        
//        viewMatrix.Translate(-1.0f*player->position.x, -1.0f*player->position.y, 0.0f);
        
        glUseProgram(program->programID);
        
        program->SetProjectionMatrix(projectionMatrix);
    
        program->SetModelMatrix(matrix);
        program->SetViewMatrix(viewMatrix);
        
//        matrix.display();
        
        sprite.Draw(program);
        
    }
    
    bool SatCollision(Entity* entity){
        
        matrix.Identity();
        matrix.Translate(position.x, position.y, position.z);
        matrix.Scale(size.x, size.y, size.z);
        if(entityType == ENTITY_PLAYER){
            matrix.Scale(1.3f, 1.3f, 1.0f);
        }else if(entityType == ENTITY_STATIC){
            matrix.Scale(1.2f, 1.2f, 1.0f);
        }else if(entityType == ENTITY_COIN){
        }
        if(entityType == ENTITY_PLAYER){
            matrix.Rotate(0.8);
        }else if(entityType == ENTITY_STATIC){
            matrix.Rotate(1.5);
        }else if(entityType == ENTITY_COIN){
            matrix.Rotate(0.6);
        }else if(entityType == ENTITY_PLAYER2){
            matrix.Rotate(2.5);
        }

        std::pair<float,float> penetration;
        std::vector<std::pair<float,float>> e1Points;
        std::vector<std::pair<float,float>> e2Points;
        
        std::vector<Vector3> points;
        points.push_back(Vector3(-0.5f, -0.5f, 0.0f));
        points.push_back(Vector3(0.5f, 0.5f, 0.0f));
        points.push_back(Vector3(-0.5f, 0.5f, 0.0f));
        points.push_back(Vector3(0.5f, -0.5f, 0.0f));
        

        for(int i=0; i < points.size(); i++) {
//            Vector3 point = points[i] * matrix;
            Vector3 point = matrix * points[i];
            
//            std::cout << "object A: point.x " << point.x << " point.y " << point.y << std::endl;
            e1Points.push_back(std::make_pair(point.x, point.y));
        }

        for(int i=0; i < points.size(); i++) {
//            Vector3 point = points[i] * entity->matrix;
            Vector3 point = entity->matrix * points[i];
//            std::cout << "object B: point.x " << point.x << " point.y " << point.y << std::endl;
            e2Points.push_back(std::make_pair(point.x, point.y));
        }
    
        bool collided = CheckSATCollision(e1Points, e2Points, penetration);
        
        if(collided){
            position.x += (penetration.first * 0.5f);
            position.y += (penetration.second * 0.5f);
//
            entity->position.x -= (penetration.first * 0.5f);
            entity->position.y -= (penetration.second * 0.5f);
            
//            position.x += penetration.first;
//            position.y += penetration.second;
//            velocity.x = 0.0f;
            velocity.y = 0.0f;
        }
        
        
//        std::cout << "penetration first: " << penetration.first << " penetration second: " << penetration.second << std::endl;
        
    
        return collided;
    }
    
    bool CollidesWithX(Entity* entity){
        
        if(position.x+size.x*0.5 < entity->position.x-entity->size.x*0.5 || position.x-size.x*0.5 > entity->position.x+entity->size.x*0.5|| position.y+size.y*0.5 < entity->position.y-entity->size.y*0.5 || position.y-size.y*0.5 > entity->position.y+entity->size.y*0.5){
            return false;
        }else{
            if(entity->entityType == ENTITY_COIN){
                entity->position.x = -2000.0f;
            }else if(entity->entityType == ENTITY_STATIC){
                double Xpenetration = 0.0f;
                
                Xpenetration = fabs(fabs(position.x-entity->position.x) - size.x*0.5 - entity->size.x*0.5);
                
                if(position.x>entity->position.x){
                    position.x = position.x + Xpenetration + 0.00001f;
                    collideLeft = true;
                }else{
                    position.x = position.x - Xpenetration - 0.000001f;
                    collideRight = true;
                }
                
                velocity.x = 0.0f;
            }
            return true;
        }
    }
    
    bool CollidesWithY(Entity* entity){
        
        if(position.x+size.x*0.5 < entity->position.x-entity->size.x*0.5 || position.x-size.x*0.5 > entity->position.x+entity->size.x*0.5|| position.y+size.y*0.5 < entity->position.y-entity->size.y*0.5 || position.y-size.y*0.5 > entity->position.y+entity->size.y*0.5){
            return false;
        }else{
            if(entity->entityType == ENTITY_COIN){
                entity->position.x = -2000.0f;
            }else if(entity->entityType == ENTITY_STATIC){
                double Ypenetration = 0.0f;
                
                
                Ypenetration = fabs(fabs(position.y-entity->position.y) - size.y*0.5 - entity->size.y*0.5);
                
                if(position.y>entity->position.y){
                    position.y = position.y + Ypenetration + 0.00001f;
                    collideBottom = true;
                }else{
                    position.y = position.y - Ypenetration - 0.00001f;
                    collideTop = true;
                }
                
                velocity.y = 0.0f;
                
            }
            return true;
        }
    }
    
    SheetSprite sprite;
    
    Vector3 position;
    Vector3 size;
    Vector3 velocity;
    Vector3 acceleration;
    Matrix matrix;
    
    //    bool isStatic;
    
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

void processGameInput(SDL_Event* event, bool& done, Entity* player, Entity* player2){
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }else if(event->type == SDL_KEYDOWN){
//            if(event->key.keysym.scancode == SDL_SCANCODE_SPACE && player->collideBottom == true){
            if(event->key.keysym.scancode == SDL_SCANCODE_SPACE){
                player->velocity.y = 2.0f;
            }
            else if(event->key.keysym.scancode == SDL_SCANCODE_J){
                player2->velocity.y = 2.0f;
            }
        }
    }
}

void updateGame(float elapsed, Entity* player, std::vector<Entity*> woods, Entity* coin, Entity* player2){
    player->collideBottom = false;
    player->collideTop = false;
    player->collideRight = false;
    player->collideLeft = false;
    
    player->UpdateY(elapsed);
    player2->UpdateY(elapsed);
//    for (Entity* woodPtr : woods){
//        player->CollidesWithY(woodPtr);
//    }
//
//    player->CollidesWithY(coin);
    
    
    player->UpdateX(elapsed);
    player2->UpdateX(elapsed);
//    for (Entity* woodPtr : woods){
//        player->CollidesWithX(woodPtr);
//    }
//    player->CollidesWithX(coin);

    for (Entity* woodPtr:woods){
        player->SatCollision(woodPtr);
    }
    player->SatCollision(coin);
    
    
    for (Entity* woodPtr:woods){
        player2->SatCollision(woodPtr);
    }
    player2->SatCollision(coin);
    
}

void renderGame(ShaderProgram* program, Entity* player, std::vector<Entity*> woods, Entity* coin, Entity* player2){
    player->Render(program, player);
    player2->Render(program, player2);
    for (Entity* woodPtr : woods){
        woodPtr->Render(program, player);
    }
    
    coin->Render(program, player);
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
    
    GLuint spaceSpriteSheet = LoadTexture(RESOURCE_FOLDER"sheet.png");
    GLuint itemSpriteSheet = LoadTexture(RESOURCE_FOLDER"items_spritesheet.png");
    GLuint woodSpriteSheet = LoadTexture(RESOURCE_FOLDER"spritesheet_wood.png");
    
    SheetSprite playerSheet = SheetSprite(spaceSpriteSheet, 247.0f/1024.0f, 84.0f/1024.0f, 99.0f/1024.0f, 75.0f/1024.0f, 0.2);
    SheetSprite itemSheet = SheetSprite(itemSpriteSheet, 288.0f/1024.0f, 432.0f/1024.0f, 70.0f/1024.0f, 70.0f/1024.0f, 0.2);
    SheetSprite woodSheet = SheetSprite(woodSpriteSheet, 0.0f/1024.0f, 630.0f/1024.0f, 220.0f/1024.0f, 140.0f/1024.0f, 0.2);
    
    
    Entity player(playerSheet, -3.35f, -1.0f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, -2.0f, ENTITY_PLAYER);
    Entity player2(playerSheet, 3.35f, 1.0f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, -2.0f, ENTITY_PLAYER2);
    Entity coin(itemSheet, 2.5f, 1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_COIN);
    
    float posX = -1.5f;
    float posY = -1.8f;
    
    std::vector<Entity*> woods;
    
    for (size_t i=0; i<5; i++){
        Entity* newWoodPtr = new Entity(woodSheet, posX, posY, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
        woods.push_back(newWoodPtr);
        posX += 0.8f;
        posY += 0.7f;
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
        
        processGameInput(&event, done, &player, &player2);
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        while(elapsed >= FIXED_TIMESTEP) {
            updateGame(FIXED_TIMESTEP, &player, woods, &coin, &player2);
            elapsed -= FIXED_TIMESTEP;
        }
        
        accumulator = elapsed;
        renderGame(&program, &player, woods, &coin, &player2);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    cleanup();
    return 0;
}


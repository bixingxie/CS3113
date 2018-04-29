#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include <vector>
#include "Matrix.h"
#include "Vector3.h"
#include "ShaderProgram.h"
#include "SheetSprite.h"
#include "Entity_untextured.h"

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


//Globals
SDL_Window* displayWindow;
bool gameOver = false;
enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL_1, STATE_GAME_LEVEL_2, STATE_GAME_LEVEL_3, STATE_GAME_OVER, STATE_WIN};
enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY_SNAIL, ENTITY_COIN, ENTITY_STATIC};


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


void DrawText(ShaderProgram *program, GLuint fontTexture, std::string text, float size, float spacing) {
    float texture_size = 1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for(int i=0; i < text.size(); i++) {
        int spriteIndex = (int)text[i];
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        }); }
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    glUseProgram(program->programID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    glDrawArrays(GL_TRIANGLES, 0, GLsizei(text.size()*6));
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
    
}

float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

class Entity{
public:
    
    Entity(const SheetSprite& sprite, float positionX, float positionY, float sizeX, float sizeY, float velocityX, float velocityY, float accelerationX, float accelerationY, EntityType entityType):sprite(sprite), position(positionX, positionY, 0.0f), size(sizeX*sprite.size*sprite.width/sprite.height, sizeY*sprite.size, 0.0f), velocity(velocityX, velocityY, 0.0f), acceleration(accelerationX, accelerationY, 0.0f), entityType(entityType){};
    
    
    void UpdateX(float elapsed){
        
        if(entityType == ENTITY_PLAYER){
            
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            
            if(gameOver == false){
                if(keys[SDL_SCANCODE_RIGHT]){
                    acceleration.x = 2.5f;
                }else if(keys[SDL_SCANCODE_LEFT]){
                    acceleration.x = -2.5f;
                }else{
                    acceleration.x = 0.0f;
                }
            }
                
            velocity.x = lerp(velocity.x, 0.0f, elapsed * 1.5f);
            velocity.x += acceleration.x * elapsed;
            position.x += velocity.x * elapsed;
            
        }if(entityType == ENTITY_ENEMY_SNAIL){
            if(collideTop){
                acceleration.x = 20.0f;
            }else{
                if(snailNum == 1){
                    if(position.x >= -1.5+2.5*0.5*0.2){
                        invertX = false;
                        acceleration.x = -1.0f;
                    }else if(position.x <= -1.5-2.5*0.5*0.2){
                        invertX = true;
                        acceleration.x = 1.0f;
                    }
                }else if(snailNum == 2){
                    if(position.x >= -1.5+0.8+2.5*0.5*0.2){
                        invertX = false;
                        acceleration.x = -1.0f;
                    }else if(position.x <= -1.5+0.8-2.5*0.5*0.2){
                        invertX = true;
                        acceleration.x = 1.0f;
                    }
                }else if(snailNum == 3){
                    if(position.x >= -1.5+2.4+2.5*0.5*0.2){
                        invertX = false;
                        acceleration.x = -1.0f;
                    }else if(position.x <= -1.5+2.4-2.5*0.5*0.2){
                        invertX = true;
                        acceleration.x = 1.0f;
                    }
                }
            }
            
            velocity.x = lerp(velocity.x, 0.0f, elapsed * 10.0f);
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
            
        }else if(entityType == ENTITY_ENEMY_SNAIL){
            if(collideTop){
                acceleration.y = -2.0f;
                
                velocity.y += acceleration.y * elapsed;
                position.y += velocity.y * elapsed;
                
                if(position.y <= -2.0f+size.y*0.5){
                    position.y = -2.0f+size.y*0.5;
                    collideBottom = true;
                }
            }
        }
    }
    
    
    void Render(ShaderProgram* program, Entity* player, float elapsed){
        
        Matrix projectionMatrix;
        projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        modelMatrix.Identity();
        modelMatrix.Translate(position.x, position.y, position.z);
        modelMatrix.Scale(size.x, size.y, size.z);
        
        Matrix viewMatrix;
        
//        viewMatrix.Translate(-1.0f*player->position.x, -1.0f*player->position.y, 0.0f);
        
        glUseProgram(program->programID);
        
        program->SetProjectionMatrix(projectionMatrix);
        program->SetModelMatrix(modelMatrix);
        program->SetViewMatrix(viewMatrix);
    
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        
        if(entityType == ENTITY_PLAYER){
            if(collideBottom == true){
                if(keys[SDL_SCANCODE_RIGHT]){
//                    drawUniformSpriteSheetSprite(program, 2, 7, 5, uniformSpriteSheetTextureID);
                    animate(program, elapsed);
                }else if(keys[SDL_SCANCODE_LEFT]){
                    modelMatrix.Scale(-1.0f, 1.0f, 1.0f);
                    program->SetModelMatrix(modelMatrix);
//                    drawUniformSpriteSheetSprite(program, 2, 7, 5, uniformSpriteSheetTextureID);
                    animate(program, elapsed);
                }else{
                   drawSingleSprite(program, frontTextureID);
                }
            }else{
                drawSingleSprite(program, jumpTextureID);
            }
        }else if(entityType == ENTITY_STATIC){
//            if(player->collideBottom == true && player->position.y > -2.0f+player->size.y*0.5){
            if(collideTop == true){
                drawSingleSprite(program, blockMadTextureID);
                collideTop = false;
            }else{
                drawSingleSprite(program, blockTextureID);
            }
        }else if (entityType == ENTITY_COIN){
            sprite.Draw(program);
        }else if (entityType == ENTITY_ENEMY_SNAIL){
            if(collideTop == true){
                drawSingleSprite(program, snailShell);
            }else{
                if(invertX){
                    modelMatrix.Scale(-1.0f, 1.0f, 1.0f);
                    program->SetModelMatrix(modelMatrix);
                }
                animate(program, elapsed);
            }
        }
    }
    
    void drawUniformSpriteSheetSprite(ShaderProgram* program, int index, int spriteCountX, int spriteCountY, GLuint textureID){
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        float u = (float)(((int)index) % spriteCountX) / (float) spriteCountX;
        float v = (float)(((int)index) / spriteCountX) / (float) spriteCountY;
        float spriteWidth = 1.0/(float)spriteCountX;
        float spriteHeight = 1.0/(float)spriteCountY;
        
        float texCoods[] = {
            u, v+spriteHeight,
            u+spriteWidth, v,
            u, v,
            u+spriteWidth, v,
            u, v+spriteHeight,
            u+spriteWidth, v+spriteHeight
        };
        
        float vertices[] = {-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f,  -0.5f,
            -0.5f, 0.5f, -0.5f};
        
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoods);
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
        
    }
    
    void drawSingleSprite(ShaderProgram* program, GLuint textureID){
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        float vertices01[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices01);
        glEnableVertexAttribArray(program->positionAttribute);
        
        float texCoords01[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords01);
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
    
    }
    
    void animate(ShaderProgram* program, float elapsed){
        if(entityType == ENTITY_PLAYER){
            const int runAnimation[] = {3,4,5,6,7};
            const int numFrames = 5;
            float framesPerSecond = 10.0f;
            
            animationElapsed += elapsed;
            
            if(animationElapsed >= 1.0/framesPerSecond){
                currentIndex++;
                animationElapsed = 0.0;
                
                if(currentIndex>numFrames-1){
                    currentIndex = 0;
                }
            }

    //        drawUniformSpriteSheetSprite(program, runAnimation[currentIndex], 7, 5, uniformSpriteSheetTextureID);
            if(runAnimation[currentIndex]==3){
                drawSingleSprite(program, walk03);
            }else if(runAnimation[currentIndex]==4){
                drawSingleSprite(program, walk04);
            }else if(runAnimation[currentIndex]==5){
                drawSingleSprite(program, walk05);
            }else if(runAnimation[currentIndex]==6){
                drawSingleSprite(program, walk06);
            }else{
                drawSingleSprite(program, walk07);
            }
        }else if(entityType == ENTITY_ENEMY_SNAIL){
            const int runAnimation[] = {1, 2};
            const int numFrames = 2;
            float framesPerSecond = 5.0f;
            
            animationElapsed += elapsed;
            
            if(animationElapsed >= 1.0/framesPerSecond){
                currentIndex++;
                animationElapsed = 0.0;
                
                if(currentIndex>numFrames-1){
                    currentIndex = 0;
                }
            }
            
            if(runAnimation[currentIndex]==1){
                drawSingleSprite(program, snailWalk01);
            }else if(runAnimation[currentIndex]==2){
                drawSingleSprite(program, snailWalk02);
            }
            

        }
    }
    
    
    bool CollidesWithX(Entity* entity){
        
        if(position.x+size.x*0.5 < entity->position.x-entity->size.x*0.5 || position.x-size.x*0.5 > entity->position.x+entity->size.x*0.5|| position.y+size.y*0.5 < entity->position.y-entity->size.y*0.5 || position.y-size.y*0.5 > entity->position.y+entity->size.y*0.5){
            return false;
        }else{
            if(entity->entityType == ENTITY_COIN){
                entity->position.x = -2000.0f;
                gameOver = true;
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
            }else if(entity->entityType == ENTITY_ENEMY_SNAIL){
                std::cout << "I'M DEADD???" << std::endl;
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
                gameOver = true;
            }else if(entity->entityType == ENTITY_STATIC){
                double Ypenetration = 0.0f;
                
                
                Ypenetration = fabs(fabs(position.y-entity->position.y) - size.y*0.5 - entity->size.y*0.5);
                
                if(position.y>entity->position.y){
                    position.y = position.y + Ypenetration + 0.00001f;
                    collideBottom = true;
                    entity->collideTop = true;
                }else{
                    position.y = position.y - Ypenetration - 0.00001f;
                    collideTop = true;
                }
                
                velocity.y = 0.0f;
                
            }else if(entity->entityType == ENTITY_ENEMY_SNAIL){
                if(position.y>entity->position.y){
                    collideBottom = true;
                    entity->collideTop = true;
                    std::cout << "THE SNAIL IS DEAD!!!" << std::endl;
                }else{
                    collideTop = true;
                    std::cout << "I'M DEADD???" << std::endl;
                }
                
            }
            return true;
        }
    }
    
    SheetSprite sprite;
    
    
    Vector3 position;
    Vector3 size;
    Vector3 velocity;
    Vector3 acceleration;
    
    Matrix modelMatrix;
    
    //    bool isStatic;
    
    float animationElapsed = 0.0f;
    int currentIndex = 0;
    
    EntityType entityType;
    GLuint frontTextureID;
    GLuint jumpTextureID;
    GLuint uniformSpriteSheetTextureID;
    GLuint blockTextureID;
    GLuint blockMadTextureID;
    GLuint walk03;
    GLuint walk04;
    GLuint walk05;
    GLuint walk06;
    GLuint walk07;
    
    GLuint snailWalk01;
    GLuint snailWalk02;
    GLuint snailShell;
    
    bool collideTop=false;
    bool collideBottom=false;
    bool collideLeft=false;
    bool collideRight=false;
    
    bool invertX = false;
    GLuint snailNum;
};

class mainMenuState{
public:
    mainMenuState():
    font1(SheetSprite(), -2.08, 0.3f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
    font2(SheetSprite(), -0.28f, 0.3f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
    font3(SheetSprite(), 1.57f, 0.3f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
    font4(SheetSprite(), -0.8f,-0.8f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
    board1(-1.9f, 0.3f, 1.3f, 0.5f),
    board2(-0.1f, 0.3f, 1.3f, 0.5f),
    board3(1.8f, 0.3f, 1.3f, 0.5f),
    board4(-0.1f, -0.8f, 1.7f, 0.55f){
        fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");
        mainBGTexture = LoadTexture(RESOURCE_FOLDER"mainBG.png");
        font1.size.x = 0.2f;
        font1.size.y = 0.2f;
        font2.size.x = 0.2f;
        font2.size.y = 0.2f;
        font3.size.x = 0.2f;
        font3.size.y = 0.2f;
        font4.size.x = 0.2f;
        font4.size.y = 0.2f;
        
    };
    
    
    GLuint fontTexture;
    GLuint mainBGTexture;
    
    Entity font1;
    Entity font2;
    Entity font3;
    Entity font4;
    Entity_untextured board1;
    Entity_untextured board2;
    Entity_untextured board3;
    Entity_untextured board4;
};

class gameState{
public:
    gameState():
    font1(SheetSprite(), -1.08-1.5f+0.3f+0.2f, 0.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
    font2(SheetSprite(), -1.08-1.5f+2.0f+0.1f+0.2f, 0.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
    player(playerSheet, -3.35f, -1.0f, 1.0f, 1.5f, 0.0f, 0.0f, 0.0f, -2.0f, ENTITY_PLAYER),
    coin(itemSheet, 2.5f, 1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_COIN),
    snail01(playerSheet, -1.5f, -1.5f+1.8*0.2*0.5-1.0*0.5*0.5, 1.5f/1.5, 1.0f/1.5, 0.0, 0.0f, -1.0f, -2.0f, ENTITY_ENEMY_SNAIL),
    snail02(playerSheet, -1.5f+0.9f, -1.5f+0.7f+1.8*0.2*0.5-1.0*0.5*0.5, 1.5f/1.5, 1.0f/1.5, 0.0, 0.0f, -1.0f, -2.0f, ENTITY_ENEMY_SNAIL),
    snail03(playerSheet, -1.5f+2.3f, -1.5f+2.1f+1.8*0.2*0.5-1.0*0.5*0.5, 1.5f/1.5, 1.0f/1.5, 0.0, 0.0f, -1.0f, -2.0f, ENTITY_ENEMY_SNAIL)
    {
        
        player.frontTextureID = p1_front;
        player.jumpTextureID = p1_jump;
        player.uniformSpriteSheetTextureID = p1_walk;
        player.walk03 = walk03;
        player.walk04 = walk04;
        player.walk05 = walk05;
        player.walk06 = walk06;
        player.walk07 = walk07;
        
        snail01.snailWalk01 = snailWalk01;
        snail01.snailWalk02 = snailWalk02;
        snail01.snailShell = snailShell;
        snail01.snailNum = 1;
        
        snail02.snailWalk01 = snailWalk01;
        snail02.snailWalk02 = snailWalk02;
        snail02.snailShell = snailShell;
        snail02.snailNum = 2;
        
        snail03.snailWalk01 = snailWalk01;
        snail03.snailWalk02 = snailWalk02;
        snail03.snailShell = snailShell;
        snail03.snailNum = 3;
        
        font1.size.x = 0.2f;
        font1.size.y = 0.2f;
        font2.size.x = 0.2f;
        font2.size.y = 0.2f;
        
        
        float posX = -1.5f;
        float posY = -1.8f;
        
        for (size_t i=0; i<5; i++){
            Entity* newWoodPtr = new Entity(woodSheet, posX, posY, 2.5f, 1.8f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            newWoodPtr->blockTextureID = blockTexture;
            newWoodPtr->blockMadTextureID = blockMadTexture;
            woods.push_back(newWoodPtr);
            posX += 0.8f;
            posY += 0.7f;
        }
    };
    
    GLuint fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");
    GLuint alienTexture = LoadTexture(RESOURCE_FOLDER"alienGreen.png");
    GLuint spaceSpriteSheet = LoadTexture(RESOURCE_FOLDER"spaceSpriteSheet.png");
    GLuint itemSpriteSheet = LoadTexture(RESOURCE_FOLDER"itemSpriteSheet.png");
    GLuint p1_front = LoadTexture(RESOURCE_FOLDER"p1_front.png");
    GLuint p1_jump = LoadTexture(RESOURCE_FOLDER"p1_jump.png");
    GLuint p1_walk = LoadTexture(RESOURCE_FOLDER"p1_spritesheet.png");
    GLuint woodSpriteSheet = LoadTexture(RESOURCE_FOLDER"woodSpriteSheet.png");
    GLuint blockTexture = LoadTexture(RESOURCE_FOLDER"blockerBody.png");
    GLuint blockMadTexture = LoadTexture(RESOURCE_FOLDER"blockerMad.png");
    
    GLuint walk03 = LoadTexture(RESOURCE_FOLDER"p1_walk03.png");
    GLuint walk04 = LoadTexture(RESOURCE_FOLDER"p1_walk04.png");
    GLuint walk05 = LoadTexture(RESOURCE_FOLDER"p1_walk05.png");
    GLuint walk06 = LoadTexture(RESOURCE_FOLDER"p1_walk06.png");
    GLuint walk07 = LoadTexture(RESOURCE_FOLDER"p1_walk07.png");
    
    GLuint level1BG = LoadTexture(RESOURCE_FOLDER"level1BG.png");
    GLuint level2BG = LoadTexture(RESOURCE_FOLDER"level2BG.png");
    GLuint level3BG = LoadTexture(RESOURCE_FOLDER"level3BG.png");
    
    GLuint snailWalk01 = LoadTexture(RESOURCE_FOLDER"snailWalk1.png");
    GLuint snailWalk02 = LoadTexture(RESOURCE_FOLDER"snailWalk2.png");
    GLuint snailShell = LoadTexture(RESOURCE_FOLDER"snailShell.png");
    

    SheetSprite woodSheet = SheetSprite(woodSpriteSheet, 0.0f/1024.0f, 630.0f/1024.0f, 220.0f/1024.0f, 140.0f/1024.0f, 0.2);
    SheetSprite playerSheet = SheetSprite(spaceSpriteSheet, 247.0f/1024.0f, 84.0f/1024.0f, 99.0f/1024.0f, 75.0f/1024.0f, 0.2);
    SheetSprite itemSheet = SheetSprite(itemSpriteSheet, 288.0f/1024.0f, 432.0f/1024.0f, 70.0f/1024.0f, 70.0f/1024.0f, 0.2);
    SheetSprite alienSheet = SheetSprite(alienTexture, 70.0f/512.0f, 92.0f/512.0f, 66.0f/512.0f, 92.0f/512.0f, 0.2);
    
    std::vector<Entity*> woods;
    Entity player;
    Entity coin;
    Entity snail01;
    Entity snail02;
    Entity snail03;
    Entity font1;
    Entity font2;
};

void drawBackground(ShaderProgram* program, GLuint textureID, gameState* gameState){
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    
    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    modelMatrix.Scale(3.55f*2, 2.0f*2, 1.0f);
    
    program->SetProjectionMatrix(projectionMatrix);
    program->SetModelMatrix(modelMatrix);
    program->SetViewMatrix(viewMatrix);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    float vertices01[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices01);
    glEnableVertexAttribArray(program->positionAttribute);
    
    float texCoords01[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords01);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    if(gameState){
        
        //didn't put viewMatrix because it was identity
        
        
        Matrix projModelMatrix = gameState->player.modelMatrix * projectionMatrix;
        
        Vector3 finalPos = projModelMatrix * gameState->player.position;
        
        std::cout << "x: " << finalPos.x << " y: " << finalPos.y << " z: " << finalPos.z << std::endl;
        program->SetPlayerPos(finalPos.x, finalPos.y, finalPos.z);
    }
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);

    program->SetPlayerPos(0, 0, 0);
    
}

void setup(ShaderProgram* program, ShaderProgram* program_untextured){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 1280, 720);
//    glClearColor(209.0f/255.0f, 244.0f/255.0f, 248.0f/255.0f, 1.0f);
    
    program->Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    program_untextured->Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void processGameInputLevel1(SDL_Event* event, bool& done, gameState* gameState){
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }else if(event->type == SDL_KEYDOWN){
            if(gameOver == false){
                if(event->key.keysym.scancode == SDL_SCANCODE_SPACE && gameState->player.collideBottom == true){
                    gameState->player.velocity.y = 2.0f;
                }
            }
                
        }
    }
}

void processGameInputLevel2(SDL_Event* event, bool& done, gameState* gameState){
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

void processGameInputLevel3(SDL_Event* event, bool& done, gameState* gameState){
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

void ProcessMainMenuInput(SDL_Event* event, bool& done, mainMenuState* menuState, GameMode& gameMode){
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }else if(event->type == SDL_MOUSEBUTTONDOWN){
            if(event->button.button == 1){
                float unitX = (((float)event->button.x / 1280.0f) * 3.554f ) - 1.777f;
                float unitY = (((float)(720-event->button.y) / 720.0f) * 2.0f ) - 1.0f;
                
                
                float board1PosX = menuState->board1.position.x * 0.5;
                float board1PosY = menuState->board1.position.y * 0.5;
                float board1SizeX = menuState->board1.size.x * 0.5;
                float board1SizeY = menuState->board1.size.y * 0.5;

                
                if(unitX>=board1PosX-board1SizeX/2 && unitX<=board1PosX+board1SizeX/2
                   && unitY>=board1PosY-board1SizeY/2 && unitY<=board1PosY+board1SizeY/2){
                    gameMode = STATE_GAME_LEVEL_1;
                }
                
                float board2PosX = menuState->board2.position.x * 0.5;
                float board2PosY = menuState->board2.position.y * 0.5;
                float board2SizeX = menuState->board2.size.x * 0.5;
                float board2SizeY = menuState->board2.size.y * 0.5;

                if(unitX>=board2PosX-board2SizeX/2 && unitX<=board2PosX+board2SizeX/2
                   && unitY>=board2PosY-board2SizeY/2 && unitY<=board2PosY+board2SizeY/2){
                    gameMode = STATE_GAME_LEVEL_2;
                }
                
                float board3PosX = menuState->board3.position.x * 0.5;
                float board3PosY = menuState->board3.position.y * 0.5;
                float board3SizeX = menuState->board3.size.x * 0.5;
                float board3SizeY = menuState->board3.size.y * 0.5;
                
                if(unitX>=board3PosX-board3SizeX/2 && unitX<=board3PosX+board3SizeX/2
                   && unitY>=board3PosY-board3SizeY/2 && unitY<=board3PosY+board3SizeY/2){
                    gameMode = STATE_GAME_LEVEL_3;
                }
                
                float board4PosX = menuState->board4.position.x * 0.5;
                float board4PosY = menuState->board4.position.y * 0.5;
                float board4SizeX = menuState->board4.size.x * 0.5;
                float board4SizeY = menuState->board4.size.y * 0.5;
                
                if(unitX>=board4PosX-board4SizeX/2 && unitX<=board4PosX+board4SizeX/2
                   && unitY>=board4PosY-board4SizeY/2 && unitY<=board4PosY+board4SizeY/2){
                    done = true;
                }
                
                
            }
        }
    }
}

void UpdateMainMenu(){}

void updateGameLevel1(float elapsed, gameState* gameState, GameMode* mode){
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if(keys[SDL_SCANCODE_ESCAPE]){
        *mode = STATE_MAIN_MENU;
    }

    gameState->player.collideBottom = false;
    gameState->player.collideTop = false;
    gameState->player.collideRight = false;
    gameState->player.collideLeft = false;
    
    gameState->player.UpdateY(elapsed);
    gameState->snail01.UpdateY(elapsed);
    gameState->snail02.UpdateY(elapsed);
    gameState->snail03.UpdateY(elapsed);
    for (Entity* woodPtr : gameState->woods){
        gameState->player.CollidesWithY(woodPtr);
    }
    gameState->player.CollidesWithY(&gameState->coin);
    gameState->player.CollidesWithY(&gameState->snail01);
    gameState->player.CollidesWithY(&gameState->snail02);
    gameState->player.CollidesWithY(&gameState->snail03);
    
    
    gameState->player.UpdateX(elapsed);
    gameState->snail01.UpdateX(elapsed);
    gameState->snail02.UpdateX(elapsed);
    gameState->snail03.UpdateX(elapsed);
    
    for (Entity* woodPtr : gameState->woods){
        gameState->player.CollidesWithX(woodPtr);
    }
    gameState->player.CollidesWithX(&gameState->coin);
    gameState->player.CollidesWithX(&gameState->snail01);
    gameState->player.CollidesWithX(&gameState->snail02);
    gameState->player.CollidesWithX(&gameState->snail03);
    
}

void updateGameLevel2(float elapsed, gameState* gameState, GameMode* mode){
    
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if(keys[SDL_SCANCODE_ESCAPE]){
        *mode = STATE_MAIN_MENU;
    }
    
    gameState->player.collideBottom = false;
    gameState->player.collideTop = false;
    gameState->player.collideRight = false;
    gameState->player.collideLeft = false;
    
    gameState->player.UpdateY(elapsed);
    gameState->snail01.UpdateY(elapsed);
    gameState->snail02.UpdateY(elapsed);
    gameState->snail03.UpdateY(elapsed);
    for (Entity* woodPtr : gameState->woods){
        gameState->player.CollidesWithY(woodPtr);
    }
    gameState->player.CollidesWithY(&gameState->coin);
    gameState->player.CollidesWithY(&gameState->snail01);
    gameState->player.CollidesWithY(&gameState->snail02);
    gameState->player.CollidesWithY(&gameState->snail03);
    
    
    gameState->player.UpdateX(elapsed);
    gameState->snail01.UpdateX(elapsed);
    gameState->snail02.UpdateX(elapsed);
    gameState->snail03.UpdateX(elapsed);
    
    for (Entity* woodPtr : gameState->woods){
        gameState->player.CollidesWithX(woodPtr);
    }
    gameState->player.CollidesWithX(&gameState->coin);
    gameState->player.CollidesWithX(&gameState->snail01);
    gameState->player.CollidesWithX(&gameState->snail02);
    gameState->player.CollidesWithX(&gameState->snail03);
    
}

void updateGameLevel3(float elapsed, gameState* gameState, GameMode* mode){
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if(keys[SDL_SCANCODE_ESCAPE]){
        *mode = STATE_MAIN_MENU;
    }
    
    gameState->player.collideBottom = false;
    gameState->player.collideTop = false;
    gameState->player.collideRight = false;
    gameState->player.collideLeft = false;
    
    gameState->player.UpdateY(elapsed);
    gameState->snail01.UpdateY(elapsed);
    gameState->snail02.UpdateY(elapsed);
    gameState->snail03.UpdateY(elapsed);
    for (Entity* woodPtr : gameState->woods){
        gameState->player.CollidesWithY(woodPtr);
    }
    gameState->player.CollidesWithY(&gameState->coin);
    gameState->player.CollidesWithY(&gameState->snail01);
    gameState->player.CollidesWithY(&gameState->snail02);
    gameState->player.CollidesWithY(&gameState->snail03);
    
    
    gameState->player.UpdateX(elapsed);
    gameState->snail01.UpdateX(elapsed);
    gameState->snail02.UpdateX(elapsed);
    gameState->snail03.UpdateX(elapsed);
    
    for (Entity* woodPtr : gameState->woods){
        gameState->player.CollidesWithX(woodPtr);
    }
    gameState->player.CollidesWithX(&gameState->coin);
    gameState->player.CollidesWithX(&gameState->snail01);
    gameState->player.CollidesWithX(&gameState->snail02);
    gameState->player.CollidesWithX(&gameState->snail03);
    
}

void RenderMainMenu(ShaderProgram* program, ShaderProgram* program_untextured, mainMenuState* menuState){
    
    //    glClearColor(209.0f/255.0f, 244.0f/255.0f, 248.0f/255.0f, 1.0f);
    
    drawBackground(program, menuState->mainBGTexture, NULL);
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    modelMatrix.Translate(menuState->font1.position.x, menuState->font1.position.y, menuState->font1.position.z);
    Matrix viewMatrix;
    
    program->SetProjectionMatrix(projectionMatrix);
    program->SetModelMatrix(modelMatrix);
    program->SetViewMatrix(viewMatrix);
    
    menuState->board1.Draw(program_untextured);
    menuState->board2.Draw(program_untextured);
    menuState->board3.Draw(program_untextured);
    menuState->board4.Draw(program_untextured);
    
    
    DrawText(program, menuState->fontTexture, "EASY", menuState->font1.size.x, -menuState->font1.size.x/2.5);
    modelMatrix.Identity();
    modelMatrix.Translate(menuState->font2.position.x, menuState->font2.position.y, menuState->font2.position.z);
    program->SetModelMatrix(modelMatrix);
    
    DrawText(program, menuState->fontTexture, "HARD", menuState->font2.size.x, -menuState->font2.size.x/2.5);
    modelMatrix.Identity();
    modelMatrix.Translate(menuState->font3.position.x, menuState->font3.position.y, menuState->font3.position.z);
    program->SetModelMatrix(modelMatrix);
    DrawText(program, menuState->fontTexture, "HELL?", menuState->font3.size.x, -menuState->font3.size.x/2.5);
    
    modelMatrix.Identity();
    modelMatrix.Translate(menuState->font4.position.x, menuState->font4.position.y, menuState->font4.position.z);
    program->SetModelMatrix(modelMatrix);
    DrawText(program, menuState->fontTexture, "DON'T EXIT...", menuState->font4.size.x, -menuState->font4.size.x/2.5);
    
    
}

void renderGameLevel1(ShaderProgram* program, gameState* gameState, float elapsed){
    drawBackground(program, gameState->level1BG, gameState);
//    glClearColor(135.0f/255.0f, 149.0f/255.0f, 150.0f/255.0f, 1.0f);
    gameState->player.Render(program, &gameState->player, elapsed);
    for (Entity* woodPtr : gameState->woods){
        woodPtr->Render(program, &gameState->player, elapsed);
    }
    
    gameState->snail01.Render(program, &gameState->player, elapsed);
    gameState->snail02.Render(program, &gameState->player, elapsed);
    gameState->snail03.Render(program, &gameState->player, elapsed);
   
    
    gameState->coin.Render(program, &gameState->player, elapsed);
    
    if(gameOver){
        Matrix projectionMatrix;
        Matrix modelMatrix;
        projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        modelMatrix.Translate(gameState->font1.position.x, gameState->font1.position.y, gameState->font1.position.z);
        Matrix viewMatrix;
        
        program->SetProjectionMatrix(projectionMatrix);
        program->SetModelMatrix(modelMatrix);
        program->SetViewMatrix(viewMatrix);
        
        DrawText(program, gameState->fontTexture, "YOU'VE WON!!!", gameState->font1.size.x, -gameState->font1.size.x/2.5);
        modelMatrix.Identity();
        modelMatrix.Translate(gameState->font2.position.x, gameState->font2.position.y, gameState->font2.position.z);
        program->SetModelMatrix(modelMatrix);
        
        DrawText(program, gameState->fontTexture, "PRESE ESC TO RETURN", gameState->font2.size.x, -gameState->font2.size.x/2.5);
    }
}

void renderGameLevel2(ShaderProgram* program, gameState* gameState, float elapsed){
    drawBackground(program, gameState->level2BG, gameState);
    //    glClearColor(135.0f/255.0f, 149.0f/255.0f, 150.0f/255.0f, 1.0f);
    gameState->player.Render(program, &gameState->player, elapsed);
    for (Entity* woodPtr : gameState->woods){
        woodPtr->Render(program, &gameState->player, elapsed);
    }
    
    gameState->snail01.Render(program, &gameState->player, elapsed);
    gameState->snail02.Render(program, &gameState->player, elapsed);
    gameState->snail03.Render(program, &gameState->player, elapsed);
    
    gameState->coin.Render(program, &gameState->player, elapsed);
    
    if(gameOver){
        Matrix projectionMatrix;
        Matrix modelMatrix;
        projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        modelMatrix.Translate(gameState->font1.position.x, gameState->font1.position.y, gameState->font1.position.z);
        Matrix viewMatrix;
        
        program->SetProjectionMatrix(projectionMatrix);
        program->SetModelMatrix(modelMatrix);
        program->SetViewMatrix(viewMatrix);
        
        DrawText(program, gameState->fontTexture, "YOU'VE WON!!!", gameState->font1.size.x, -gameState->font1.size.x/2.5);
        modelMatrix.Identity();
        modelMatrix.Translate(gameState->font2.position.x, gameState->font2.position.y, gameState->font2.position.z);
        program->SetModelMatrix(modelMatrix);
        
        DrawText(program, gameState->fontTexture, "PRESE ESC TO RETURN", gameState->font2.size.x, -gameState->font2.size.x/2.5);
    }
}

void renderGameLevel3(ShaderProgram* program, gameState* gameState, float elapsed){
    drawBackground(program, gameState->level3BG, gameState);
    //    glClearColor(135.0f/255.0f, 149.0f/255.0f, 150.0f/255.0f, 1.0f);
    gameState->player.Render(program, &gameState->player, elapsed);
    for (Entity* woodPtr : gameState->woods){
        woodPtr->Render(program, &gameState->player, elapsed);
    }
    
    gameState->snail01.Render(program, &gameState->player, elapsed);
    gameState->snail02.Render(program, &gameState->player, elapsed);
    gameState->snail03.Render(program, &gameState->player, elapsed);
    
    gameState->coin.Render(program, &gameState->player, elapsed);
    
    if(gameOver){
        Matrix projectionMatrix;
        Matrix modelMatrix;
        projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        modelMatrix.Translate(gameState->font1.position.x, gameState->font1.position.y, gameState->font1.position.z);
        Matrix viewMatrix;
        
        program->SetProjectionMatrix(projectionMatrix);
        program->SetModelMatrix(modelMatrix);
        program->SetViewMatrix(viewMatrix);
        
        DrawText(program, gameState->fontTexture, "YOU'VE WON!!!", gameState->font1.size.x, -gameState->font1.size.x/2.5);
        modelMatrix.Identity();
        modelMatrix.Translate(gameState->font2.position.x, gameState->font2.position.y, gameState->font2.position.z);
        program->SetModelMatrix(modelMatrix);
        
        DrawText(program, gameState->fontTexture, "PRESE ESC TO RETURN", gameState->font2.size.x, -gameState->font2.size.x/2.5);
    }
}

void cleanup(){
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    ShaderProgram program;
    ShaderProgram program_untextured;
    float lastFrameTicks = 0.0f;
    float accumulator = 0.0f;
    
    setup(&program, &program_untextured);
    

    mainMenuState menuState;
    gameState gameState1;
    gameState gameState2;
    gameState gameState3;
    GameMode mode = STATE_MAIN_MENU;
    
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
        
        if(mode == STATE_MAIN_MENU){
            gameOver = false;
            ProcessMainMenuInput(&event, done, &menuState, mode);
        }else if(mode == STATE_GAME_LEVEL_1){
            processGameInputLevel1(&event, done, &gameState1);
        }else if(mode == STATE_GAME_LEVEL_2){
            processGameInputLevel2(&event, done, &gameState2);
        }else if(mode == STATE_GAME_LEVEL_3){
            processGameInputLevel3(&event, done, &gameState3);
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        while(elapsed >= FIXED_TIMESTEP) {
            if(mode == STATE_MAIN_MENU){
                UpdateMainMenu();
            }else if(mode == STATE_GAME_LEVEL_1){
                updateGameLevel1(FIXED_TIMESTEP, &gameState1, &mode);
            }else if(mode == STATE_GAME_LEVEL_2){
                updateGameLevel2(FIXED_TIMESTEP, &gameState2, &mode);
            }else if(mode == STATE_GAME_LEVEL_3){
                updateGameLevel3(FIXED_TIMESTEP, &gameState3, &mode);
            }
            
            elapsed -= FIXED_TIMESTEP;
        }
        
        accumulator = elapsed;
        if(mode == STATE_MAIN_MENU){
            RenderMainMenu(&program, &program_untextured, &menuState);
        }else if(mode == STATE_GAME_LEVEL_1){
            renderGameLevel1(&program, &gameState1, FIXED_TIMESTEP);
        }else if(mode == STATE_GAME_LEVEL_2){
            renderGameLevel2(&program, &gameState2, FIXED_TIMESTEP);
        }else if(mode == STATE_GAME_LEVEL_3){
            renderGameLevel3(&program, &gameState3, FIXED_TIMESTEP);
        }
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    cleanup();
    return 0;
}

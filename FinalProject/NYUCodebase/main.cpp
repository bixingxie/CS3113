#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include "Entity.h"

#include <vector>
#include "Matrix.h"
#include "ShaderProgram.h"
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


float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

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

class Particle {
public:
    Particle(float x, float y):position(x, y, 0.0f), velocity(0.0f, 0.2f, 0.0f), size(0.4f, 0.4f, 1.0f){}

    Vector3 position;
    Vector3 velocity;
    Vector3 size;
    float lifetime;
};

class ParticleEmitter{
public:
    ParticleEmitter(unsigned int particleCount, float x, float y):
    position(x, y, 0.0f), gravity(-0.02f), maxLifetime(1.5f){
        for(int i=0; i < particleCount; i++){
            Particle newParticle(position.x, position.y);
            newParticle.lifetime = ((float)rand()/(float)RAND_MAX) * maxLifetime;
            newParticle.size.x *= ((float)rand()/(float)RAND_MAX);
            newParticle.size.y *= ((float)rand()/(float)RAND_MAX);
            newParticle.velocity.y *= ((float)rand()/(float)RAND_MAX);
            particles.push_back(newParticle);
        }
    }

    void Update(float elapsed){
        for(int i = 0; i<particles.size(); i++){
            particles[i].position.x += particles[i].velocity.x*elapsed;
            particles[i].position.y += particles[i].velocity.y*elapsed;
            
            particles[i].velocity.y -= gravity * elapsed;
            particles[i].lifetime += elapsed;
            
            if(particles[i].lifetime >= maxLifetime){
                particles[i].lifetime = 0.0f;
                particles[i].position.x = position.x;
                particles[i].position.y = position.y;
                particles[i].velocity.y = 0.2f;
                particles[i].velocity.y *= ((float)rand()/(float)RAND_MAX);
            }
        }
        
    }
    void Render(ShaderProgram* program){
        
        Matrix projectionMatrix;
        projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        Matrix modelMatrix;
        Matrix viewMatrix;
        
        program->SetViewMatrix(viewMatrix);
        program->SetModelMatrix(modelMatrix);
        program->SetProjectionMatrix(projectionMatrix);
        
        glBlendFunc (GL_SRC_ALPHA, GL_ONE);

        glUseProgram(program->programID);
        
        std::vector<float> vertices;
        std::vector<float> texCoords;

        for(int i = 0; i < particles.size(); i++){
            float m = (particles[i].lifetime/maxLifetime);
            float size = lerp(startSize, endSize, m);

            vertices.insert(vertices.end(), {
                particles[i].position.x - size, particles[i].position.y + size,
                particles[i].position.x - size, particles[i].position.y - size,
                particles[i].position.x + size, particles[i].position.y + size,
                particles[i].position.x + size, particles[i].position.y + size,
                particles[i].position.x - size, particles[i].position.y - size,
                particles[i].position.x + size, particles[i].position.y - size
            });
            texCoords.insert(texCoords.end(), {
                0.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 0.0f,
                1.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f
            });
    }
        
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
        glEnableVertexAttribArray(program->positionAttribute);
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        glBindTexture(GL_TEXTURE_2D, fireTexture);
        glDrawArrays(GL_TRIANGLES, 0, int(vertices.size()/2));
        
        
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        
    };
    

    float startSize = 0.4f;
    float endSize = 0.0f;
    Vector3 position;
    float gravity;
    GLuint fireTexture = LoadTexture(RESOURCE_FOLDER"fire2.png");

    float maxLifetime;
    std::vector<Particle> particles;
};



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
        winBGTexture = LoadTexture(RESOURCE_FOLDER"level1BG.png");
        loseBGTexture = LoadTexture(RESOURCE_FOLDER"level2BG.png");
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
    GLuint winBGTexture;
    GLuint loseBGTexture;
    
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
    gameState(GLuint StateNum):emit(12, -0.65, -1.9), emit2(6, 1.0, -1.9)
    {
        fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");
        alienTexture = LoadTexture(RESOURCE_FOLDER"alienGreen.png");
        spaceSpriteSheet = LoadTexture(RESOURCE_FOLDER"spaceSpriteSheet.png");
        itemSpriteSheet = LoadTexture(RESOURCE_FOLDER"itemSpriteSheet.png");
        p1_front = LoadTexture(RESOURCE_FOLDER"p1_front.png");
        p1_jump = LoadTexture(RESOURCE_FOLDER"p1_jump.png");
        p1_walk = LoadTexture(RESOURCE_FOLDER"p1_spritesheet.png");
        woodSpriteSheet = LoadTexture(RESOURCE_FOLDER"woodSpriteSheet.png");
        blockTexture = LoadTexture(RESOURCE_FOLDER"blockerBody.png");
        blockMadTexture = LoadTexture(RESOURCE_FOLDER"blockerMad.png");
        
        fly01 = LoadTexture(RESOURCE_FOLDER"flyFly1.png");
        fly02 = LoadTexture(RESOURCE_FOLDER"flyFly2.png");
        
        walk03 = LoadTexture(RESOURCE_FOLDER"p1_walk03.png");
        walk04 = LoadTexture(RESOURCE_FOLDER"p1_walk04.png");
        walk05 = LoadTexture(RESOURCE_FOLDER"p1_walk05.png");
        walk06 = LoadTexture(RESOURCE_FOLDER"p1_walk06.png");
        walk07 = LoadTexture(RESOURCE_FOLDER"p1_walk07.png");
        
        level1BG = LoadTexture(RESOURCE_FOLDER"level1.png");
        level2BG = LoadTexture(RESOURCE_FOLDER"level2.png");
        level3BG = LoadTexture(RESOURCE_FOLDER"level3.png");
        
        snailWalk01 = LoadTexture(RESOURCE_FOLDER"snailWalk1.png");
        snailWalk02 = LoadTexture(RESOURCE_FOLDER"snailWalk2.png");
        snailShell = LoadTexture(RESOURCE_FOLDER"snailShell.png");
        
        
        SheetSprite woodSheet = SheetSprite(woodSpriteSheet, 0.0f/1024.0f, 630.0f/1024.0f, 220.0f/1024.0f, 140.0f/1024.0f, 0.2);
        SheetSprite playerSheet = SheetSprite(spaceSpriteSheet, 247.0f/1024.0f, 84.0f/1024.0f, 99.0f/1024.0f, 75.0f/1024.0f, 0.2);
        SheetSprite itemSheet = SheetSprite(itemSpriteSheet, 288.0f/1024.0f, 432.0f/1024.0f, 70.0f/1024.0f, 70.0f/1024.0f, 0.2);
        SheetSprite alienSheet = SheetSprite(alienTexture, 70.0f/512.0f, 92.0f/512.0f, 66.0f/512.0f, 92.0f/512.0f, 0.2);
        
        font1 = Entity(SheetSprite(), -1.08-1.5f+0.3f+0.2f, 0.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
        font2 = Entity(SheetSprite(), -1.08-1.5f+2.0f+0.1f+0.2f, 0.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
        player = Entity(playerSheet, -3.35f, -1.0f, 1.0f/1.3, 1.5f/1.3, 0.0f, 0.0f, 0.0f, -2.0f, ENTITY_PLAYER);
        coin = Entity(itemSheet, 2.5f, 1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_COIN);
        snail01 = Entity(playerSheet, -1.5f, -1.5f+2.0*0.2*0.5-1.0*0.5*0.5, 1.5f/1.5, 1.0f/1.5, 0.0, 0.0f, -1.0f, -2.0f, ENTITY_ENEMY_SNAIL);
        snail02 = Entity(playerSheet, -1.5f+0.9f, -1.5f+0.7f+2.0*0.2*0.5-1.0*0.5*0.5, 1.5f/1.5, 1.0f/1.5, 0.0, 0.0f, -1.0f, -2.0f, ENTITY_ENEMY_SNAIL);
        snail03 = Entity(playerSheet, -1.5f+2.3f, -1.5f+2.1f+2.0*0.2*0.5-1.0*0.5*0.5, 1.5f/1.5, 1.0f/1.5, 0.0, 0.0f, -1.0f, -2.0f, ENTITY_ENEMY_SNAIL);
        snail04 = Entity(playerSheet, -1.0f, -1.0+2.0*0.2*0.5-1.0*0.5*0.5, 1.5f/1.5, 1.0f/1.5, 0.0, 0.0f, -2.0f, -2.0f, ENTITY_ENEMY_SNAIL);
        snail05 = Entity(playerSheet, -2.0f, -2.0+1.0/1.5*0.5*0.2, 1.5f/1.5, 1.0f/1.5, 0.0, 0.0f, -2.0f, -2.0f, ENTITY_ENEMY_SNAIL);
        fly = Entity(playerSheet, -2.0f, 1.7f, 1.5f/1.5, 1.0f/1.5, 0.0, 0.0f, -2.0f, -3.0f, ENTITY_ENEMY_FLY);
        
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
        
        snail04.snailWalk01 = snailWalk01;
        snail04.snailWalk02 = snailWalk02;
        snail04.snailShell = snailShell;
        snail04.snailNum = 4;
        
        snail05.snailWalk01 = snailWalk01;
        snail05.snailWalk02 = snailWalk02;
        snail05.snailShell = snailShell;
        snail05.snailNum = 5;
        
        fly.fly01 = fly01;
        fly.fly02 = fly02;
        
        font1.size.x = 0.2f;
        font1.size.y = 0.2f;
        font2.size.x = 0.2f;
        font2.size.y = 0.2f;
        
        jumpSound = Mix_LoadWAV("jump.wav");

        if(StateNum == 1){
            float posX = -1.5f;
            float posY = -1.8f;
            
            for (size_t i=0; i<5; i++){
                Entity* newWoodPtr = new Entity(woodSheet, posX, posY, 2.5f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
                newWoodPtr->blockTextureID = blockTexture;
                newWoodPtr->blockMadTextureID = blockMadTexture;
                woods.push_back(newWoodPtr);
                posX += 0.8f;
                posY += 0.7f;
            }
        }else if(StateNum == 2){
            
            coin.position.x = -1.3f;
            coin.position.y = 0.25f;
            
            Entity* wood1 = new Entity(woodSheet, -1.0f, -1.3f, 2.5f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            woods.push_back(wood1);
            Entity* wood2 = new Entity(woodSheet, 1.1f, -0.35f, 2.5f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            woods.push_back(wood2);
            Entity* wood3 = new Entity(woodSheet, -1.0f, 1.0f, 2.5f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            woods.push_back(wood3);
            
            for(Entity* woodPtr:woods){
                woodPtr->blockTextureID = blockTexture;
                woodPtr->blockMadTextureID = blockMadTexture;
            }
            
        }else if(StateNum == 3){
            snail01.acceleration.x = -5.0f;
            snail02.acceleration.x = -5.0f;
            snail03.acceleration.x = -5.0f;
            snail05.acceleration.x = -5.0f;
            
            coin.position.x = -3.0f;
            coin.position.y = 1.4f;
            
            Entity* wood1 = new Entity(woodSheet, -1.0f, -1.3f, 2.5f*0.9, 2.0f*0.9, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            woods.push_back(wood1);
            Entity* wood2 = new Entity(woodSheet, 1.1f, -0.5f, 2.5f*0.9, 2.0f*0.9, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            woods.push_back(wood2);
            Entity* wood3 = new Entity(woodSheet, -1.0f, 0.6f, 2.5f*0.9, 2.0f*0.9, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            woods.push_back(wood3);
            Entity* wood4 = new Entity(woodSheet, -2.0f, -1.5f, 2.5f*0.9, 2.0f*0.9, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            woods.push_back(wood4);
            Entity* wood5 = new Entity(woodSheet, 2.0f, 0.45f, 2.5f*0.9, 2.0f*0.9, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC);
            woods.push_back(wood5);
            
            for(Entity* woodPtr:woods){
                woodPtr->blockTextureID = blockTexture;
                woodPtr->blockMadTextureID = blockMadTexture;
            }
            
            
        }
    };
    

    GLuint fontTexture;
    GLuint alienTexture;
    GLuint spaceSpriteSheet;
    GLuint itemSpriteSheet;
    GLuint p1_front;
    GLuint p1_jump;
    GLuint p1_walk;
    GLuint woodSpriteSheet;
    GLuint blockTexture;
    GLuint blockMadTexture;
    
    GLuint fly01;
    GLuint fly02;
    
    GLuint walk03;
    GLuint walk04;
    GLuint walk05;
    GLuint walk06;
    GLuint walk07;
    
    GLuint level1BG;
    GLuint level2BG;
    GLuint level3BG;
    
    GLuint snailWalk01;
    GLuint snailWalk02;
    GLuint snailShell;
    

    SheetSprite woodSheet;
    SheetSprite playerSheet;
    SheetSprite itemSheet;
    SheetSprite alienSheet;
    
    
    
    std::vector<Entity*> woods;
    Entity player;
    Entity coin;
    Entity snail01;
    Entity snail02;
    Entity snail03;
    Entity snail04;
    Entity snail05;
    Entity fly;
    Entity font1;
    Entity font2;
    
    Mix_Chunk* jumpSound;
    
    ParticleEmitter emit;
    ParticleEmitter emit2;
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
    
//    if(gameState){
//        program->SetLightPos(gameState->player.position.x, gameState->player.position.y);
//    }
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);

//    program->SetLightPos(0, 0);
    
}

void setup(ShaderProgram* program, ShaderProgram* program_untextured){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Final Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 1280, 720);
//    glClearColor(209.0f/255.0f, 244.0f/255.0f, 248.0f/255.0f, 1.0f);
    
    program->Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    program_untextured->Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
    
    Mix_Music *level1BGM;
    level1BGM = Mix_LoadMUS("level1.wav");
    
    Mix_PlayMusic(level1BGM, -1);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void processGameInputLevel1(SDL_Event* event, bool& done, gameState* gameState){
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }else if(event->type == SDL_KEYDOWN){
            if(event->key.keysym.scancode == SDL_SCANCODE_SPACE && gameState->player.collideBottom == true){
                Mix_PlayChannel(-1, gameState->jumpSound, 0);
                gameState->player.velocity.y = 2.0f;
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
                Mix_PlayChannel(-1, gameState->jumpSound, 0);
                gameState->player.velocity.y = 1.98f;
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
                Mix_PlayChannel(-1, gameState->jumpSound, 0);
                gameState->player.velocity.y = 2.0f;
            }
        }
    }
}

void ProcessWinInput(SDL_Event* event, bool& done, mainMenuState* menuState, GameMode& gameMode){
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
    }
}

void ProcessLoseInput(SDL_Event* event, bool& done, mainMenuState* menuState, GameMode& gameMode){
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
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

void updateMainMenu(mainMenuState& state, float elapsed){
    state.board1.update(elapsed);
    state.board2.update(elapsed);
    state.board3.update(elapsed);
    state.board4.update(elapsed);
}

void updateWin(GameMode& mode){
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if(keys[SDL_SCANCODE_ESCAPE]){
        mode = STATE_MAIN_MENU;
    }
}

void updateLose(GameMode& mode){
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if(keys[SDL_SCANCODE_ESCAPE]){
        mode = STATE_MAIN_MENU;
    }
}


void updateGameLevel1(float elapsed, gameState* gameState, GameMode& mode){
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if(keys[SDL_SCANCODE_ESCAPE]){
        mode = STATE_MAIN_MENU;
    }

    gameState->player.collideBottom = false;
    gameState->player.collideTop = false;
    gameState->player.collideRight = false;
    gameState->player.collideLeft = false;
    
    gameState->player.updateY(elapsed);
    gameState->snail01.updateY(elapsed);
    gameState->snail02.updateY(elapsed);
    gameState->snail03.updateY(elapsed);
    for (Entity* woodPtr : gameState->woods){
        gameState->player.collidesWithY(woodPtr, mode, &gameState->player);
    }
    gameState->player.collidesWithY(&gameState->coin, mode, &gameState->player);
    gameState->player.collidesWithY(&gameState->snail01, mode, &gameState->player);
    gameState->player.collidesWithY(&gameState->snail02, mode, &gameState->player);
    gameState->player.collidesWithY(&gameState->snail03, mode, &gameState->player);
    
    
    gameState->player.updateX(elapsed, mode, gameState->player);
    gameState->snail01.updateX(elapsed, mode, gameState->player);
    gameState->snail02.updateX(elapsed, mode, gameState->player);
    gameState->snail03.updateX(elapsed, mode, gameState->player);
    
    for (Entity* woodPtr : gameState->woods){
        gameState->player.collidesWithX(woodPtr, mode, &gameState->player);
    }
    gameState->player.collidesWithX(&gameState->coin, mode, &gameState->player);
    gameState->player.collidesWithX(&gameState->snail01, mode, &gameState->player);
    gameState->player.collidesWithX(&gameState->snail02, mode, &gameState->player);
    gameState->player.collidesWithX(&gameState->snail03, mode, &gameState->player);
    
    gameState->emit.Update(elapsed);
    
}

void updateGameLevel2(float elapsed, gameState* gameState, GameMode& mode){
    
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if(keys[SDL_SCANCODE_ESCAPE]){
        mode = STATE_MAIN_MENU;
    }
    
    gameState->player.collideBottom = false;
    gameState->player.collideTop = false;
    gameState->player.collideRight = false;
    gameState->player.collideLeft = false;
    
    gameState->player.updateY(elapsed);
    gameState->snail04.updateY(elapsed);
    gameState->snail05.updateY(elapsed);
    for (Entity* woodPtr : gameState->woods){
        gameState->player.collidesWithY(woodPtr, mode, &gameState->player);
    }
    gameState->player.collidesWithY(&gameState->coin, mode, &gameState->player);
    gameState->player.collidesWithY(&gameState->snail04, mode, &gameState->player);
    gameState->player.collidesWithY(&gameState->snail05, mode, &gameState->player);
    

    gameState->player.updateX(elapsed, mode, gameState->player);
    gameState->snail04.updateX(elapsed, mode, gameState->player);
    gameState->snail05.updateX(elapsed, mode, gameState->player);

    for (Entity* woodPtr : gameState->woods){
        gameState->player.collidesWithX(woodPtr, mode, &gameState->player);
    }
    gameState->player.collidesWithX(&gameState->coin, mode, &gameState->player);
    gameState->player.collidesWithX(&gameState->snail04, mode, &gameState->player);
    gameState->player.collidesWithX(&gameState->snail05, mode, &gameState->player);
    
    gameState->emit2.Update(elapsed);
    
}

void updateGameLevel3(float elapsed, gameState* gameState, GameMode& mode){
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if(keys[SDL_SCANCODE_ESCAPE]){
        mode = STATE_MAIN_MENU;
    }
    
    gameState->player.collideBottom = false;
    gameState->player.collideTop = false;
    gameState->player.collideRight = false;
    gameState->player.collideLeft = false;
    
    gameState->player.updateY(elapsed);
    gameState->snail01.updateY(elapsed);
    gameState->snail02.updateY(elapsed);
    gameState->snail03.updateY(elapsed);
//    gameState->snail04.UpdateY(elapsed);
    gameState->snail05.updateY(elapsed);
    for (Entity* woodPtr : gameState->woods){
        gameState->player.collidesWithY(woodPtr, mode, &gameState->player);
    }
    gameState->player.collidesWithY(&gameState->coin, mode, &gameState->player);
    gameState->player.collidesWithY(&gameState->snail01, mode, &gameState->player);
    gameState->player.collidesWithY(&gameState->snail02, mode, &gameState->player);
    gameState->player.collidesWithY(&gameState->snail03, mode, &gameState->player);
    gameState->player.collidesWithY(&gameState->snail05, mode, &gameState->player);
    gameState->player.collidesWithY(&gameState->fly, mode, &gameState->player);


    gameState->player.updateX(elapsed, mode, gameState->player);
    gameState->snail01.updateX(elapsed, mode, gameState->player);
    gameState->snail02.updateX(elapsed, mode, gameState->player);
    gameState->snail03.updateX(elapsed, mode, gameState->player);
    gameState->snail05.updateX(elapsed, mode, gameState->player);
    gameState->fly.updateX(elapsed, mode, gameState->player);

    
    
    
    for (Entity* woodPtr : gameState->woods){
        gameState->player.collidesWithX(woodPtr, mode, &gameState->player);
    }
    gameState->player.collidesWithX(&gameState->coin, mode, &gameState->player);
    gameState->player.collidesWithX(&gameState->snail01, mode, &gameState->player);
    gameState->player.collidesWithX(&gameState->snail02, mode, &gameState->player);
    gameState->player.collidesWithX(&gameState->snail03, mode, &gameState->player);
    gameState->player.collidesWithX(&gameState->snail05, mode, &gameState->player);
    gameState->player.collidesWithX(&gameState->fly, mode, &gameState->player);
}

void renderWin(ShaderProgram* program, ShaderProgram* program_untextured, mainMenuState* menuState){
//    font1(SheetSprite(), -2.08, 0.3f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
//    font2(SheetSprite(), -0.28f, 0.3f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
    light lights[4] = {*new light(-1.8, 0.3f), *new light(0.7f, 0.3f), *new light(-0.7f, 0.3f), *new light(1.6f,0.3)};
    program->SetLightPos(lights);
    program->SetLightIntensity(1.5f);
//    glClearColor(209.0f/255.0f, 244.0f/255.0f, 248.0f/255.0f, 1.0f);
    drawBackground(program, menuState->winBGTexture, NULL);
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    modelMatrix.Translate(menuState->font1.position.x, menuState->font1.position.y, menuState->font1.position.z);
    Matrix viewMatrix;
    
    program->SetProjectionMatrix(projectionMatrix);
    program->SetModelMatrix(modelMatrix);
    program->SetViewMatrix(viewMatrix);
    
    DrawText(program, menuState->fontTexture, "YOU'VE WON!!!", menuState->font1.size.x, -menuState->font1.size.x/2.5);
    modelMatrix.Identity();
    modelMatrix.Translate(menuState->font2.position.x, menuState->font2.position.y, menuState->font2.position.z);
    program->SetModelMatrix(modelMatrix);

    DrawText(program, menuState->fontTexture, "PRESE ESC TO RETURN", menuState->font2.size.x, -menuState->font2.size.x/2.5);
}

void renderLose(ShaderProgram* program, ShaderProgram* program_untextured, mainMenuState* menuState){
//    program->SetLightPos(0.0f, 0.0f);
//    font4(SheetSprite(), -0.8f,-0.8f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
    light lights[4] = {*new light(0.0, 0.3f), *new light(0.7f, -0.8f), *new light(-0.7f, -0.8f), *new light(1000.0f,0.3)};
    program->SetLightPos(lights);
    program->SetLightIntensity(1);
    drawBackground(program, menuState->loseBGTexture, NULL);
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    modelMatrix.Translate(-0.6f, 0.0f, 0.0f);
    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    modelMatrix.Translate(menuState->font2.position.x, menuState->font2.position.y, menuState->font2.position.z);
    Matrix viewMatrix;
    
    program->SetProjectionMatrix(projectionMatrix);
    program->SetModelMatrix(modelMatrix);
    program->SetViewMatrix(viewMatrix);
    
    DrawText(program, menuState->fontTexture, "YOU'VE LOST!!!", menuState->font1.size.x, -menuState->font1.size.x/2.5);
    modelMatrix.Identity();
    modelMatrix.Translate(menuState->font4.position.x, menuState->font4.position.y, menuState->font4.position.z);
    modelMatrix.Translate(-0.4f, 0.0f, 0.0f);
    program->SetModelMatrix(modelMatrix);
    
    DrawText(program, menuState->fontTexture, "PRESE ESC TO RETURN", menuState->font2.size.x, -menuState->font2.size.x/2.5);
}

float timeAll = 0.0f;


void renderMainMenu(ShaderProgram* program, ShaderProgram* program_untextured, mainMenuState* menuState, float elapsed){
    
//    font1(SheetSprite(), -2.08, 0.3f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
//    font2(SheetSprite(), -0.28f, 0.3f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
//    font3(SheetSprite(), 1.57f, 0.3f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_STATIC),
//    font4(SheetSprite(), -0.8f,-0.8f
    
    timeAll += elapsed;
    if(int(timeAll)%2 ==0){
    
        light lights[4] = {*new light(-2.08, 0.3f), *new light(-0.28f, 0.3f), *new light(1.57f, 0.3f), *new light(-0.8f,-0.8f)};
        program->SetLightPos(lights);
    }else{
        light lights[4] = {*new light(-2000, -1.5f), *new light(0.00f, -0.0f), *new light(-2000.0f, -0.3f), *new light(-2000,1.0f)};
        program->SetLightPos(lights);
    }
    
    //    glClearColor(209.0f/255.0f, 244.0f/255.0f, 248.0f/255.0f, 1.0f);
    program->SetLightIntensity(1.0f);
//    program->SetLightPos(0.0f, 0.0f);
    
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
    modelMatrix.Translate(0.2, 0.0, 0.0);
    program->SetModelMatrix(modelMatrix);
    DrawText(program, menuState->fontTexture, "EXIT GAME", menuState->font4.size.x, -menuState->font4.size.x/2.5);
    
    
}

void renderGameLevel1(ShaderProgram* program, gameState* gameState, float elapsed){
    glUseProgram(program->programID);
    
    
    light lights[4] = {*new light(gameState->player.position.x, gameState->player.position.y), *new light(-0.65, -1.9f), *new light(1000.0f, 0.3f), *new light(1000.0f,-0.8f)};
    program->SetLightPos(lights);
    program->SetLightIntensity(0.5f);
//    program->SetLightPos(gameState->player.position.x, gameState->player.position.y);
    
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
    
    gameState->emit.Render(program);
}

void renderGameLevel2(ShaderProgram* program, gameState* gameState, float elapsed){
    light lights[4] = {*new light(gameState->player.position.x, gameState->player.position.y), *new light(gameState->emit2.position.x, gameState->emit2.position.y), *new light(1000.0f, 0.3f), *new light(1000.0f,-0.8f)};
    program->SetLightPos(lights);
    program->SetLightIntensity(0.5f);
//    program->SetLightPos(gameState->player.position.x, gameState->player.position.y);
    drawBackground(program, gameState->level2BG, gameState);
    //    glClearColor(135.0f/255.0f, 149.0f/255.0f, 150.0f/255.0f, 1.0f);
    gameState->player.Render(program, &gameState->player, elapsed);
    for (Entity* woodPtr : gameState->woods){
        woodPtr->Render(program, &gameState->player, elapsed);
    }

    gameState->snail04.Render(program, &gameState->player, elapsed);
    gameState->snail05.Render(program, &gameState->player, elapsed);
    gameState->coin.Render(program, &gameState->player, elapsed);
    
    gameState->emit2.Render(program);
}

void renderGameLevel3(ShaderProgram* program, gameState* gameState, float elapsed){
    light lights[4] = {*new light(gameState->player.position.x, gameState->player.position.y), *new light(-1000.0f, 0.3f), *new light(1000.0f, 0.3f), *new light(1000.0f,-0.8f)};
    program->SetLightPos(lights);
    program->SetLightIntensity(0.5f);
//    program->SetLightPos(gameState->player.position.x, gameState->player.position.y);
    drawBackground(program, gameState->level3BG, gameState);
    //    glClearColor(135.0f/255.0f, 149.0f/255.0f, 150.0f/255.0f, 1.0f);
    gameState->player.Render(program, &gameState->player, elapsed);
    for (Entity* woodPtr : gameState->woods){
        woodPtr->Render(program, &gameState->player, elapsed);
    }
    
    gameState->snail01.Render(program, &gameState->player, elapsed);
    gameState->snail02.Render(program, &gameState->player, elapsed);
    gameState->snail03.Render(program, &gameState->player, elapsed);
//    gameState->snail04.Render(program, &gameState->player, elapsed);
    gameState->snail05.Render(program, &gameState->player, elapsed);
    gameState->fly.Render(program, &gameState->player, elapsed);
    
    gameState->coin.Render(program, &gameState->player, elapsed);
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
    gameState gameState1(1);
    gameState gameState2(2);
    gameState gameState3(3);
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
            ProcessMainMenuInput(&event, done, &menuState, mode);
        }else if(mode == STATE_WIN){
            ProcessWinInput(&event, done, &menuState, mode);
        }else if(mode == STATE_LOSE){
            ProcessWinInput(&event, done, &menuState, mode);
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
                updateMainMenu(menuState, FIXED_TIMESTEP);
            }else if(mode == STATE_WIN){
                updateWin(mode);
            }else if(mode == STATE_LOSE){
                updateLose(mode);
            }else if(mode == STATE_GAME_LEVEL_1){
                updateGameLevel1(FIXED_TIMESTEP, &gameState1, mode);
            }else if(mode == STATE_GAME_LEVEL_2){
                updateGameLevel2(FIXED_TIMESTEP, &gameState2, mode);
            }else if(mode == STATE_GAME_LEVEL_3){
                updateGameLevel3(FIXED_TIMESTEP, &gameState3, mode);
            }
            
            elapsed -= FIXED_TIMESTEP;
        }
        
        accumulator = elapsed;
        if(mode == STATE_MAIN_MENU){
            renderMainMenu(&program, &program_untextured, &menuState, FIXED_TIMESTEP);
        }else if(mode == STATE_WIN){
            renderWin(&program, &program_untextured, &menuState);
        }else if(mode == STATE_LOSE){
            renderLose(&program, &program_untextured, &menuState);
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

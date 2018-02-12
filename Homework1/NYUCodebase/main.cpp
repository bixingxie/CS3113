#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "Matrix.h"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;


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



int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif

    
//Setup
    glViewport(0, 0, 1280, 720);
    
    //this is for textured polygons
    ShaderProgram program_textured;
    program_textured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    //this is for untextured polygons
    ShaderProgram program_untextured;
    program_untextured.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    program_untextured.SetColor(250.0f/255.0f, 214.0f/255.0f, 137.0f/255.0f, 0.7f);
    
    GLuint durianTexture01 = LoadTexture(RESOURCE_FOLDER"durian01.png");
    GLuint durianTexture02 = LoadTexture(RESOURCE_FOLDER"durian02.png");
    GLuint monsterTexture = LoadTexture(RESOURCE_FOLDER"monster.png");
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;

    projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    
    glClearColor(254.0f/255.0f, 223.0f/255.0f, 225.0f/255.0f, 1.0f);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    float lastFrameTicks = 0.0f;
    float angle = 0.0f;
    float positionX = 0.0f;
    float positionY = 0.0f;
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
                
            }
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        glUseProgram(program_untextured.programID);
        
        modelMatrix.Translate(0.0f, 1.0f, 0.0f);
        program_untextured.SetProjectionMatrix(projectionMatrix);
        program_untextured.SetModelMatrix(modelMatrix);
        program_untextured.SetViewMatrix(viewMatrix);
        
        float vertices[] = {0.5f, -0.5f, 0.0f, 0.5f, -0.5f, -0.5f};
        glVertexAttribPointer(program_untextured.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program_untextured.positionAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glDisableVertexAttribArray(program_untextured.positionAttribute);
        

        
        glUseProgram(program_textured.programID);
        
        modelMatrix.Identity();
        modelMatrix.Translate(1.0f, 0.0f, 0.0f);
        modelMatrix.Scale(1.3f, 1.3f, 1.0f);
        modelMatrix.Rotate(angle);
        angle += 3.14/2*elapsed;
        
        program_textured.SetProjectionMatrix(projectionMatrix);
        program_textured.SetModelMatrix(modelMatrix);
        program_textured.SetViewMatrix(viewMatrix);
        
        glBindTexture(GL_TEXTURE_2D, durianTexture01);
        
        float vertices01[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program_textured.positionAttribute, 2, GL_FLOAT, false, 0, vertices01);
        glEnableVertexAttribArray(program_textured.positionAttribute);
        
        float texCoords01[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program_textured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords01);
        glEnableVertexAttribArray(program_textured.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        
        
        modelMatrix.Identity();
        modelMatrix.Scale(1.69, 1.2, 1);
        modelMatrix.Translate(-3.55/2, -1.0f, 0.0f);
        modelMatrix.Translate(positionX, positionY, 0.0f);
        program_textured.SetModelMatrix(modelMatrix);
        //value of cos(45)
        positionX += 0.525322 * elapsed * 0.2;
        //value of sin(45)
        positionY += 0.850904 * elapsed * 0.2;
        
        
        glBindTexture(GL_TEXTURE_2D, durianTexture02);
        
        float vertices02[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program_textured.positionAttribute, 2, GL_FLOAT, false, 0, vertices02);
        glEnableVertexAttribArray(program_textured.positionAttribute);
        
        float texCoords02[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program_textured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords02);
        glEnableVertexAttribArray(program_textured.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        
        modelMatrix.Identity();
        modelMatrix.Translate(-0.1f, 0.0f, 0.0f);
        program_textured.SetModelMatrix(modelMatrix);
        
        glBindTexture(GL_TEXTURE_2D, monsterTexture);
        
        float vertices03[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program_textured.positionAttribute, 2, GL_FLOAT, false, 0, vertices03);
        glEnableVertexAttribArray(program_textured.texCoordAttribute);
        
        glUseProgram(program_textured.programID);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        
        glDisableVertexAttribArray(program_textured.positionAttribute);
        glDisableVertexAttribArray(program_textured.texCoordAttribute);
       
        
        SDL_GL_SwapWindow(displayWindow);

        

    }
    
    SDL_Quit();
    return 0;
}

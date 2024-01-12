#ifndef DISPLAY_H
#define DISPLAY_H

#include <cstdint>
#include <vector>
#include <GL/glew.h>
#include "Memory.h"

#define DISPLAY_WIDTH 256
#define DISPLAY_HEIGHT 192

#define VERTEX_SHADER_FILE "shaders/vertex.glsl"
#define FRAGMENT_SHADER_FILE "shaders/fragment.glsl"

class Display {
public:
    Display(Spectrum48KMemory* memory);
    ~Display();
    void draw(int windowWidth, int windowHeight);

    float getScale();
    void setScale(float scale);

protected:
        void generateVertexBuffer();

        void generateUVs();

        void glDraw(int windowWidth, int windowHeight);

private:
    Spectrum48KMemory* m_memory;
    uint8_t m_pixels[DISPLAY_WIDTH * DISPLAY_HEIGHT * 3];

    std::vector<GLfloat> m_vertexBuffer;
    std::vector<GLfloat> m_UVs;
    GLuint m_vaoID;
    GLuint m_vboID;
    GLuint m_programID;
    GLuint m_textureID;
    GLuint m_samplerID;
    GLuint m_uvID;

    float m_scale;

    // Are the flashing colors currently inverted?
    bool m_inverted;

    // Number of frames since last inversion of colors
    int m_frames;

    // Methods for OpenGL setup and rendering
    void InitialiseOpenGL();
    void cleanupOpenGL();
    void updatePixelsFromMemory();

    GLuint loadAndCompileShader(const std::string& filename, GLenum shaderType);
    GLuint linkShaderProgram(GLuint vertexShaderID, GLuint fragmentShaderID);

    void convertColourCodeToRGBA(int colourCode, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);
};

#endif 

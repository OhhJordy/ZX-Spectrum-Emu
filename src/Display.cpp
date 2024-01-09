#include "Display.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include "memory.h"

// Constants for display dimensions
const int DISPLAY_WIDTH = 256;
const int DISPLAY_HEIGHT = 192;

// Shaders file paths (adjust these paths as necessary)
const std::string VERTEX_SHADER_FILE = "path/to/vertex_shader.glsl";
const std::string FRAGMENT_SHADER_FILE = "path/to/fragment_shader.glsl";

Display::Display(Spectrum48KMemory* memory)
    : m_memory(memory), m_inverted(false), m_frames(0), m_scale(2.0f) {
    // Initialise OpenGL, shaders, and textures
    InitialiseOpenGL();
}

Display::~Display() {
    cleanupOpenGL();
}

std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


void Display::draw(int windowWidth, int windowHeight) {
    // Rendering logic
    updatePixelsFromMemory();
    render(windowWidth, windowHeight);
    handleFlashingAttributes();
}


void Display::InitialiseOpenGL() {
    // Initialise GLEW or another OpenGL loader here, if needed
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("Failed to Initialise GLEW");
    }

    // Generate and bind Vertex Array Object (VAO)
    glGenVertexArrays(1, &m_vaoID);
    glBindVertexArray(m_vaoID);

    // Generate Vertex Buffer Object (VBO) and UVs buffer
    glGenBuffers(1, &m_vboID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_vertexBuffer.size(), m_vertexBuffer.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_uvID);
    glBindBuffer(GL_ARRAY_BUFFER, m_uvID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_UVs.size(), m_UVs.data(), GL_STATIC_DRAW);

    // Generate and configure texture
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    // Set texture parameters here (wrap, filter)

    // Load, compile, and link shaders
    GLuint vertexShaderID = loadAndCompileShader(VERTEX_SHADER_FILE, GL_VERTEX_SHADER);
    GLuint fragmentShaderID = loadAndCompileShader(FRAGMENT_SHADER_FILE, GL_FRAGMENT_SHADER);
    m_programID = linkShaderProgram(vertexShaderID, fragmentShaderID);

    // Clean up shaders (they can be detached and deleted after linking)
    glDetachShader(m_programID, vertexShaderID);
    glDetachShader(m_programID, fragmentShaderID);
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

}


void Display::cleanupOpenGL() {
    glDeleteBuffers(1, &m_vboID);
    glDeleteBuffers(1, &m_uvID);
    glDeleteTextures(1, &m_textureID);
    glDeleteVertexArrays(1, &m_vaoID);
    glDeleteProgram(m_programID);
}

GLuint Display::loadAndCompileShader(const std::string& filename, GLenum shaderType) {
    std::string shaderCode = readFileToString(filename);
    GLuint shaderID = glCreateShader(shaderType);
    const char* sourcePtr = shaderCode.c_str();
    glShaderSource(shaderID, 1, &sourcePtr, nullptr);
    glCompileShader(shaderID);

    // Error checking for shader compilation
    GLint success;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shaderID, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        throw std::runtime_error("Shader compilation failed");
    }

    return shaderID;
}


GLuint Display::linkShaderProgram(GLuint vertexShaderID, GLuint fragmentShaderID) {
    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    // Error checking for shader program linking
    GLint success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programID, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        throw std::runtime_error("Shader program linking failed");
    }

    return programID;
}




void Display::updatePixelsFromMemory() {
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH / 8; x++) {
            // Calculate memory position for screen and colour attributes
            uint16_t address = 0x4000 + y * (DISPLAY_WIDTH / 8) + x;
            uint16_t colourAddress = 0x5800 + (y / 8) * 32 + x;

            uint8_t byte = (*m_memory)[address];
            uint8_t attr = (*m_memory)[colourAddress];
            bool bright = attr & 0x40;
            bool flash = attr & 0x80;

            for (int bit = 0; bit < 8; bit++) {
                bool set = byte & (0x80 >> bit);
                int colourCode = set ? (attr & 0x07) : (attr >> 3) & 0x07;
                colourCode += bright ? 8 : 0;

                uint8_t r, g, b;

                int pixelIndex = (y * DISPLAY_WIDTH + x * 8 + bit) * 3;
                m_pixels[pixelIndex] = r;
                m_pixels[pixelIndex + 1] = g;
                m_pixels[pixelIndex + 2] = b;
            }
        }
    }
}

void Display::convertColourCodeToRGBA(int colourCode, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
    a = 255;

    // Define the colour palette (RGB values for 0-7 colour codes)
    const uint8_t palette[8][3] = {
        {0, 0, 0},       // Black
        {0, 0, 0xD7},    // Blue
        {0xD7, 0, 0},    // Red
        {0xD7, 0, 0xD7}, // Magenta
        {0, 0xD7, 0},    // Green
        {0, 0xD7, 0xD7}, // Cyan
        {0xD7, 0xD7, 0}, // Yellow
        {0xD7, 0xD7, 0xD7} // White
    };

    // Determine if it's a bright colour
    bool bright = colourCode & 0x08;

    // Get the base colour
    int baseColourCode = colourCode & 0x07;

    // Apply the base colour
    r = palette[baseColourCode][0];
    g = palette[baseColourCode][1];
    b = palette[baseColourCode][2];

    // If bright, increase the RGB values
    if (bright) {
        r = std::min(255, r + 0x80);
        g = std::min(255, g + 0x80);
        b = std::min(255, b + 0x80);
    }
}



void Display::render(int windowWidth, int windowHeight) {
    // Use the shader program
    glUseProgram(m_programID);

    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pixels);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Set up the viewport
    glViewport(0, 0, windowWidth, windowHeight);

    // Bind VAO
    glBindVertexArray(m_vaoID);

    glDrawArrays(GL_TRIANGLES, 0, 6); 

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


void Display::handleFlashingAttributes() {
    static int frameCounter = 0;
    frameCounter++;

    if (frameCounter >= 30) {
        frameCounter = 0;
        m_flashState = !m_flashState;

        updatePixelsFromMemory();
    }
}



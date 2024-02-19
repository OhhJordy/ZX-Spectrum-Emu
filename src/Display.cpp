#include "Display.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <GL/glew.h>

Display::Display(Spectrum48KMemory* memory)
    : m_memory(memory), m_inverted(false), m_frames(0), m_scale(2.0f) {
    // Initialize OpenGL, shaders, and textures
    InitialiseOpenGL();
    generateVertexBuffer();
    generateUVs();
}

Display::~Display() {
    cleanupOpenGL();
}

void Display::convertColourCodeToRGBA(int colourCode, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
    switch (colourCode) {
        case 0: // Black
            r = 0; g = 0; b = 0; a = 255;
            break;
        case 1: // Blue
            r = 0; g = 0; b = 255; a = 255;
            break;
        case 2: // Red
            r = 255; g = 0; b = 0; a = 255;
            break;
        case 3: // Magenta
            r = 255; g = 0; b = 255; a = 255;
            break;
        case 4: // Green
            r = 0; g = 255; b = 0; a = 255;
            break;
        case 5: // Cyan
            r = 0; g = 255; b = 255; a = 255;
            break;
        case 6: // Yellow
            r = 255; g = 255; b = 0; a = 255;
            break;
        case 7: // White
            r = 255; g = 255; b = 255; a = 255;
            break;
        default:
            r = 0; g = 0; b = 0; a = 0; // Transparent or invalid code
    }
}

void Display::draw(int windowWidth, int windowHeight) {
    updatePixelsFromMemory();
    glDraw(windowWidth, windowHeight);
   
}

void Display::InitialiseOpenGL() {
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW." << std::endl;
        return;
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
    // Set texture parameters here (wrap, filter, etc.)

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

void glLogLastError(const std::string& message) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << message << " (" << err << ")" << std::endl;
    }
}


void Display::cleanupOpenGL() {
    glDeleteBuffers(1, &m_vboID);
    glDeleteBuffers(1, &m_uvID);
    glDeleteTextures(1, &m_textureID);
    glDeleteVertexArrays(1, &m_vaoID);
    glDeleteProgram(m_programID);
}

GLuint Display::loadAndCompileShader(const std::string& filename, GLenum shaderType) {
    // Open the file
    std::ifstream shaderFile(filename);
    if (!shaderFile.is_open()) {
        throw std::runtime_error("Could not open shader file: " + filename);
    }

    // Read file's buffer contents into stream
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    
    // Convert stream into string
    std::string shaderCode = shaderStream.str();
    
    // Close file handler
    shaderFile.close();

    // Create a shader object
    GLuint shaderID = glCreateShader(shaderType);
    if (shaderID == 0) {
        throw std::runtime_error("Error creating shader type: " + std::to_string(shaderType));
    }

    // Compile Shader
    std::cout << "Compiling shader: " << filename << std::endl;
    const char* shaderSourcePointer = shaderCode.c_str();
    glShaderSource(shaderID, 1, &shaderSourcePointer, NULL);
    glCompileShader(shaderID);

    // Check Shader
    GLint result = GL_FALSE;
    int infoLogLength;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> shaderErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(shaderID, infoLogLength, NULL, &shaderErrorMessage[0]);
        std::cerr << &shaderErrorMessage[0] << std::endl;
    }

    if (result == GL_FALSE) {
        throw std::runtime_error("Shader compilation failed: " + filename);
    }

    return shaderID;
}


GLuint Display::linkShaderProgram(GLuint vertexShaderID, GLuint fragmentShaderID) {
    std::cout << "Linking shader program..." << std::endl;
    GLint Result = GL_FALSE;
    int InfoLogLength;

    GLuint programID = glCreateProgram();
    glLogLastError("glCreateProgram");

    glAttachShader(programID, vertexShaderID);
    glLogLastError("glAttachShader for vertexShaderID");

    glAttachShader(programID, fragmentShaderID);
    glLogLastError("glAttachShader for fragmentShaderID");

    glLinkProgram(programID);
    glLogLastError("glLinkProgram");


    // Check the program
    glGetProgramiv(programID, GL_LINK_STATUS, &Result);
    if (Result == GL_FALSE) { std::cerr << "Link error" << std::endl; }
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        std::cerr << "Error: " << ProgramErrorMessage[0] << std::endl;
        return -1;
    }
    std::cout << "Successful" << std::endl;

    return programID;
}

void Display::generateVertexBuffer() {
    m_UVs.push_back(0.0f);
    m_UVs.push_back(1.0f);

    m_UVs.push_back(1.0f);
    m_UVs.push_back(1.0f);

    m_UVs.push_back(1.0f);
    m_UVs.push_back(0.0f);

    m_UVs.push_back(0.0f);
    m_UVs.push_back(1.0f);

    m_UVs.push_back(1.0f);
    m_UVs.push_back(0.0f);

    m_UVs.push_back(0.0f);
    m_UVs.push_back(0.0f);
}

void Display::generateUVs() {
    m_UVs.push_back(0.0f);
    m_UVs.push_back(1.0f);

    m_UVs.push_back(1.0f);
    m_UVs.push_back(1.0f);

    m_UVs.push_back(1.0f);
    m_UVs.push_back(0.0f);

    m_UVs.push_back(0.0f);
    m_UVs.push_back(1.0f);

    m_UVs.push_back(1.0f);
    m_UVs.push_back(0.0f);

    m_UVs.push_back(0.0f);
    m_UVs.push_back(0.0f);
}

void Display::updatePixelsFromMemory() {
    // Screen dimensions of ZX Spectrum
    const int width = 256;
    const int height = 192;

    // Iterate over each row
    for (int y = 0; y < height; y++) {
        // Calculate which memory address this row starts at
        int rowStart = 0x4000 + ((y & 0xC0) << 5) + ((y & 0x38) << 2) + ((y & 0x07) << 8);

        // Iterate over each column
        for (int x = 0; x < width; x += 8) {
            // Get the byte for these 8 pixels
            uint8_t pixels = m_memory->read(rowStart + (x >> 3));

            // Get color information
            int colorAddress = 0x5800 + ((y >> 3) << 5) + (x >> 3);
            uint8_t colorData = m_memory->read(colorAddress);
            uint8_t inkColor = colorData & 0x07; // Lower 3 bits
            uint8_t paperColor = (colorData >> 3) & 0x07; // Next 3 bits

            // Convert colors to RGB
            uint8_t inkRGB[3], paperRGB[3], alpha;
            convertColourCodeToRGBA(inkColor, inkRGB[0], inkRGB[1], inkRGB[2], alpha);
            convertColourCodeToRGBA(paperColor, paperRGB[0], paperRGB[1], paperRGB[2], alpha);


            // Set pixels
            for (int bit = 0; bit < 8; bit++) {
                int pixelIndex = (y * width + x + bit) * 3;
                if (pixels & (0x80 >> bit)) {
                    // Set to ink color
                    m_pixels[pixelIndex] = inkRGB[0];
                    m_pixels[pixelIndex + 1] = inkRGB[1];
                    m_pixels[pixelIndex + 2] = inkRGB[2];
                } else {
                    // Set to paper color
                    m_pixels[pixelIndex] = paperRGB[0];
                    m_pixels[pixelIndex + 1] = paperRGB[1];
                    m_pixels[pixelIndex + 2] = paperRGB[2];
                }
            }
        }
    }
}


void Display::glDraw(int windowWidth, int windowHeight) {
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


float Display::getScale() {
    return m_scale;
}

void Display::setScale(float scale) {
    m_scale = scale;
}

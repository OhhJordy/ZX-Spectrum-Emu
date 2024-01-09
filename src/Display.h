#ifndef DISPLAY_H
#define DISPLAY_H

#include <cstdint>
#include <vector>

class ScreenAttribute {
public:
    bool flash;
    bool bright;
    int ink;
    int paper;

    ScreenAttribute() : flash(false), bright(false), ink(0), paper(7) {}
};

class Display {
public:
    Display();
    ~Display();

    void setBorderColour(uint8_t colour);
    void setPaperColour(int attrPos, uint8_t colour);
    void setInkColour(int attrPos, uint8_t colour);
    void refreshScreen();


    void handleResize(int newWidth, int newHeight);

    // Methods to update pixels on the screen
    void setPixels(int xpos, int ypos, uint8_t byte);

private:
    std::vector<ScreenAttribute> screenAttributes;
    std::vector<uint32_t> screenBuffer; 
    uint8_t borderColour;
    int screenWidth;
    int screenHeight;
    int borderSize;

    void convertColourCodeToRGBA(int colorCode, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);
    void renderScreen();
    void renderBorder();

};

#endif 

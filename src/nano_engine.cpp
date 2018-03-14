/*
    MIT License

    Copyright (c) 2018, Alexey Dynda

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "nano_engine.h"
#include "lcd/lcd_common.h"
#include "ssd1306.h"

extern SFixedFontInfo s_fixedFont;

///////////////////////////////////////////////////////////////////////////////
////// NANO ENGINE RGB        /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

NanoCanvas8 NanoEngine8::canvas;

//uint8_t NanoEngine8::m_buffer[NE_TILE_SIZE * NE_TILE_SIZE];

NanoEngine8::NanoEngine8()
   : NanoEngineBase()
   , NanoEngineTiler()
{
}

void NanoEngine8::begin()
{
    // TODO: Maybe, this is not good place for this, but
    // TODO: ssd1331 must be initialized in Horizontal addressing mode
    ssd1331_setMode(0);
    canvas.begin(NE_TILE_WIDTH, NE_TILE_HEIGHT, m_buffer);
    NanoEngineBase::begin();
}

void NanoEngine8::display()
{
    m_lastFrameTs = millis();
    for (uint8_t y = 0; y < s_displayHeight; y = y + NE_TILE_HEIGHT)
    {
        uint16_t flag = m_refreshFlags[y >> NE_TILE_SIZE_BITS];
        m_refreshFlags[y >> NE_TILE_SIZE_BITS] = 0;
        for (uint8_t x = 0; x < s_displayWidth; x = x + NE_TILE_WIDTH)
        {
            if (flag & 0x01)
            {
                canvas.setOffset(x, y);
                if ((m_onDraw) && (m_onDraw()))
                {
                    canvas.blt();
                }
            }
            flag >>=1;
        }
    }
    m_cpuLoad = ((millis() - m_lastFrameTs)*100)/m_frameDurationMs;
}

void NanoEngine8::notify(const char *str)
{
    NanoRect rect = { 8, (s_displayHeight>>1) - 8, s_displayWidth - 8, (s_displayHeight>>1) + 8 };
    // TODO: It would be nice to calculate message height
    NanoPoint textPos = { (s_displayWidth - (lcdint_t)strlen(str)*s_fixedFont.width) >> 1, (s_displayHeight>>1) - 4 };
    refresh(rect);
    for (uint8_t y = 0; y < s_displayHeight; y = y + NE_TILE_HEIGHT)
    {
        uint16_t flag = m_refreshFlags[y >> NE_TILE_SIZE_BITS];
        m_refreshFlags[y >> NE_TILE_SIZE_BITS] = 0;
        for (uint8_t x = 0; x < s_displayWidth; x = x + NE_TILE_WIDTH)
        {
            if (flag & 0x01)
            {
                canvas.setOffset(x, y);
                if (m_onDraw) m_onDraw();
                canvas.setColor(RGB_COLOR8(0,0,0));
                canvas.fillRect(rect);
                canvas.setColor(RGB_COLOR8(192,192,192));
                canvas.drawRect(rect);
                canvas.printFixed( textPos.x, textPos.y, str);

                canvas.blt();
            }
            flag >>=1;
        }
    }
    delay(1000);
    m_lastFrameTs = millis();
    refresh();
}

///////////////////////////////////////////////////////////////////////////////
////// NANO ENGINE MONOCHROME /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/** object, representing canvas. Use it in your draw handler */
NanoCanvas1 NanoEngine1::canvas;

//uint8_t   NanoEngine1::m_buffer[NE_TILE_SIZE * NE_TILE_SIZE / 8];

NanoEngine1::NanoEngine1()
   : NanoEngineBase()
   , NanoEngineTiler()
{
}

void NanoEngine1::begin()
{
    canvas.begin(NE_TILE_WIDTH, NE_TILE_HEIGHT, m_buffer);
    NanoEngineBase::begin();
}

void NanoEngine1::display()
{
    m_lastFrameTs = millis();
    for (uint8_t y = 0; y < s_displayHeight; y = y + NE_TILE_HEIGHT)
    {
        uint16_t flag = m_refreshFlags[y >> NE_TILE_SIZE_BITS];
        m_refreshFlags[y >> NE_TILE_SIZE_BITS] = 0;
        for (uint8_t x = 0; x < s_displayWidth; x = x + NE_TILE_WIDTH)
        {
            if (flag & 0x01)
            {
                canvas.setOffset(x, y);
                if (m_onDraw)
                {
                    if (m_onDraw()) canvas.blt();
                }
            }
            flag >>=1;
        }
    }
    m_cpuLoad = ((millis() - m_lastFrameTs)*100)/m_frameDurationMs;
}

void NanoEngine1::notify(const char *str)
{
    NanoRect rect = { 8, (s_displayHeight>>1) - 8, s_displayWidth - 8, (s_displayHeight>>1) + 8 };
    // TODO: It would be nice to calculate message height
    NanoPoint textPos = { (s_displayWidth - (lcdint_t)strlen(str)*s_fixedFont.width) >> 1, (s_displayHeight>>1) - 4 };
    refresh(rect);
    for (uint8_t y = 0; y < s_displayHeight; y = y + NE_TILE_HEIGHT)
    {
        uint16_t flag = m_refreshFlags[y >> NE_TILE_SIZE_BITS];
        m_refreshFlags[y >> NE_TILE_SIZE_BITS] = 0;
        for (uint8_t x = 0; x < s_displayWidth; x = x + NE_TILE_WIDTH)
        {
            if (flag & 0x01)
            {
                canvas.setOffset(x, y);
                if (m_onDraw) m_onDraw();
                canvas.setColor(RGB_COLOR8(0,0,0));
                canvas.fillRect(rect);
                canvas.setColor(RGB_COLOR8(192,192,192));
                canvas.drawRect(rect);
                canvas.printFixed( textPos.x, textPos.y, str);

                canvas.blt();
            }
            flag >>=1;
        }
    }
    delay(1000);
    m_lastFrameTs = millis();
    refresh();
}

///////////////////////////////////////////////////////////////////////////////
////// NANO ENGINE TILER CLASS ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if 0

uint16_t NanoEngineTiler::m_refreshFlags[NanoEngineTiler::NE_MAX_TILES_NUM];

NanoEngineTiler::NanoEngineTiler()
{
    refresh();
}

void NanoEngineTiler::refresh(const NanoRect &rect)
{
    refresh(rect.p1.x, rect.p1.y, rect.p2.x, rect.p2.y);
}

void NanoEngineTiler::refresh(lcdint_t x1, lcdint_t y1, lcdint_t x2, lcdint_t y2)
{
    y1 = max(0,y1>>NE_TILE_SIZE_BITS);
    y2 = min((y2>>NE_TILE_SIZE_BITS), NE_MAX_TILES_NUM - 1);
    for(uint8_t y=y1; y<=y2; y++)
    {
        for(uint8_t x=(x1>>NE_TILE_SIZE_BITS); x<=(x2>>NE_TILE_SIZE_BITS); x++)
        {
            m_refreshFlags[y] |= (1<<x);
        }
    }
}

void NanoEngineTiler::refresh(const NanoPoint &point)
{
    if ((point.y<0) || ((point.y>>NE_TILE_SIZE_BITS)>=NE_MAX_TILES_NUM)) return;
    m_refreshFlags[(point.y>>NE_TILE_SIZE_BITS)] |= (1<<(point.x>>NE_TILE_SIZE_BITS));
}

void NanoEngineTiler::refresh()
{
    for(uint8_t i=0; i<NE_MAX_TILES_NUM; i++)
    {
        m_refreshFlags[i] = ~(0);
    }
}

#endif

///////////////////////////////////////////////////////////////////////////////
////// NANO ENGINE BASE CLASS /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static const uint8_t ENGINE_DEFAULT_FPS = 30;

/** Duration between frames in milliseconds */
uint8_t   NanoEngineBase::m_frameDurationMs = 1000/ENGINE_DEFAULT_FPS;
/** Current fps */
uint8_t   NanoEngineBase::m_fps = ENGINE_DEFAULT_FPS;
/** Current cpu load in percents */
uint8_t   NanoEngineBase::m_cpuLoad = 0;
/** Last timestamp in milliseconds the frame was updated on oled display */
uint32_t  NanoEngineBase::m_lastFrameTs;
/** Callback to call if specific tile needs to be updated */
TNanoEngineOnDraw NanoEngineBase::m_onDraw = nullptr;
/** Callback to call if buttons state needs to be updated */
TNanoEngineGetButtons NanoEngineBase::m_onButtons = nullptr;
/** Callback to call before starting oled update */
TLoopCallback NanoEngineBase::m_loop = nullptr;

NanoEngineBase::NanoEngineBase()
{
}

void NanoEngineBase::begin()
{
    m_lastFrameTs = millis();
}

void NanoEngineBase::setFrameRate(uint8_t fps)
{
    m_fps = fps;
    m_frameDurationMs = 1000/fps;
}

bool NanoEngineBase::nextFrame()
{
    bool needUpdate = (uint32_t)(millis() - m_lastFrameTs) >= m_frameDurationMs;
    if (needUpdate && m_loop) m_loop();
    return needUpdate;
}

bool NanoEngineBase::pressed(uint8_t buttons)
{
    return (m_onButtons() & buttons) == buttons;
}

bool NanoEngineBase::notPressed(uint8_t buttons)
{
    return (m_onButtons() & buttons) == 0;
}

void NanoEngineBase::connectCustomKeys(TNanoEngineGetButtons handler)
{
    m_onButtons = handler;
}

void NanoEngineBase::connectArduboyKeys()
{
    m_onButtons = arduboyButtons;
}

uint8_t NanoEngineBase::s_zkeypadPin;

uint8_t NanoEngineBase::zkeypadButtons()
{
    int buttonValue = analogRead(s_zkeypadPin);
    if (buttonValue < 100) return BUTTON_RIGHT;  
    if (buttonValue < 200) return BUTTON_UP;
    if (buttonValue < 400) return BUTTON_DOWN;
    if (buttonValue < 600) return BUTTON_LEFT;
    if (buttonValue < 800) return BUTTON_A;
    /** Z-keypad has only 5 analog buttons: no button B */
    return BUTTON_NONE;
}

void NanoEngineBase::connectZKeypad(uint8_t analogPin)
{
    NanoEngineBase::s_zkeypadPin = analogPin;
    m_onButtons = zkeypadButtons;
}

uint8_t NanoEngineBase::arduboyButtons()
{
    uint8_t buttons;
    /* Arduboy buttons available only for Atmega32U4 platform */
    #if defined(__AVR_ATmega32U4__)
    // down, up, left right
    buttons = (((~PINF) & 0B11110000)>>4);
    // A (left)
    buttons |= (((~PINE) & 0B01000000) >> 2);
    // B (right)
    buttons |= (((~PINB) & 0B00010000) << 1);
    #else
    buttons = 0;
    #endif
    return buttons;
}

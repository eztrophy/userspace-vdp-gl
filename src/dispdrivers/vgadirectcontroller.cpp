/*
  Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com) - <http://www.fabgl.com>
  Copyright (c) 2019-2022 Fabrizio Di Vittorio.
  All rights reserved.


* Please contact fdivitto2013@gmail.com if you need a commercial license.


* This library and related software is available under GPL v3.

  FabGL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FabGL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FabGL.  If not, see <http://www.gnu.org/licenses/>.
 */



#include <stdarg.h>
#include <math.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "soc/i2s_struct.h"
#include "soc/i2s_reg.h"
#include "driver/periph_ctrl.h"
#include "soc/rtc.h"
#include "esp_spi_flash.h"
#include "esp_heap_caps.h"

#ifdef USERSPACE
# include "fake_fabgl.h"
# include "malloc_wrapper.h"
#endif /* USERSPACE */

#include "fabutils.h"
#include "vgadirectcontroller.h"
#include "devdrivers/swgenerator.h"



#pragma GCC optimize ("O2")


namespace fabgl {





/*************************************************************************************/
/* VGADirectController definitions */


VGADirectController *    VGADirectController::s_instance = nullptr;
int             VGADirectController::s_scanLine;
bool                     VGADirectController::s_VSync;
#ifndef USERSPACE
lldesc_t *      VGADirectController::s_frameResetDesc;
lldesc_t * *    VGADirectController::s_DMALines = nullptr;
#endif /* !USERSPACE */



VGADirectController::VGADirectController(bool autoRun)
  : m_linesCount(2),
    m_lines(nullptr),
    m_drawScanlineCallback(nullptr),
    m_autoRun(autoRun)
{
  s_instance = this;
}


void VGADirectController::init()
{
  VGABaseController::init();
  m_doubleBufferOverDMA = false;
}


void VGADirectController::allocateViewPort()
{
  m_lines = (uint8_t**) heap_caps_malloc(sizeof(uint8_t*) * m_linesCount, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
  m_lines[0] = (uint8_t*) heap_caps_malloc(m_viewPortWidth * m_linesCount, MALLOC_CAP_DMA);
  for (int i = 1; i < m_linesCount; ++i)
    m_lines[i] = m_lines[0] + i * m_viewPortWidth;
#ifndef USERSPACE
  s_DMALines = new lldesc_t *[m_viewPortHeight];
#endif /* !USERSPACE */
}


void VGADirectController::freeViewPort()
{
  VGABaseController::freeViewPort();

  heap_caps_free((void*)m_lines[0]);
  heap_caps_free((void*)m_lines);
  m_lines = nullptr;
#ifndef USERSPACE
  delete s_DMALines;
#endif /* !USERSPACE */
}


void VGADirectController::setResolution(VGATimings const& timings, int viewPortWidth, int viewPortHeight, bool doubleBuffered)
{
  // fail if setDrawScanlineCallback() has not been called
  if (!m_drawScanlineCallback)
    return;

  VGABaseController::setResolution(timings, viewPortWidth, viewPortHeight, doubleBuffered);

  if (m_autoRun)
    run();
}


void VGADirectController::run()
{
  // must be started before interrupt alloc
#ifndef USERSPACE
  startGPIOStream();
#endif /* !USERSPACE */

  s_scanLine = 0;

  // ESP_INTR_FLAG_LEVEL1: should be less than PS2Controller interrupt level, necessary when running on the same core
#ifndef USERSPACE
  if (m_isr_handle == nullptr) {
    CoreUsage::setBusiestCore(FABGLIB_VIDEO_CPUINTENSIVE_TASKS_CORE);
    esp_intr_alloc_pinnedToCore(ETS_I2S1_INTR_SOURCE, ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_IRAM, ISRHandler, this, &m_isr_handle, FABGLIB_VIDEO_CPUINTENSIVE_TASKS_CORE);
    I2S1.int_clr.val     = 0xFFFFFFFF;
    I2S1.int_ena.out_eof = 1;
  }
#endif /* !USERSPACE */
}

void IRAM_ATTR VGADirectController::rawCopyToBitmap(int srcX, int srcY, int width, void * saveBuffer, int X1, int Y1, int XCount, int YCount)
{
  /*
  genericRawCopyToBitmap(srcX, srcY, width, (uint8_t*)saveBuffer, X1, Y1, XCount, YCount,
                        [&] (int y)                { return (uint8_t*) m_viewPort[y]; },  // rawGetRow
                        [&] (uint8_t * row, int x) { return VGA_PIXELINROW(row, x); }     // rawGetPixelInRow
                      );
                      */
}


void VGADirectController::onSetupDMABuffer(lldesc_t volatile * buffer, bool isStartOfVertFrontPorch, int scan, bool isVisible, int visibleRow)
{
#ifndef USERSPACE
  if (isVisible) {
    s_DMALines[visibleRow] = buffer;

    buffer->buf = (uint8_t *) m_lines[visibleRow % m_linesCount];

    // generate interrupt every half m_linesCount
    if ((scan == 0 && (visibleRow % (m_linesCount / 2)) == 0)) {
      if (visibleRow == 0)
        s_frameResetDesc = buffer;
      buffer->eof = 1;
    }
  }
#endif /* !USERSPACE */
}


void VGADirectController::setScanlineBuffer(int scanline, uint8_t * lineBuffer)
{
#ifndef USERSPACE
  s_DMALines[scanline]->buf = lineBuffer;
#endif /* !USERSPACE */
}


uint8_t * VGADirectController::getScanlineBuffer(int scanline)
{
#ifndef USERSPACE
  return s_DMALines[scanline]->buf;
#endif /* !USERSPACE */
  abort();
}


uint8_t * VGADirectController::getDefaultScanlineBuffer(int scanline)
{
  return m_lines[scanline % m_linesCount];
}


void VGADirectController::setPixelAt(PixelDesc const & pixelDesc, Rect & updateRect)
{
}


void VGADirectController::absDrawLine(int X1, int Y1, int X2, int Y2, RGB888 color)
{
}


void VGADirectController::rawFillRow(int y, int x1, int x2, RGB888 color)
{
}


void VGADirectController::rawFillRow(int y, int x1, int x2, uint8_t colorIndex)
{
}


void VGADirectController::rawInvertRow(int y, int x1, int x2)
{
}


void VGADirectController::rawCopyRow(int x1, int x2, int srcY, int dstY)
{
}


void VGADirectController::swapRows(int yA, int yB, int x1, int x2)
{
}


void VGADirectController::drawEllipse(Size const & size, Rect & updateRect)
{
}


void VGADirectController::clear(Rect & updateRect)
{
}


void VGADirectController::VScroll(int scroll, Rect & updateRect)
{
}


void VGADirectController::HScroll(int scroll, Rect & updateRect)
{
}


void VGADirectController::drawGlyph(Glyph const & glyph, GlyphOptions glyphOptions, RGB888 penColor, RGB888 brushColor, Rect & updateRect)
{
}


void VGADirectController::invertRect(Rect const & rect, Rect & updateRect)
{
}


void VGADirectController::swapFGBG(Rect const & rect, Rect & updateRect)
{
}


void VGADirectController::copyRect(Rect const & source, Rect & updateRect)
{
}


void VGADirectController::readScreen(Rect const & rect, RGB888 * destBuf)
{
  for (int y = rect.Y1; y <= rect.Y2; ++y) {
    m_drawScanlineCallback(m_drawScanlineArg, m_lines[0], y);
    for (int x = rect.X1; x <= rect.X2; ++x, ++destBuf) {
      uint8_t rawpix = VGA_PIXELINROW(m_lines[0], x);
      *destBuf = RGB888((rawpix & 3) * 85, ((rawpix >> 2) & 3) * 85, ((rawpix >> 4) & 3) * 85);
    }
  }
}


void VGADirectController::rawDrawBitmap_Native(int destX, int destY, Bitmap const * bitmap, int X1, int Y1, int XCount, int YCount)
{
}


void VGADirectController::rawDrawBitmap_Mask(int destX, int destY, Bitmap const * bitmap, void * saveBackground, int X1, int Y1, int XCount, int YCount)
{
}


void VGADirectController::rawDrawBitmap_RGBA2222(int destX, int destY, Bitmap const * bitmap, void * saveBackground, int X1, int Y1, int XCount, int YCount)
{
}


void VGADirectController::rawDrawBitmap_RGBA8888(int destX, int destY, Bitmap const * bitmap, void * saveBackground, int X1, int Y1, int XCount, int YCount)
{
}


void IRAM_ATTR VGADirectController::ISRHandler(void * arg)
{
#ifndef USERSPACE
  #if FABGLIB_VGAXCONTROLLER_PERFORMANCE_CHECK
  auto s1 = getCycleCount();
  #endif

  if (I2S1.int_st.out_eof) {

    auto ctrl = (VGADirectController *) arg;

    auto viewPortHeight = ctrl->m_viewPortHeight;

    auto desc = (volatile lldesc_t*) I2S1.out_eof_des_addr;

    if (desc == s_frameResetDesc) {
      s_scanLine = 0;
      s_VSync = false;
    }

    int linesCount = ctrl->m_linesCount;
    int scanLine = (s_scanLine + linesCount / 2) % viewPortHeight;

    const auto lineIndex = scanLine & (linesCount - 1);

    ctrl->m_drawScanlineCallback(ctrl->m_drawScanlineArg, (uint8_t*)(ctrl->m_lines[lineIndex]), scanLine);

    s_scanLine += linesCount / 2;

    if (s_scanLine >= viewPortHeight)
      s_VSync = true;

  }

  #if FABGLIB_VGAXCONTROLLER_PERFORMANCE_CHECK
  s_vgapalctrlcycles += getCycleCount() - s1;
  #endif

  I2S1.int_clr.val = I2S1.int_st.val;
#endif /* !USERSPACE */
}



} // end of namespace


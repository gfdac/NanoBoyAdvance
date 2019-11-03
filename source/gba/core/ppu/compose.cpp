/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#include <algorithm>

#include "ppu.hpp"

using namespace GameBoyAdvance;

auto PPU::ConvertColor(std::uint16_t color) -> std::uint32_t {
  int r = (color >>  0) & 0x1F;
  int g = (color >>  5) & 0x1F;
  int b = (color >> 10) & 0x1F;

  return r << 19 |
         g << 11 |
         b <<  3 |
         0xFF000000;
}

void PPU::InitBlendTable() {
  for (int color0 = 0; color0 <= 31; color0++) {
    for (int color1 = 0; color1 <= 31; color1++) {
      for (int factor0 = 0; factor0 <= 16; factor0++) {
        for (int factor1 = 0; factor1 <= 16; factor1++) {
          int result = (color0 * factor0 + color1 * factor1) >> 4;

          blend_table[factor0][factor1][color0][color1] = std::min<std::uint8_t>(result, 31);
        }
      }
    }
  }
}

void PPU::ComposeScanline(int bg_min, int bg_max) {
  std::uint32_t* line = &output[mmio.vcount * 240];
  std::uint16_t backdrop = ReadPalette(0, 0);
  
  for (int x = 0; x < 240; x++) {
    int priority = 4;
    std::uint16_t pixel = backdrop;
    
    /* TODO: for maximum effiency maybe we can pre-sort the BGs by priority. */
    for (int bg = bg_max; bg >= bg_min; bg--) {
      if (mmio.dispcnt.enable[bg] &&
          mmio.bgcnt[bg].priority <= priority &&
          buffer_bg[bg][x] != s_color_transparent) {
        pixel = buffer_bg[bg][x];
        priority = mmio.bgcnt[bg].priority;
      }
    }
    
    if (mmio.dispcnt.enable[4] &&
        buffer_obj[x].priority <= priority &&
        buffer_obj[x].color != s_color_transparent) {
      pixel = buffer_obj[x].color;
      priority = buffer_obj[x].priority;
    }
    
    line[x] = ConvertColor(pixel);
  }
}

void PPU::Blend(std::uint16_t& target1,
                std::uint16_t target2,
                BlendControl::Effect sfx) {
  int r1 = (target1 >>  0) & 0x1F;
  int g1 = (target1 >>  5) & 0x1F;
  int b1 = (target1 >> 10) & 0x1F;

  switch (sfx) {
    case BlendControl::Effect::SFX_BLEND: {
      int eva = std::min<int>(16, mmio.eva);
      int evb = std::min<int>(16, mmio.evb);

      int r2 = (target2 >>  0) & 0x1F;
      int g2 = (target2 >>  5) & 0x1F;
      int b2 = (target2 >> 10) & 0x1F;

      r1 = blend_table[eva][evb][r1][r2];
      g1 = blend_table[eva][evb][g1][g2];
      b1 = blend_table[eva][evb][b1][b2];
      break;
    }
    case BlendControl::Effect::SFX_BRIGHTEN: {
      int evy = std::min<int>(16, mmio.evy);

      r1 = blend_table[16 - evy][evy][r1][31];
      g1 = blend_table[16 - evy][evy][g1][31];
      b1 = blend_table[16 - evy][evy][b1][31];
      break;
    }
    case BlendControl::Effect::SFX_DARKEN: {
      int evy = std::min<int>(16, mmio.evy);

      r1 = blend_table[16 - evy][evy][r1][0];
      g1 = blend_table[16 - evy][evy][g1][0];
      b1 = blend_table[16 - evy][evy][b1][0];
      break;
    }
  }

  target1 = r1 | (g1 << 5) | (b1 << 10);
}
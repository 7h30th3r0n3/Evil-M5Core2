/*
  Keyboard_fr_FR.h

  Copyright (c) 2022, Edgar Bonet

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef KEYBOARD_FR_FR_h
#define KEYBOARD_FR_FR_h

#include "HID.h"

#if !defined(_USING_HID)

#warning "Using legacy HID core (non pluggable)"

#else

//================================================================================
//================================================================================
//  Keyboard

// fr_FR keys
#define KEY_SUPERSCRIPT_TWO (136+0x35)
#define KEY_E_ACUTE         (136+0x1f)
#define KEY_E_GRAVE         (136+0x24)
#define KEY_C_CEDILLA       (136+0x26)
#define KEY_A_GRAVE         (136+0x27)
#define KEY_CIRCUMFLEX      (136+0x2f)
#define KEY_U_GRAVE         (136+0x34)

#endif
#endif
extern const uint8_t KeyboardLayout_fr_FR[128] PROGMEM =
{
				0x00,                 // NUL
				0x00,                 // SOH
				0x00,                 // STX
				0x00,                 // ETX
				0x00,                 // EOT
				0x00,                 // ENQ
				0x00,                 // ACK
				0x00,                 // BEL
				42,                   // BS Backspace
				43,                   // TAB  Tab
				40,                   // LF Enter
				0x00,                 // VT
				0x00,                 // FF
				0x00,                 // CR
				0x00,                 // SO
				0x00,                 // SI
				0x00,                 // DEL
				0x00,                 // DC1
				0x00,                 // DC2
				0x00,                 // DC3
				0x00,                 // DC4
				0x00,                 // NAK
				0x00,                 // SYN
				0x00,                 // ETB
				0x00,                 // CAN
				0x00,                 // EM
				0x00,                 // SUB
				0x00,                 // ESC
				0x00,                 // FS
				0x00,                 // GS
				0x00,                 // RS
				0x00,                 // US
				44,                   // ' '
				56,                   // !
				32,                   // "
				32|FR_MOD_ALT_RIGHT,  // #
				48,                   // $
				52|FR_MOD_SHIFT_LEFT, // %
				30,                   // &
				33,                   // '
				34,                   // (
				45,                   // )
				85,                   // *
				46|FR_MOD_SHIFT_LEFT, // +
				16,                   // ,
				35,                   // -
				54|FR_MOD_SHIFT_LEFT, // .
				55|FR_MOD_SHIFT_LEFT, // /
				39|FR_MOD_SHIFT_LEFT, // 0
				30|FR_MOD_SHIFT_LEFT, // 1
				31|FR_MOD_SHIFT_LEFT, // 2
				32|FR_MOD_SHIFT_LEFT, // 3
				33|FR_MOD_SHIFT_LEFT, // 4
				34|FR_MOD_SHIFT_LEFT, // 5
				35|FR_MOD_SHIFT_LEFT, // 6
				36|FR_MOD_SHIFT_LEFT, // 7
				37|FR_MOD_SHIFT_LEFT, // 8
				38|FR_MOD_SHIFT_LEFT, // 9
				55,                   // :
				54,                   // ;
				100,                  // <
				46,                   // =
				100|FR_MOD_SHIFT_LEFT,// >
				16|FR_MOD_SHIFT_LEFT, // ?
				39|FR_MOD_ALT_RIGHT,  // @
				20|FR_MOD_SHIFT_LEFT, // A
				5|FR_MOD_SHIFT_LEFT,  // B
				6|FR_MOD_SHIFT_LEFT,  // C
				7|FR_MOD_SHIFT_LEFT,  // D
				8|FR_MOD_SHIFT_LEFT,  // E
				9|FR_MOD_SHIFT_LEFT,  // F
				10|FR_MOD_SHIFT_LEFT, // G
				11|FR_MOD_SHIFT_LEFT, // H
				12|FR_MOD_SHIFT_LEFT, // I
				13|FR_MOD_SHIFT_LEFT, // J
				14|FR_MOD_SHIFT_LEFT, // K
				15|FR_MOD_SHIFT_LEFT, // L
				51|FR_MOD_SHIFT_LEFT, // M
				17|FR_MOD_SHIFT_LEFT, // N
				18|FR_MOD_SHIFT_LEFT, // O
				19|FR_MOD_SHIFT_LEFT, // P
				4|FR_MOD_SHIFT_LEFT,  // Q
				21|FR_MOD_SHIFT_LEFT, // R
				22|FR_MOD_SHIFT_LEFT, // S
				23|FR_MOD_SHIFT_LEFT, // T
				24|FR_MOD_SHIFT_LEFT, // U
				25|FR_MOD_SHIFT_LEFT, // V
				29|FR_MOD_SHIFT_LEFT, // W
				27|FR_MOD_SHIFT_LEFT, // X
				28|FR_MOD_SHIFT_LEFT, // Y
				26|FR_MOD_SHIFT_LEFT, // Z
				34|FR_MOD_ALT_RIGHT,  // [
				37|FR_MOD_ALT_RIGHT,  // bslash
				45|FR_MOD_ALT_RIGHT,  // ]
				47,                   // ^
				37,                   // _
				36|FR_MOD_ALT_RIGHT,  // `
				20,                   // a
				5,                    // b
				6,                    // C
				7,                    // d
				8,                    // e
				9,                    // f
				10,                   // g
				11,                   // h
				12,                   // i
				13,                   // j
				14,                   // k
				15,                   // l
				51,                   // m
				17,                   // n
				18,                   // O
				19,                   // P
				4,                    // Q
				21,                   // r
				22,                   // s
				23,                   // t
				24,                   // u
				25,                   // v
				29,                   // w
				27,                   // x
				28,                   // y
				26,                   // z
				33|FR_MOD_ALT_RIGHT,  // {
				35|FR_MOD_ALT_RIGHT,  // |
				46|FR_MOD_ALT_RIGHT,  // }
				31|FR_MOD_ALT_RIGHT,  // ~
				0,                    // DEL
				38,                   //Ç
				24,                   //ü
				31,                   //é
				20,                   //â
				20,                   //ä
				39,                   //à
				20,                   //å
				38,                   //ç
				8,                   //ê
				8,                   //ë
				36,                   //è
};
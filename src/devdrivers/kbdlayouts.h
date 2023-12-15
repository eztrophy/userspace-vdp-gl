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


#pragma once



/**
 * @file
 *
 * @brief This file contains keyboard layouts
 */



#include <stdint.h>

#include "fabutils.h"




namespace fabgl {



/**
 * @brief Associates scancode to virtualkey.
 */
struct VirtualKeyDef {
  uint8_t      scancode;    /**< Raw scancode received from the Keyboard device */
  VirtualKey   virtualKey;  /**< Real virtualkey (non shifted) associated to the scancode */
};


/**
 * @brief Associates a virtualkey and various shift states (ctrl, alt, etc..) to another virtualkey.
 */
struct AltVirtualKeyDef {
  VirtualKey reqVirtualKey; /**< Source virtualkey translated using VirtualKeyDef. */
  struct {
    uint8_t ctrl     : 1;   /**< CTRL needs to be down. */
    uint8_t lalt     : 1;   /**< LEFT-ALT needs to be down. */
    uint8_t ralt     : 1;   /**< RIGHT-ALT needs to be down. */
    uint8_t shift    : 1;   /**< SHIFT needs to be down (OR-ed with capslock). */
  };
  VirtualKey virtualKey;    /**< Generated virtualkey. */
};


struct DeadKeyVirtualKeyDef {
  VirtualKey   deadKey;         /**< Currently pressed dead key */
  VirtualKey   reqVirtualKey;   /**< Currently pressed virtual key */
  VirtualKey   virtualKey;      /**< Virtual key result */
};


/** @brief All in one structure to fully represent a keyboard layout */
struct KeyboardLayout {
  const char *             name;                /**< Layout name. */
  const char *             desc;                /**< Layout description. */
  KeyboardLayout const *   inherited;           /**< Inherited layout. Useful to avoid to repeat the same scancode-virtualkeys associations. */
  VirtualKeyDef            scancodeToVK[86];    /**< Direct one-byte-scancode->virtualkey associations. */
  VirtualKeyDef            exScancodeToVK[22];  /**< Direct extended-scancode->virtualkey associations. Extended scancodes begin with 0xE0. */
  AltVirtualKeyDef         alternateVK[73];     /**< Virtualkeys generated by other virtualkeys and shift combinations. */

  VirtualKey               deadKeysVK[8];       /**< Dead keys identifiers. */
  DeadKeyVirtualKeyDef     deadkeysToVK[60];    /**< Translation dead key + virtual key = replaced virtual key */
};



/** @brief Predefined US layout. Often used as inherited layout for other layouts. */
extern const KeyboardLayout USLayout;

/** @brief UK keyboard layout */
extern const KeyboardLayout UKLayout;

/** @brief German keyboard layout */
extern const KeyboardLayout GermanLayout;

/** @brief Italian keyboard layout */
extern const KeyboardLayout ItalianLayout;

/** @brief Spanish keyboard layout */
extern const KeyboardLayout SpanishLayout;

/** @brief French keyboard layout */
extern const KeyboardLayout FrenchLayout;

/** @brief Belgian keyboard layout */
extern const KeyboardLayout BelgianLayout;

/** @brief Norwegian keyboard layout */
extern const KeyboardLayout NorwegianLayout;

/** @brief Japanese keyboard layout */
extern const KeyboardLayout JapaneseLayout;

/** @brief US International keyboard layout */
extern const KeyboardLayout USInternationalLayout;

/** @brief US International Alt-Gr dead keys keyboard layout */
extern const KeyboardLayout USInternationalAltLayout;

/** @brief Swiss German keyboard layout */
extern const KeyboardLayout SwissGLayout;
  
/** @brief Swiss French keyboard layout */
extern const KeyboardLayout SwissFLayout;
  
/** @brief Danish keyboard layout */
extern const KeyboardLayout DanishLayout;
  
/** @brief Swedish keyboard layout */
extern const KeyboardLayout SwedishLayout;
  
/** @brief Portuguese keyboard layout */
extern const KeyboardLayout PortugueseLayout;

/** @brief Brazilian Portuguese keyboard layout */
extern const KeyboardLayout BrazilianPortugueseLayout;
  
/** @brief Dvorak keyboard layout */
extern const KeyboardLayout DvorakLayout;
  


struct SupportedLayouts {

  static constexpr int LAYOUTSCOUNT = 18;

  static int count()               { return LAYOUTSCOUNT; }

  static char const * * names() {
    static char const * NAMES[LAYOUTSCOUNT] =  {
        GermanLayout.desc,
        ItalianLayout.desc,
        UKLayout.desc,
        USLayout.desc,
        SpanishLayout.desc,
        FrenchLayout.desc,
        BelgianLayout.desc,
        NorwegianLayout.desc,
        JapaneseLayout.desc,
	USInternationalLayout.desc,
	USInternationalAltLayout.desc,
	SwissGLayout.desc,
	SwissFLayout.desc,
	DanishLayout.desc,
	SwedishLayout.desc,
	PortugueseLayout.desc,
	BrazilianPortugueseLayout.desc,
  DvorakLayout.desc,
    };
    return NAMES;
  }

  static char const * * shortNames() {
    static char const * SNAMES[LAYOUTSCOUNT] = {
        GermanLayout.name,
        ItalianLayout.name,
        UKLayout.name,
        USLayout.name,
        SpanishLayout.name,
        FrenchLayout.name,
        BelgianLayout.name,
        NorwegianLayout.name,
        JapaneseLayout.name,
	USInternationalLayout.name,
	USInternationalAltLayout.name,
	SwissGLayout.name,
	SwissFLayout.name,
	DanishLayout.name,
	SwedishLayout.name,
	PortugueseLayout.name,
	BrazilianPortugueseLayout.name,
  DvorakLayout.name,
    };
    return SNAMES;
  }

  static const KeyboardLayout * * layouts() {
    static KeyboardLayout const * LAYOUTS[LAYOUTSCOUNT] = {
        &GermanLayout,
        &ItalianLayout,
        &UKLayout,
        &USLayout,
        &SpanishLayout,
        &FrenchLayout,
        &BelgianLayout,
        &NorwegianLayout,
        &JapaneseLayout,
	&USInternationalLayout,
	&USInternationalAltLayout,
	&SwissGLayout,
	&SwissFLayout,
	&DanishLayout,
	&SwedishLayout,
	&PortugueseLayout,
	&BrazilianPortugueseLayout,
  &DvorakLayout,
    };
    return LAYOUTS;
  }

};



} // end of namespace

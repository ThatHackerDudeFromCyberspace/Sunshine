/**
 * @file src/platform/linux/input/inputtino_keyboard.cpp
 * @brief Definitions for inputtino keyboard input handling.
 */
// lib includes
#include <boost/locale.hpp>
#include <inputtino/input.hpp>
#include <libevdev/libevdev.h>

// local includes
#include "inputtino_common.h"
#include "inputtino_keyboard.h"
#include "src/config.h"
#include "src/logging.h"
#include "src/platform/common.h"
#include "src/utility.h"

#include <X11/extensions/XTest.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>

using namespace std::literals;

namespace platf::keyboard {


  struct keycode_t {
    std::uint32_t keycode;
    std::uint32_t scancode;
  
  #ifdef SUNSHINE_BUILD_X11
    KeySym keysym;
  #endif
  };
  
  constexpr auto UNKNOWN = 0;

  /**
 * @brief Initializes the keycode constants for translating
 *        moonlight keycodes to linux/X11 keycodes
 */
static constexpr std::array<keycode_t, 0xE3> init_keycodes() {
  std::array<keycode_t, 0xE3> keycodes {};

#ifdef SUNSHINE_BUILD_X11
#define __CONVERT_UNSAFE(wincode, linuxcode, scancode, keysym) \
  keycodes[wincode] = keycode_t { linuxcode, scancode, keysym };
#else
#define __CONVERT_UNSAFE(wincode, linuxcode, scancode, keysym) \
  keycodes[wincode] = keycode_t { linuxcode, scancode };
#endif

#define __CONVERT(wincode, linuxcode, scancode, keysym)                               \
  static_assert(wincode < keycodes.size(), "Keycode doesn't fit into keycode array"); \
  static_assert(wincode >= 0, "Are you mad?, keycode needs to be greater than zero"); \
  __CONVERT_UNSAFE(wincode, linuxcode, scancode, keysym)

  __CONVERT(0x08 /* VKEY_BACK */, KEY_BACKSPACE, 0x7002A, XK_BackSpace);
  __CONVERT(0x09 /* VKEY_TAB */, KEY_TAB, 0x7002B, XK_Tab);
  __CONVERT(0x0C /* VKEY_CLEAR */, KEY_CLEAR, UNKNOWN, XK_Clear);
  __CONVERT(0x0D /* VKEY_RETURN */, KEY_ENTER, 0x70028, XK_Return);
  __CONVERT(0x10 /* VKEY_SHIFT */, KEY_LEFTSHIFT, 0x700E1, XK_Shift_L);
  __CONVERT(0x11 /* VKEY_CONTROL */, KEY_LEFTCTRL, 0x700E0, XK_Control_L);
  __CONVERT(0x12 /* VKEY_MENU */, KEY_LEFTALT, UNKNOWN, XK_Alt_L);
  __CONVERT(0x13 /* VKEY_PAUSE */, KEY_PAUSE, UNKNOWN, XK_Pause);
  __CONVERT(0x14 /* VKEY_CAPITAL */, KEY_CAPSLOCK, 0x70039, XK_Caps_Lock);
  __CONVERT(0x15 /* VKEY_KANA */, KEY_KATAKANAHIRAGANA, UNKNOWN, XK_Kana_Shift);
  __CONVERT(0x16 /* VKEY_HANGUL */, KEY_HANGEUL, UNKNOWN, XK_Hangul);
  __CONVERT(0x17 /* VKEY_JUNJA */, KEY_HANJA, UNKNOWN, XK_Hangul_Jeonja);
  __CONVERT(0x19 /* VKEY_KANJI */, KEY_KATAKANA, UNKNOWN, XK_Kanji);
  __CONVERT(0x1B /* VKEY_ESCAPE */, KEY_ESC, 0x70029, XK_Escape);
  __CONVERT(0x20 /* VKEY_SPACE */, KEY_SPACE, 0x7002C, XK_space);
  __CONVERT(0x21 /* VKEY_PRIOR */, KEY_PAGEUP, 0x7004B, XK_Page_Up);
  __CONVERT(0x22 /* VKEY_NEXT */, KEY_PAGEDOWN, 0x7004E, XK_Page_Down);
  __CONVERT(0x23 /* VKEY_END */, KEY_END, 0x7004D, XK_End);
  __CONVERT(0x24 /* VKEY_HOME */, KEY_HOME, 0x7004A, XK_Home);
  __CONVERT(0x25 /* VKEY_LEFT */, KEY_LEFT, 0x70050, XK_Left);
  __CONVERT(0x26 /* VKEY_UP */, KEY_UP, 0x70052, XK_Up);
  __CONVERT(0x27 /* VKEY_RIGHT */, KEY_RIGHT, 0x7004F, XK_Right);
  __CONVERT(0x28 /* VKEY_DOWN */, KEY_DOWN, 0x70051, XK_Down);
  __CONVERT(0x29 /* VKEY_SELECT */, KEY_SELECT, UNKNOWN, XK_Select);
  __CONVERT(0x2A /* VKEY_PRINT */, KEY_PRINT, UNKNOWN, XK_Print);
  __CONVERT(0x2C /* VKEY_SNAPSHOT */, KEY_SYSRQ, 0x70046, XK_Sys_Req);
  __CONVERT(0x2D /* VKEY_INSERT */, KEY_INSERT, 0x70049, XK_Insert);
  __CONVERT(0x2E /* VKEY_DELETE */, KEY_DELETE, 0x7004C, XK_Delete);
  __CONVERT(0x2F /* VKEY_HELP */, KEY_HELP, UNKNOWN, XK_Help);
  __CONVERT(0x30 /* VKEY_0 */, KEY_0, 0x70027, XK_0);
  __CONVERT(0x31 /* VKEY_1 */, KEY_1, 0x7001E, XK_1);
  __CONVERT(0x32 /* VKEY_2 */, KEY_2, 0x7001F, XK_2);
  __CONVERT(0x33 /* VKEY_3 */, KEY_3, 0x70020, XK_3);
  __CONVERT(0x34 /* VKEY_4 */, KEY_4, 0x70021, XK_4);
  __CONVERT(0x35 /* VKEY_5 */, KEY_5, 0x70022, XK_5);
  __CONVERT(0x36 /* VKEY_6 */, KEY_6, 0x70023, XK_6);
  __CONVERT(0x37 /* VKEY_7 */, KEY_7, 0x70024, XK_7);
  __CONVERT(0x38 /* VKEY_8 */, KEY_8, 0x70025, XK_8);
  __CONVERT(0x39 /* VKEY_9 */, KEY_9, 0x70026, XK_9);
  __CONVERT(0x41 /* VKEY_A */, KEY_A, 0x70004, XK_A);
  __CONVERT(0x42 /* VKEY_B */, KEY_B, 0x70005, XK_B);
  __CONVERT(0x43 /* VKEY_C */, KEY_C, 0x70006, XK_C);
  __CONVERT(0x44 /* VKEY_D */, KEY_D, 0x70007, XK_D);
  __CONVERT(0x45 /* VKEY_E */, KEY_E, 0x70008, XK_E);
  __CONVERT(0x46 /* VKEY_F */, KEY_F, 0x70009, XK_F);
  __CONVERT(0x47 /* VKEY_G */, KEY_G, 0x7000A, XK_G);
  __CONVERT(0x48 /* VKEY_H */, KEY_H, 0x7000B, XK_H);
  __CONVERT(0x49 /* VKEY_I */, KEY_I, 0x7000C, XK_I);
  __CONVERT(0x4A /* VKEY_J */, KEY_J, 0x7000D, XK_J);
  __CONVERT(0x4B /* VKEY_K */, KEY_K, 0x7000E, XK_K);
  __CONVERT(0x4C /* VKEY_L */, KEY_L, 0x7000F, XK_L);
  __CONVERT(0x4D /* VKEY_M */, KEY_M, 0x70010, XK_M);
  __CONVERT(0x4E /* VKEY_N */, KEY_N, 0x70011, XK_N);
  __CONVERT(0x4F /* VKEY_O */, KEY_O, 0x70012, XK_O);
  __CONVERT(0x50 /* VKEY_P */, KEY_P, 0x70013, XK_P);
  __CONVERT(0x51 /* VKEY_Q */, KEY_Q, 0x70014, XK_Q);
  __CONVERT(0x52 /* VKEY_R */, KEY_R, 0x70015, XK_R);
  __CONVERT(0x53 /* VKEY_S */, KEY_S, 0x70016, XK_S);
  __CONVERT(0x54 /* VKEY_T */, KEY_T, 0x70017, XK_T);
  __CONVERT(0x55 /* VKEY_U */, KEY_U, 0x70018, XK_U);
  __CONVERT(0x56 /* VKEY_V */, KEY_V, 0x70019, XK_V);
  __CONVERT(0x57 /* VKEY_W */, KEY_W, 0x7001A, XK_W);
  __CONVERT(0x58 /* VKEY_X */, KEY_X, 0x7001B, XK_X);
  __CONVERT(0x59 /* VKEY_Y */, KEY_Y, 0x7001C, XK_Y);
  __CONVERT(0x5A /* VKEY_Z */, KEY_Z, 0x7001D, XK_Z);
  __CONVERT(0x5B /* VKEY_LWIN */, KEY_LEFTMETA, 0x700E3, XK_Meta_L);
  __CONVERT(0x5C /* VKEY_RWIN */, KEY_RIGHTMETA, 0x700E7, XK_Meta_R);
  __CONVERT(0x5F /* VKEY_SLEEP */, KEY_SLEEP, UNKNOWN, UNKNOWN);
  __CONVERT(0x60 /* VKEY_NUMPAD0 */, KEY_KP0, 0x70062, XK_KP_0);
  __CONVERT(0x61 /* VKEY_NUMPAD1 */, KEY_KP1, 0x70059, XK_KP_1);
  __CONVERT(0x62 /* VKEY_NUMPAD2 */, KEY_KP2, 0x7005A, XK_KP_2);
  __CONVERT(0x63 /* VKEY_NUMPAD3 */, KEY_KP3, 0x7005B, XK_KP_3);
  __CONVERT(0x64 /* VKEY_NUMPAD4 */, KEY_KP4, 0x7005C, XK_KP_4);
  __CONVERT(0x65 /* VKEY_NUMPAD5 */, KEY_KP5, 0x7005D, XK_KP_5);
  __CONVERT(0x66 /* VKEY_NUMPAD6 */, KEY_KP6, 0x7005E, XK_KP_6);
  __CONVERT(0x67 /* VKEY_NUMPAD7 */, KEY_KP7, 0x7005F, XK_KP_7);
  __CONVERT(0x68 /* VKEY_NUMPAD8 */, KEY_KP8, 0x70060, XK_KP_8);
  __CONVERT(0x69 /* VKEY_NUMPAD9 */, KEY_KP9, 0x70061, XK_KP_9);
  __CONVERT(0x6A /* VKEY_MULTIPLY */, KEY_KPASTERISK, 0x70055, XK_KP_Multiply);
  __CONVERT(0x6B /* VKEY_ADD */, KEY_KPPLUS, 0x70057, XK_KP_Add);
  __CONVERT(0x6C /* VKEY_SEPARATOR */, KEY_KPCOMMA, UNKNOWN, XK_KP_Separator);
  __CONVERT(0x6D /* VKEY_SUBTRACT */, KEY_KPMINUS, 0x70056, XK_KP_Subtract);
  __CONVERT(0x6E /* VKEY_DECIMAL */, KEY_KPDOT, 0x70063, XK_KP_Decimal);
  __CONVERT(0x6F /* VKEY_DIVIDE */, KEY_KPSLASH, 0x70054, XK_KP_Divide);
  __CONVERT(0x70 /* VKEY_F1 */, KEY_F1, 0x70046, XK_F1);
  __CONVERT(0x71 /* VKEY_F2 */, KEY_F2, 0x70047, XK_F2);
  __CONVERT(0x72 /* VKEY_F3 */, KEY_F3, 0x70048, XK_F3);
  __CONVERT(0x73 /* VKEY_F4 */, KEY_F4, 0x70049, XK_F4);
  __CONVERT(0x74 /* VKEY_F5 */, KEY_F5, 0x7004a, XK_F5);
  __CONVERT(0x75 /* VKEY_F6 */, KEY_F6, 0x7004b, XK_F6);
  __CONVERT(0x76 /* VKEY_F7 */, KEY_F7, 0x7004c, XK_F7);
  __CONVERT(0x77 /* VKEY_F8 */, KEY_F8, 0x7004d, XK_F8);
  __CONVERT(0x78 /* VKEY_F9 */, KEY_F9, 0x7004e, XK_F9);
  __CONVERT(0x79 /* VKEY_F10 */, KEY_F10, 0x70044, XK_F10);
  __CONVERT(0x7A /* VKEY_F11 */, KEY_F11, 0x70044, XK_F11);
  __CONVERT(0x7B /* VKEY_F12 */, KEY_F12, 0x70045, XK_F12);
  __CONVERT(0x7C /* VKEY_F13 */, KEY_F13, 0x7003a, XK_F13);
  __CONVERT(0x7D /* VKEY_F14 */, KEY_F14, 0x7003b, XK_F14);
  __CONVERT(0x7E /* VKEY_F15 */, KEY_F15, 0x7003c, XK_F15);
  __CONVERT(0x7F /* VKEY_F16 */, KEY_F16, 0x7003d, XK_F16);
  __CONVERT(0x80 /* VKEY_F17 */, KEY_F17, 0x7003e, XK_F17);
  __CONVERT(0x81 /* VKEY_F18 */, KEY_F18, 0x7003f, XK_F18);
  __CONVERT(0x82 /* VKEY_F19 */, KEY_F19, 0x70040, XK_F19);
  __CONVERT(0x83 /* VKEY_F20 */, KEY_F20, 0x70041, XK_F20);
  __CONVERT(0x84 /* VKEY_F21 */, KEY_F21, 0x70042, XK_F21);
  __CONVERT(0x85 /* VKEY_F22 */, KEY_F12, 0x70043, XK_F12);
  __CONVERT(0x86 /* VKEY_F23 */, KEY_F23, 0x70044, XK_F23);
  __CONVERT(0x87 /* VKEY_F24 */, KEY_F24, 0x70045, XK_F24);
  __CONVERT(0x90 /* VKEY_NUMLOCK */, KEY_NUMLOCK, 0x70053, XK_Num_Lock);
  __CONVERT(0x91 /* VKEY_SCROLL */, KEY_SCROLLLOCK, 0x70047, XK_Scroll_Lock);
  __CONVERT(0xA0 /* VKEY_LSHIFT */, KEY_LEFTSHIFT, 0x700E1, XK_Shift_L);
  __CONVERT(0xA1 /* VKEY_RSHIFT */, KEY_RIGHTSHIFT, 0x700E5, XK_Shift_R);
  __CONVERT(0xA2 /* VKEY_LCONTROL */, KEY_LEFTCTRL, 0x700E0, XK_Control_L);
  __CONVERT(0xA3 /* VKEY_RCONTROL */, KEY_RIGHTCTRL, 0x700E4, XK_Control_R);
  __CONVERT(0xA4 /* VKEY_LMENU */, KEY_LEFTALT, 0x7002E, XK_Alt_L);
  __CONVERT(0xA5 /* VKEY_RMENU */, KEY_RIGHTALT, 0x700E6, XK_Alt_R);
  __CONVERT(0xBA /* VKEY_OEM_1 */, KEY_SEMICOLON, 0x70033, XK_semicolon);
  __CONVERT(0xBB /* VKEY_OEM_PLUS */, KEY_EQUAL, 0x7002E, XK_equal);
  __CONVERT(0xBC /* VKEY_OEM_COMMA */, KEY_COMMA, 0x70036, XK_comma);
  __CONVERT(0xBD /* VKEY_OEM_MINUS */, KEY_MINUS, 0x7002D, XK_minus);
  __CONVERT(0xBE /* VKEY_OEM_PERIOD */, KEY_DOT, 0x70037, XK_period);
  __CONVERT(0xBF /* VKEY_OEM_2 */, KEY_SLASH, 0x70038, XK_slash);
  __CONVERT(0xC0 /* VKEY_OEM_3 */, KEY_GRAVE, 0x70035, XK_grave);
  __CONVERT(0xDB /* VKEY_OEM_4 */, KEY_LEFTBRACE, 0x7002F, XK_braceleft);
  __CONVERT(0xDC /* VKEY_OEM_5 */, KEY_BACKSLASH, 0x70031, XK_backslash);
  __CONVERT(0xDD /* VKEY_OEM_6 */, KEY_RIGHTBRACE, 0x70030, XK_braceright);
  __CONVERT(0xDE /* VKEY_OEM_7 */, KEY_APOSTROPHE, 0x70034, XK_apostrophe);
  __CONVERT(0xE2 /* VKEY_NON_US_BACKSLASH */, KEY_102ND, 0x70064, XK_backslash);
#undef __CONVERT
#undef __CONVERT_UNSAFE

  return keycodes;
}

static constexpr auto keycodes = init_keycodes();

  /**
   * Takes an UTF-32 encoded string and returns a hex string representation of the bytes (uppercase)
   *
   * ex: ['👱'] = "1F471" // see UTF encoding at https://www.compart.com/en/unicode/U+1F471
   *
   * adapted from: https://stackoverflow.com/a/7639754
   */
  std::string to_hex(const std::basic_string<char32_t> &str) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto &ch : str) {
      ss << static_cast<uint32_t>(ch);
    }

    std::string hex_unicode(ss.str());
    std::ranges::transform(hex_unicode, hex_unicode.begin(), ::toupper);
    return hex_unicode;
  }

  /**
   * A map of linux scan code -> Moonlight keyboard code
   */
  static const std::map<short, short> key_mappings = {
    {KEY_BACKSPACE, 0x08},
    {KEY_TAB, 0x09},
    {KEY_ENTER, 0x0D},
    {KEY_LEFTSHIFT, 0x10},
    {KEY_LEFTCTRL, 0x11},
    {KEY_CAPSLOCK, 0x14},
    {KEY_ESC, 0x1B},
    {KEY_SPACE, 0x20},
    {KEY_PAGEUP, 0x21},
    {KEY_PAGEDOWN, 0x22},
    {KEY_END, 0x23},
    {KEY_HOME, 0x24},
    {KEY_LEFT, 0x25},
    {KEY_UP, 0x26},
    {KEY_RIGHT, 0x27},
    {KEY_DOWN, 0x28},
    {KEY_SYSRQ, 0x2C},
    {KEY_INSERT, 0x2D},
    {KEY_DELETE, 0x2E},
    {KEY_0, 0x30},
    {KEY_1, 0x31},
    {KEY_2, 0x32},
    {KEY_3, 0x33},
    {KEY_4, 0x34},
    {KEY_5, 0x35},
    {KEY_6, 0x36},
    {KEY_7, 0x37},
    {KEY_8, 0x38},
    {KEY_9, 0x39},
    {KEY_A, 0x41},
    {KEY_B, 0x42},
    {KEY_C, 0x43},
    {KEY_D, 0x44},
    {KEY_E, 0x45},
    {KEY_F, 0x46},
    {KEY_G, 0x47},
    {KEY_H, 0x48},
    {KEY_I, 0x49},
    {KEY_J, 0x4A},
    {KEY_K, 0x4B},
    {KEY_L, 0x4C},
    {KEY_M, 0x4D},
    {KEY_N, 0x4E},
    {KEY_O, 0x4F},
    {KEY_P, 0x50},
    {KEY_Q, 0x51},
    {KEY_R, 0x52},
    {KEY_S, 0x53},
    {KEY_T, 0x54},
    {KEY_U, 0x55},
    {KEY_V, 0x56},
    {KEY_W, 0x57},
    {KEY_X, 0x58},
    {KEY_Y, 0x59},
    {KEY_Z, 0x5A},
    {KEY_LEFTMETA, 0x5B},
    {KEY_RIGHTMETA, 0x5C},
    {KEY_KP0, 0x60},
    {KEY_KP1, 0x61},
    {KEY_KP2, 0x62},
    {KEY_KP3, 0x63},
    {KEY_KP4, 0x64},
    {KEY_KP5, 0x65},
    {KEY_KP6, 0x66},
    {KEY_KP7, 0x67},
    {KEY_KP8, 0x68},
    {KEY_KP9, 0x69},
    {KEY_KPASTERISK, 0x6A},
    {KEY_KPPLUS, 0x6B},
    {KEY_KPMINUS, 0x6D},
    {KEY_KPDOT, 0x6E},
    {KEY_KPSLASH, 0x6F},
    {KEY_F1, 0x70},
    {KEY_F2, 0x71},
    {KEY_F3, 0x72},
    {KEY_F4, 0x73},
    {KEY_F5, 0x74},
    {KEY_F6, 0x75},
    {KEY_F7, 0x76},
    {KEY_F8, 0x77},
    {KEY_F9, 0x78},
    {KEY_F10, 0x79},
    {KEY_F11, 0x7A},
    {KEY_F12, 0x7B},
    {KEY_NUMLOCK, 0x90},
    {KEY_SCROLLLOCK, 0x91},
    {KEY_LEFTSHIFT, 0xA0},
    {KEY_RIGHTSHIFT, 0xA1},
    {KEY_LEFTCTRL, 0xA2},
    {KEY_RIGHTCTRL, 0xA3},
    {KEY_LEFTALT, 0xA4},
    {KEY_RIGHTALT, 0xA5},
    {KEY_SEMICOLON, 0xBA},
    {KEY_EQUAL, 0xBB},
    {KEY_COMMA, 0xBC},
    {KEY_MINUS, 0xBD},
    {KEY_DOT, 0xBE},
    {KEY_SLASH, 0xBF},
    {KEY_GRAVE, 0xC0},
    {KEY_LEFTBRACE, 0xDB},
    {KEY_BACKSLASH, 0xDC},
    {KEY_RIGHTBRACE, 0xDD},
    {KEY_APOSTROPHE, 0xDE},
    {KEY_102ND, 0xE2}
  };

  void update(input_raw_t *raw, uint16_t modcode, bool release, uint8_t flags) {
    if (raw->XDisplay) {
      if(modcode > keycodes.size()) {
        return;
      }

      if(keycodes[modcode].keysym == UNKNOWN) {
        return;
      }

      const auto keycode_x = XKeysymToKeycode(raw->XDisplay, keycodes[modcode].keysym);
      if(keycode_x == 0) {
        return;
      }

      XTestFakeKeyEvent(raw->XDisplay, keycode_x, !release, CurrentTime);
  
      XFlush(raw->XDisplay);
    }
  }

  void unicode(input_raw_t *raw, char *utf8, int size) {
    if (raw->keyboard) {
      /* Reading input text as UTF-8 */
      auto utf8_str = boost::locale::conv::to_utf<wchar_t>(utf8, utf8 + size, "UTF-8");
      /* Converting to UTF-32 */
      auto utf32_str = boost::locale::conv::utf_to_utf<char32_t>(utf8_str);
      /* To HEX string */
      auto hex_unicode = to_hex(utf32_str);
      BOOST_LOG(debug) << "Unicode, typing U+"sv << hex_unicode;

      /* pressing <CTRL> + <SHIFT> + U */
      (*raw->keyboard).press(0xA2);  // LEFTCTRL
      (*raw->keyboard).press(0xA0);  // LEFTSHIFT
      (*raw->keyboard).press(0x55);  // U
      (*raw->keyboard).release(0x55);  // U

      /* input each HEX character */
      for (auto &ch : hex_unicode) {
        auto key_str = "KEY_"s + ch;
        auto keycode = libevdev_event_code_from_name(EV_KEY, key_str.c_str());
        auto wincode = key_mappings.find(keycode);
        if (keycode == -1 || wincode == key_mappings.end()) {
          BOOST_LOG(warning) << "Unicode, unable to find keycode for: "sv << ch;
        } else {
          (*raw->keyboard).press(wincode->second);
          (*raw->keyboard).release(wincode->second);
        }
      }

      /* releasing <SHIFT> and <CTRL> */
      (*raw->keyboard).release(0xA0);  // LEFTSHIFT
      (*raw->keyboard).release(0xA2);  // LEFTCTRL
    }
  }
}  // namespace platf::keyboard

/**
 * @file src/platform/linux/input/inputtino_mouse.cpp
 * @brief Definitions for inputtino mouse input handling.
 */
// lib includes
#include <X11/Xutil.h>
#include <boost/locale.hpp>
#include <cmath>
#include <inputtino/input.hpp>
#include <libevdev/libevdev.h>

// local includes
#include "inputtino_common.h"
#include "inputtino_mouse.h"
#include "src/config.h"
#include "src/logging.h"
#include "src/platform/common.h"
#include "src/utility.h"
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>

#define Button6 6
#define Button7 7

using namespace std::literals;

namespace platf::mouse {

  void move(input_raw_t *raw, int deltaX, int deltaY) {
    if (raw->XDisplay) {
      XTestFakeRelativeMotionEvent(raw->XDisplay, deltaX, deltaY, CurrentTime);
      XFlush(raw->XDisplay);
    }
  }

  void move_abs(input_raw_t *raw, const touch_port_t &touch_port, float x, float y) {
    if (raw->XDisplay) {
      XTestFakeMotionEvent(raw->XDisplay, -1, static_cast<int>(std::round(x)), static_cast<int>(std::round(y)), CurrentTime);
      XFlush(raw->XDisplay);
    }
  }

  void button(input_raw_t *raw, int button, bool release) {
    unsigned int XButtonType;

    switch (button) {
      case BUTTON_LEFT:
        XButtonType = Button1;
        break;
      case BUTTON_MIDDLE:
        XButtonType = Button2;
        break;
      case BUTTON_RIGHT:
        XButtonType = Button3;
        break;
      default:
        BOOST_LOG(warning) << "Unknown mouse button: " << button;
        return;
    }

    if (raw->XDisplay) {
      XTestFakeButtonEvent(raw->XDisplay, XButtonType, !release, CurrentTime);
      XFlush(raw->XDisplay);
    }
  }

  /**
  * @brief XTest mouse scroll.
  * @param input The input_t instance to use.
  * @param distance How far to scroll
  * @param button_pos Which mouse button to emulate for positive scroll.
  * @param button_neg Which mouse button to emulate for negative scroll.
  *
  * EXAMPLES:
  * ```cpp
  * x_scroll(input, 10, 4, 5);
  * ```
  */
  static void x_scroll(input_raw_t *raw, int distance, int button_pos, int button_neg) {
    Display *xdisplay = raw->XDisplay;
    if(!xdisplay) {
      return;
    }
  
    const int button = distance > 0 ? button_pos : button_neg;
    for(int i = 0; i < abs(distance); i++) {
      XTestFakeButtonEvent(xdisplay, button, true, CurrentTime);
      XTestFakeButtonEvent(xdisplay, button, false, CurrentTime);
    }
    XFlush(xdisplay);
  }

  void scroll(input_raw_t *raw, int high_res_distance) {
    x_scroll(raw, high_res_distance / 60, 4, 5);
  }
  
  void hscroll(input_raw_t *raw, int high_res_distance) {
    x_scroll(raw, high_res_distance / 60, 6, 7);
  }

  util::point_t get_location(input_raw_t *raw) {
    if (raw->mouse) {
      // TODO: decide what to do after https://github.com/games-on-whales/inputtino/issues/6 is resolved.
      // TODO: auto x = (*raw->mouse).get_absolute_x();
      // TODO: auto y = (*raw->mouse).get_absolute_y();
      return {0, 0};
    }
    return {0, 0};
  }
}  // namespace platf::mouse

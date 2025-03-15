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

using namespace std::literals;

namespace platf::mouse {

  void move(input_raw_t *raw, int deltaX, int deltaY) {
    if (raw->XDisplay) {
      XTestFakeRelativeMotionEvent(raw->XDisplay, deltaX, deltaY, 0);
    }
  }

  void move_abs(input_raw_t *raw, const touch_port_t &touch_port, float x, float y) {
    /*if (raw->mouse) {
      (*raw->mouse).move_abs(x, y, touch_port.width, touch_port.height);
    }*/
    if (raw->XDisplay) {
      XTestFakeMotionEvent(raw->XDisplay, -1, std::round(x), std::round(y), 0);
    }
  }

  void button(input_raw_t *raw, int button, bool release) {
    unsigned int XButtonType;

    switch (button) {
      case BUTTON_LEFT:
        XButtonType = XK_Pointer_Button1;
        break;
      case BUTTON_MIDDLE:
        XButtonType = XK_Pointer_Button2;
        break;
      case BUTTON_RIGHT:
        XButtonType = XK_Pointer_Button3;
        break;
      default:
        BOOST_LOG(warning) << "Unknown mouse button: " << button;
        return;
    }

    if (raw->XDisplay) {
      XTestFakeButtonEvent(raw->XDisplay, XButtonType, !release, 0);
    }
  }

  void scroll(input_raw_t *raw, int high_res_distance) {
    if (raw->XDisplay) {
      if (high_res_distance > 0) {
        XTestFakeButtonEvent(raw->XDisplay, XK_Pointer_Button5, true, 0);
        XTestFakeButtonEvent(raw->XDisplay, XK_Pointer_Button5, false, 20);
      } else {
        XTestFakeButtonEvent(raw->XDisplay, XK_Pointer_Button4, true, 0);
        XTestFakeButtonEvent(raw->XDisplay, XK_Pointer_Button4, false, 20);
      }
    }
  }

  void hscroll(input_raw_t *raw, int high_res_distance) {
    /*if (raw->XDisplay) {
      if (high_res_distance > 0) {
        XTestFakeButtonEvent(raw->XDisplay, XK_Pointer_Button6, true, 0);
        XTestFakeButtonEvent(raw->XDisplay, XK_Pointer_Button6, false, 20);
      } else {
        XTestFakeButtonEvent(raw->XDisplay, XK_Pointer_Button7, true, 0);
        XTestFakeButtonEvent(raw->XDisplay, XK_Pointer_Button7, false, 20);
      }
    }*/
    return; // @TODO: Figure out horizontal scrolling
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

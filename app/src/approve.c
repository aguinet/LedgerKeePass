#include "os.h"
#include "menu.h"
#include "approve.h"

char ApproveLine1[32] = {0}; 
char ApproveLine2[32] = {0};

static const bagl_element_t bagl_ui_approval_nanos[] = {
  {
    {BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000,
      0xFFFFFF, 0, 0},
    NULL,
  },
  {
    {BAGL_LABELINE, 0x01, 0, 12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
    ApproveLine1,
  },
  {
    {BAGL_LABELINE, 0x02, 0, 26, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
    ApproveLine2,
  },
  {
    {BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CROSS},
    NULL,
  },
  {
    {BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CHECK},
    NULL,
  },
};

static approve_callback_ty approve_cb_;

static void touch_deny()
{
  G_io_apdu_buffer[0] = 0x69;
  G_io_apdu_buffer[1] = 0x85;
  // Send back the response, do not restart the event loop
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
  // Display back the original UX
  ui_idle();
}

static unsigned int 
bagl_ui_approval_nanos_button(unsigned int button_mask,
    unsigned int button_mask_counter) {
  switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
      approve_cb_();
      ui_idle();
      break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
      touch_deny();
      break;
  }   
  return 0;
}

void ui_approval(approve_callback_ty func) {
  approve_cb_ = func;
  UX_DISPLAY(bagl_ui_approval_nanos, NULL);
}

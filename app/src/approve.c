#include "approve.h"
#include "menu.h"
#include "os.h"
#include <kpl/app_errors.h>

char ApproveLine1[32] = {0};
char ApproveLine2[32] = {0};

static void touch_okay();
static void touch_deny();

#ifdef TARGET_NANOX
UX_STEP_VALID(ux_approval_flow_1_step, pbb, touch_okay(),
              {&C_icon_validate_14, ApproveLine1, ApproveLine2});
UX_STEP_VALID(ux_approval_flow_2_step, pb, touch_deny(),
              {&C_icon_crossmark, "Cancel"});

const ux_flow_step_t *const ux_approval_flow[] = {
    &ux_approval_flow_1_step,
    &ux_approval_flow_2_step,
    FLOW_END_STEP,
};

#elif defined(TARGET_NANOS)
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
#endif

static approve_callback_ty approve_cb_;

static void touch_okay() {
  uint8_t tx = 0;
  BEGIN_TRY {
    TRY {
      const uint16_t ret = approve_cb_(&tx);
      THROW(ret);
    }
    CATCH(EXCEPTION_IO_RESET) { THROW(EXCEPTION_IO_RESET); }
    CATCH_OTHER(e) {
      uint16_t sw;
      switch (e & 0xF000) {
      case 0x6000:
      case 0x9000:
        sw = e;
        break;
      default:
        sw = 0x6800 | (e & 0x7FF);
        break;
      }
      G_io_apdu_buffer[tx++] = sw >> 8;
      G_io_apdu_buffer[tx++] = sw & 0xFF;
      io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
    }
    FINALLY {}
  }
  END_TRY;
  ui_idle();
}

static void touch_deny() {
  G_io_apdu_buffer[0] = KPL_SW_UNAUTHORIZED_ACCESS >> 8;
  G_io_apdu_buffer[1] = KPL_SW_UNAUTHORIZED_ACCESS & 0xFF;
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
    touch_okay();
    break;

  case BUTTON_EVT_RELEASED | BUTTON_LEFT:
    touch_deny();
    break;
  }
  return 0;
}

void ui_approval(approve_callback_ty func) {
  approve_cb_ = func;
#if defined(TARGET_NANOX)
  // reserve a display stack slot if none yet
  if (G_ux.stack_count == 0) {
    ux_stack_push();
  }
  ux_flow_init(0, ux_approval_flow, NULL);
#elif defined(TARGET_NANOS)
  UX_DISPLAY(bagl_ui_approval_nanos, NULL);
#else
#error Unsupported target
#endif
}

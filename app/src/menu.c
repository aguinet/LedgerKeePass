#include "menu.h"
#include "os.h"

#include <kpl/kpl_csts.h>

UX_STEP_NOCB(ux_idle_flow_1_step, pnn,
             {
                 &C_logo,
                 "LedgerKeePass",
                 "app",
             });
UX_STEP_NOCB(ux_idle_flow_2_step, bn, {"Version", APPVERSION});
UX_STEP_VALID(ux_idle_flow_3_step, pb, os_sched_exit(-1),
              {
                  &C_icon_dashboard_x,
                  "Quit",
              });
UX_FLOW(ux_idle_flow, &ux_idle_flow_1_step, &ux_idle_flow_2_step,
        &ux_idle_flow_3_step, FLOW_LOOP);

void ui_idle(void) {
  // reserve a display stack slot if none yet
  if (G_ux.stack_count == 0) {
    ux_stack_push();
  }
  ux_flow_init(0, ux_idle_flow, NULL);
}

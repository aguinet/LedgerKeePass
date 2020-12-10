#ifndef APP_APPROVE_H
#define APP_APPROVE_H

extern char ApproveLine1[32];
extern char ApproveLine2[32];

typedef void(*approve_callback_ty)();
void ui_approval(approve_callback_ty cb);

#endif

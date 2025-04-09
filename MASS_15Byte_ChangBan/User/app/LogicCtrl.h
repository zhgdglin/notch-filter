#ifndef __LOGIC_CTRL_H
#define __LOGIC_CTRL_H
#include "bsp.h"

void CmdPickUp1(u8 *Data,u16 Len);
void CmdPickUp8(u8 *Data,u16 Len);
void PowerSet(u8 Level);
void PreAmpSet(u8 Level);

#endif /*__LOGIC_CTRL_H*/

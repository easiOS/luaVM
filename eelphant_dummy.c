#include "eelphant.h"
#include <stdio.h>

ep_window w;

ep_window* eelphant_create_window()
{
  w.flags = 1;
  return &w;
}

void eelphant_destroy_window(ep_window* w)
{
  w->flags = 0;
}

void eelphant_switch_active(ep_window* w)
{
}

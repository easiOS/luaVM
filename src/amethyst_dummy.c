#include <amethyst.h>
#include <stdio.h>

am_win w;

am_win* amethyst_create_window()
{
  w.flags = 1;
  return &w;
}

void amethyst_destroy_window(am_win* w)
{
  w->flags = 0;
}

void amethyst_set_active(am_win* w)
{
}

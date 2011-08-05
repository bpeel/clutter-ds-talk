
#include <clutter/clutter.h>

int
main (int argc, char **argv)
{
  ClutterActor *stage;
  const ClutterColor sky = { 0x98, 0xc1, 0xda, 0xff };

  if (clutter_init(&argc, &argv) != CLUTTER_INIT_SUCCESS)
    return 1;

  stage = clutter_stage_get_default ();
  clutter_stage_set_color (CLUTTER_STAGE (stage), &sky);
  clutter_stage_set_fullscreen (CLUTTER_STAGE (stage), TRUE);

  g_signal_connect (stage, "key-press-event",
                    G_CALLBACK (clutter_main_quit), NULL);

  clutter_actor_show (stage);

  clutter_main ();

  return 0;
}

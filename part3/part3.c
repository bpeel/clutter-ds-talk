
#include <clutter/clutter.h>

int
main (int argc, char **argv)
{
  GError *error = NULL;
  ClutterActor *stage, *cloud;
  const ClutterColor sky = { 0x98, 0xc1, 0xda, 0xff };

  if (clutter_init(&argc, &argv) != CLUTTER_INIT_SUCCESS)
    return 1;

  stage = clutter_stage_get_default ();
  clutter_stage_set_color (CLUTTER_STAGE (stage), &sky);
  clutter_stage_set_fullscreen (CLUTTER_STAGE (stage), TRUE);

  g_signal_connect (stage, "key-press-event",
                    G_CALLBACK (clutter_main_quit), NULL);

  if (!(cloud = clutter_texture_new_from_file ("cloud.png", &error)))
    {
      g_message ("Error loading image: %s", error->message);
      g_error_free (error);
      return -1;
    }

  clutter_actor_set_depth (cloud, -500);
  clutter_actor_animate (cloud, CLUTTER_LINEAR, 4000, "depth", 0, NULL);

  clutter_container_add_actor (CLUTTER_CONTAINER (stage), cloud);

  clutter_actor_show (stage);

  clutter_main ();

  return 0;
}

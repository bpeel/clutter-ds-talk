
#include <clutter/clutter.h>

int
main (int argc, char **argv)
{
  GError *error = NULL;
  ClutterAnimator *animator;
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

  animator = clutter_animator_new ();
  clutter_animator_set (animator,
                        cloud, "y", CLUTTER_LINEAR, 0.0, 0.0,
                        cloud, "opacity", CLUTTER_LINEAR, 0.0, 0,
                        cloud, "opacity", CLUTTER_EASE_IN_CUBIC, 0.15, 0xff,
                        cloud, "depth", CLUTTER_LINEAR, 0.0, -500.0,
                        cloud, "depth", CLUTTER_LINEAR, 1.0, 0.0,
                        cloud, "opacity", CLUTTER_LINEAR, 0.85, 0xff,
                        cloud, "opacity", CLUTTER_EASE_OUT_CUBIC, 1.0, 0,
                        cloud, "y", CLUTTER_LINEAR, 1.0, 100.0,
                        NULL);
  clutter_animator_set_duration (animator, 4000);
  clutter_animator_start (animator);

  clutter_container_add_actor (CLUTTER_CONTAINER (stage), cloud);

  clutter_actor_show (stage);

  clutter_main ();

  return 0;
}

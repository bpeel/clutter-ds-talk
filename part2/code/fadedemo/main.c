#ifdef _MSC_VER
#include <Windows.h>
#endif

#include <clutter/clutter.h>

#include "foo-fader.h"

static int which = 0;

static gboolean
button_press_event_cb (ClutterActor *stage,
                       const ClutterButtonEvent *event,
                       ClutterActor *fader)
{
  clutter_actor_animate (fader,
                         CLUTTER_LINEAR,
                         3000, /* duration */
                         "progress", (float) !which,
                         NULL);

  which = !which;

  return FALSE;
}

int
main (int argc, char **argv)
{
  ClutterActor *stage;
  ClutterActor *fader;
  ClutterConstraint *constraint;
  CoglHandle tex0, tex1;
  GError *error = NULL;

  if (clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS)
    return 1;

  if (argc != 3)
    {
      g_print ("usage: %s <image1> <image2>\n",
               argv[0]);
      return 1;
    }

  tex0 = cogl_texture_new_from_file (argv[1],
                                     COGL_TEXTURE_NO_SLICING |
                                     COGL_TEXTURE_NO_ATLAS,
                                     COGL_PIXEL_FORMAT_ANY,
                                     &error);
  if (tex0 == COGL_INVALID_HANDLE)
    {
      g_print ("error opening %s: %s\n", argv[1], error->message);
      g_clear_error (&error);
      return 1;
    }

  tex1 = cogl_texture_new_from_file (argv[2],
                                     COGL_TEXTURE_NO_SLICING |
                                     COGL_TEXTURE_NO_ATLAS,
                                     COGL_PIXEL_FORMAT_ANY,
                                     &error);
  if (tex1 == COGL_INVALID_HANDLE)
    {
      cogl_handle_unref (tex0);
      g_print ("error opening %s: %s\n", argv[2], error->message);
      g_clear_error (&error);
      return 1;
    }

  fader = foo_fader_new ();

  foo_fader_set_texture_0 (FOO_FADER (fader), tex0);
  foo_fader_set_texture_1 (FOO_FADER (fader), tex1);

  cogl_handle_unref (tex0);
  cogl_handle_unref (tex1);

  stage = clutter_stage_get_default ();
  clutter_stage_set_fullscreen (CLUTTER_STAGE (stage), TRUE);

  g_signal_connect (stage, "button-press-event",
                    G_CALLBACK (button_press_event_cb),
                    fader);

  clutter_container_add_actor (CLUTTER_CONTAINER (stage), fader);

  constraint =
    clutter_bind_constraint_new (stage,
                                 CLUTTER_BIND_SIZE,
                                 0.0f);
  clutter_actor_add_constraint (fader, constraint);

  /* massive hack --- nothing to see here! */
  foo_fader_set_progress (FOO_FADER (fader), 0.00001f);

  clutter_actor_show (stage);

  clutter_main ();

  return 0;
}

#ifdef _MSC_VER
int CALLBACK
WinMain (HINSTANCE instance,
	     HINSTANCE prev_instance,
		 LPSTR cmd_line,
		 int cmd_show)
{
	return main (__argc, __argv);
}
#endif
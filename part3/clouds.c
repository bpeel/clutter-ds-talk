#include <clutter/clutter.h>

#define SCALE_X_VARIANCE 0.3
#define SCALE_Y_VARIANCE 0.2
#define ROT_Z_VARIANCE 0.1
#define ALPHA_VARIANCE 0.5
#define ANIM_LENGTH 10000
#define ANIM_VARIANCE 0.3
#define OPACITY 0.60
#define OPACITY_VARIANCE 0.33
#define HEIGHT 0.85
#define HEIGHT_VARIANCE 0.2
#define HEIGHT_DROP 1.7

typedef struct
{
  ClutterActor *stage;
  ClutterActor *group;
  ClutterActor *cloud_texture;
  gfloat first_x;
  gfloat first_y;
} Data;

static inline gdouble
get_variance (gdouble value, gdouble constant)
{
  return value + value * (2.0 * g_random_double () - 1.0) * constant;
}

static gboolean
create_cloud(Data *data)
{
  ClutterPerspective perspective;
  ClutterAnimator *animator;
  ClutterTimeline *timeline;
  guint8 opacity;
  gfloat height;

  // Create a clone of the cloud texture
  ClutterActor *cloud = clutter_clone_new (data->cloud_texture);

  // Vary scale, rotation and position
  clutter_actor_set_scale_with_gravity (cloud,
                                        get_variance (1, SCALE_X_VARIANCE),
                                        get_variance (1, SCALE_Y_VARIANCE),
                                        CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_z_rotation_from_gravity (cloud,
                                             get_variance (360, ROT_Z_VARIANCE),
                                             CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_x (cloud,
    g_random_double_range (-clutter_actor_get_width (cloud),
                           clutter_actor_get_width (data->stage)));
  height = get_variance (clutter_actor_get_height (data->stage) * HEIGHT -
                         clutter_actor_get_height (cloud) / 2.0,
                         HEIGHT_VARIANCE);

  // Add new cloud to the cloud group
  clutter_container_add_actor (CLUTTER_CONTAINER (data->group), cloud);

  // Setup an animation for the cloud
  animator = clutter_animator_new();
  opacity = (guint8)(get_variance (OPACITY, OPACITY_VARIANCE) * 255.0);
  clutter_stage_get_perspective (CLUTTER_STAGE (data->stage), &perspective);
  clutter_animator_set (animator,
                        cloud, "y", CLUTTER_LINEAR, 0.0, height,
                        cloud, "opacity", CLUTTER_LINEAR, 0.0, 0,
                        cloud, "opacity", CLUTTER_EASE_IN_CUBIC, 0.15, opacity,
                        cloud, "depth", CLUTTER_LINEAR, 0.0, -perspective.z_far,
                        cloud, "depth", CLUTTER_LINEAR, 1.0, 200.0,
                        cloud, "opacity", CLUTTER_LINEAR, 0.85, opacity,
                        cloud, "opacity", CLUTTER_EASE_OUT_CUBIC, 1.0, 0,
                        cloud, "y", CLUTTER_LINEAR, 1.0, height * HEIGHT_DROP,
                        NULL);

  clutter_animator_set_duration (animator,
                                 get_variance (ANIM_LENGTH, ANIM_VARIANCE));

  // Destroy the cloud and the animator once the animation is finished
  timeline = clutter_animator_get_timeline (animator);
  g_signal_connect_swapped (timeline, "completed",
                            G_CALLBACK (clutter_actor_destroy), cloud);
  g_object_weak_ref (G_OBJECT (cloud), (GWeakNotify)g_object_unref, animator);

  // Start the animation
  clutter_animator_start (animator);

  return TRUE;
}

static void
stage_resized_cb (ClutterActor *stage,
                  GParamSpec   *pspec,
                  Data         *data)
{
  ClutterVertex vertex;
  gfloat width, height;
  ClutterPerspective perspective;

  clutter_actor_get_size (stage, &width, &height);

  // Set the stage perspective
  perspective.aspect = width / height;
  perspective.fovy = 1.0 / perspective.aspect * 135;
  perspective.z_near = 0.2;
  perspective.z_far = 1000;

  clutter_stage_set_perspective (CLUTTER_STAGE (stage), &perspective);

  // Set the rotation center of the group
  vertex.x = width / 2.f;
  vertex.y = height / 2.f;
  vertex.z = perspective.z_far * 0.65f + perspective.z_far * 0.35f;
  g_object_set (G_OBJECT (data->group),
                "rotation-center-x", &vertex,
                "rotation-center-z", &vertex,
                NULL);
}

static gboolean
stage_motion_event_cb (ClutterActor *stage,
                       ClutterEvent *event,
                       Data         *data)
{
  gfloat x, y, width, height, z_angle, x_angle;

  clutter_actor_get_size (stage, &width, &height);
  clutter_event_get_coords (event, &x, &y);

  // Set the cloud group angle based on the mouse coordinates
  z_angle = CLAMP (((x - data->first_x) / (width / 2.f)) * 180.f,
                   -80.f, 80.f);
  x_angle = CLAMP (((y - data->first_y) / (height / 2.f)) * 30.f,
                   -30.f, 30.f);

  clutter_actor_animate (data->group, CLUTTER_EASE_OUT_QUAD, 250,
                         "rotation-angle-z", z_angle,
                         "rotation-angle-x", x_angle,
                         NULL);
  return TRUE;
}

static gboolean
stage_button_press_event_cb (ClutterActor *stage,
                             ClutterEvent *event,
                             Data         *data)
{
  if (clutter_event_get_button (event) != 1)
    return FALSE;

  // Record the coordinates of the cursor on button-press
  clutter_event_get_coords (event, &data->first_x, &data->first_y);

  // Start listening to motion events
  g_signal_connect (stage, "motion-event",
                    G_CALLBACK (stage_motion_event_cb), data);

  return TRUE;
}

static gboolean
stage_button_release_event_cb (ClutterActor *stage,
                               ClutterEvent *event,
                               Data         *data)
{
  if (clutter_event_get_button (event) != 1)
    return FALSE;

  // Stop listening to motion events
  g_signal_handlers_disconnect_by_func (stage, stage_motion_event_cb, data);

  // Animate back to the 'flat' rotation
  clutter_actor_animate (data->group, CLUTTER_EASE_OUT_QUAD, 750,
                         "rotation-angle-z", 0.f,
                         "rotation-angle-x", 0.f,
                         NULL);

  return TRUE;
}

int
main (int argc, char **argv)
{
  Data data;
  guint source;

  GError *error = NULL;
  const ClutterColor sky = { 0x98, 0xc1, 0xda, 0xff };

  if (clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS)
    return 1;

  // Get the stage and set its colour and user-resizability
  data.stage = clutter_stage_get_default ();

  clutter_stage_set_color (CLUTTER_STAGE (data.stage), &sky);
  clutter_stage_set_fullscreen (CLUTTER_STAGE (data.stage), TRUE);

  // Quit on key-press
  g_signal_connect (data.stage, "key-press-event",
                    G_CALLBACK (clutter_main_quit), NULL);

  // Set the initial size of the stage
  clutter_actor_set_size (data.stage, 848, 480);

  // Create a group to put the clouds in
  data.group = clutter_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (data.stage), data.group);

  // Attach to the stage size changing so we can set perspective and
  // rotation centers on the cloud group
  g_signal_connect (data.stage, "notify::width",
                    G_CALLBACK (stage_resized_cb), &data);
  g_signal_connect (data.stage, "notify::height",
                    G_CALLBACK (stage_resized_cb), &data);

  // Attach to mouse events on the stage so we can rotate the group based on
  // mouse input
  g_signal_connect (data.stage, "button-press-event",
                    G_CALLBACK (stage_button_press_event_cb), &data);
  g_signal_connect (data.stage, "button-release-event",
                    G_CALLBACK (stage_button_release_event_cb), &data);

  // Load the cloud texture from disk
  if (!(data.cloud_texture =
        clutter_texture_new_from_file ("cloud.png", &error)))
    {
      g_message ("Error loading image: %s", error->message);
      g_error_free (error);
      return -1;
    }

  // Parent cloud texture so we can clone it
  clutter_actor_hide (data.cloud_texture);
  clutter_container_add_actor (CLUTTER_CONTAINER (data.stage),
                               data.cloud_texture);

  // Start generating clouds
  source = g_timeout_add_full (CLUTTER_PRIORITY_REDRAW, 100,
                               (GSourceFunc)create_cloud,
                               &data, NULL);

  // Show the stage
  clutter_actor_show (data.stage);

  clutter_main ();

  g_source_remove (source);

  return 0;
}

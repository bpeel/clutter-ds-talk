#include <clutter/clutter.h>
#include <string.h>
#include <math.h>

#include "foo-fader.h"

enum
{
  PROP_0,

  PROP_TEXTURE_0,
  PROP_TEXTURE_1,
  PROP_PROGRESS,

  N_PROPERTIES
};

static GParamSpec *
properties[N_PROPERTIES];

#define SPOTLIGHT_SIZE 256

G_DEFINE_TYPE (FooFader, foo_fader, CLUTTER_TYPE_ACTOR);

#define FOO_FADER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), FOO_TYPE_FADER, \
                                FooFaderPrivate))

struct _FooFaderPrivate
{
  CoglMaterial *material;

  float progress;
};

static void
foo_fader_set_property (GObject *object,
                        guint prop_id,
                        const GValue *value,
                        GParamSpec *pspec)
{
  FooFader *self = FOO_FADER (object);

  switch (prop_id)
    {
    case PROP_TEXTURE_0:
      foo_fader_set_texture_0 (self, g_value_get_boxed (value));
      break;

    case PROP_TEXTURE_1:
      foo_fader_set_texture_1 (self, g_value_get_boxed (value));
      break;

    case PROP_PROGRESS:
      foo_fader_set_progress (self, g_value_get_float (value));
      break;
    }
}

static void
foo_fader_get_property (GObject *object,
                        guint prop_id,
                        GValue *value,
                        GParamSpec *pspec)
{
  FooFader *self = FOO_FADER (object);

  switch (prop_id)
    {
    case PROP_PROGRESS:
      g_value_set_float (value, foo_fader_get_progress (self));
      break;
    }
}

static void
foo_fader_paint (ClutterActor *actor)
{
  FooFader *self = FOO_FADER (actor);
  FooFaderPrivate *priv = self->priv;
  guint8 paint_opacity = clutter_actor_get_paint_opacity (actor);
  ClutterActorBox allocation;

  clutter_actor_get_allocation_box (actor, &allocation);

  cogl_material_set_color4ub (priv->material,
                              paint_opacity,
                              paint_opacity,
                              paint_opacity,
                              paint_opacity);

  cogl_push_source (priv->material);

  cogl_rectangle (0, 0,
                  allocation.x2 - allocation.x1,
                  allocation.y2 - allocation.y1);

  cogl_pop_source ();
}

static gboolean
foo_fader_get_paint_volume (ClutterActor *actor,
                            ClutterPaintVolume *volume)
{
  return clutter_paint_volume_set_from_allocation (volume, actor);
}

static void
foo_fader_finalize (GObject *object)
{
  FooFader *self = (FooFader *) object;
  FooFaderPrivate *priv = self->priv;

  cogl_object_unref (priv->material);

  G_OBJECT_CLASS (foo_fader_parent_class)->finalize (object);
}

static void
foo_fader_class_init (FooFaderClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  ClutterActorClass *actor_class = (ClutterActorClass *) klass;

  gobject_class->finalize = foo_fader_finalize;
  gobject_class->set_property = foo_fader_set_property;
  gobject_class->get_property = foo_fader_get_property;

  actor_class->paint = foo_fader_paint;
  actor_class->get_paint_volume = foo_fader_get_paint_volume;

  properties[PROP_TEXTURE_0]
    = g_param_spec_boxed ("texture-0",
                          "Texture 0",
                          "The start texture for the fade",
                          COGL_TYPE_HANDLE,
                          G_PARAM_WRITABLE
                          | G_PARAM_STATIC_NAME
                          | G_PARAM_STATIC_NICK
                          | G_PARAM_STATIC_BLURB);

  properties[PROP_TEXTURE_1]
    = g_param_spec_boxed ("texture-1",
                          "Texture 1",
                          "The end texture for the fade",
                          COGL_TYPE_HANDLE,
                          G_PARAM_WRITABLE
                          | G_PARAM_STATIC_NAME
                          | G_PARAM_STATIC_NICK
                          | G_PARAM_STATIC_BLURB);

  properties[PROP_PROGRESS]
    = g_param_spec_float ("progress",
                          "Progress",
                          "The progress of the animation from 0 to 1",
                          0.0f, /* min */
                          1.0f, /* max */
                          0.0f, /* default */
                          G_PARAM_WRITABLE
                          | G_PARAM_READABLE
                          | G_PARAM_STATIC_NAME
                          | G_PARAM_STATIC_NICK
                          | G_PARAM_STATIC_BLURB);

  g_object_class_install_properties (gobject_class,
                                     N_PROPERTIES,
                                     properties);

  g_type_class_add_private (klass, sizeof (FooFaderPrivate));
}

static CoglHandle
create_spotlight_texture (void)
{
  guint8 buf[SPOTLIGHT_SIZE * SPOTLIGHT_SIZE * 4];
  int x, y;

  /* Clear the image first */
  memset (buf, 0, sizeof (buf));

  /* Algorithmically create a radial gradient in the image data */
  for (x = 0; x < SPOTLIGHT_SIZE; x++)
    for (y = 0; y < SPOTLIGHT_SIZE; y++)
      {
        int x_distance = (x - SPOTLIGHT_SIZE / 2);
        int y_distance = (y - SPOTLIGHT_SIZE / 2);
        float distance = sqrtf (x_distance * x_distance
                                + y_distance * y_distance);
        int value = (1.0f - distance / SPOTLIGHT_SIZE / 2) * 255.0f;

        buf[y * SPOTLIGHT_SIZE + x] = CLAMP (value, 0, 255);
      }

  /* Convert it into a cogl texture */
  return cogl_texture_new_from_data (SPOTLIGHT_SIZE, /* width */
                                     SPOTLIGHT_SIZE, /* height */
                                     COGL_TEXTURE_NO_SLICING
                                     | COGL_TEXTURE_NO_ATLAS,
                                     COGL_PIXEL_FORMAT_A_8, /* format */
                                     COGL_PIXEL_FORMAT_ANY, /* internal fmt */
                                     SPOTLIGHT_SIZE, /* rowstride */
                                     buf);
}

static void
foo_fader_init (FooFader *self)
{
  FooFaderPrivate *priv;
  CoglHandle spotlight;

  priv = self->priv = FOO_FADER_GET_PRIVATE (self);

  priv->material = cogl_material_new ();

  /* The first two layers just directly read the two images in the
     material */
  cogl_material_set_layer_combine (priv->material,
                                   0, /* layer_index */
                                   "RGBA = REPLACE (TEXTURE)",
                                   NULL);
  cogl_material_set_layer_combine (priv->material,
                                   1, /* layer_index */
                                   "RGBA = REPLACE (TEXTURE)",
                                   NULL);

  /* The third layer looks up a value in the spotlight image and then
     uses that to interpolate inbetween the two textures */
  cogl_material_set_layer_combine (priv->material,
                                   2, /* layer_index */
                                   "RGBA = INTERPOLATE (TEXTURE_0,"
                                   "                    TEXTURE_1,"
                                   "                    TEXTURE[A])",
                                   NULL);

  /* The final layer multiplies the result of that by the material's
     primary color. This is just to implement the actor's opacity */
  cogl_material_set_layer_combine (priv->material,
                                   3, /* layer_index */
                                   "RGBA = MODULATE (PREVIOUS, PRIMARY)",
                                   NULL);

  spotlight = create_spotlight_texture ();
  cogl_material_set_layer (priv->material, 2, spotlight);
  cogl_handle_unref (spotlight);
}

ClutterActor *
foo_fader_new (void)
{
  return g_object_new (FOO_TYPE_FADER, NULL);
}

void
foo_fader_set_texture_0 (FooFader *self,
                         CoglHandle texture)
{
  FooFaderPrivate *priv = self->priv;

  cogl_material_set_layer (priv->material, 0, texture);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (self));
}

void
foo_fader_set_texture_1 (FooFader *self,
                         CoglHandle texture)
{
  FooFaderPrivate *priv = self->priv;

  cogl_material_set_layer (priv->material, 1, texture);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (self));
}

void
foo_fader_set_progress (FooFader *self,
                        float progress)
{
  FooFaderPrivate *priv = self->priv;

  priv->progress = progress;

  clutter_actor_queue_redraw (CLUTTER_ACTOR (self));

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PROGRESS]);
}

float
foo_fader_get_progress (FooFader *self)
{
  return self->priv->progress;
}

#ifndef __FOO_FADER_H__
#define __FOO_FADER_H__

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define FOO_TYPE_FADER                                                  \
  (foo_fader_get_type())
#define FOO_FADER(obj)                                                  \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                   \
                               FOO_TYPE_FADER,                          \
                               FooFader))
#define FOO_FADER_CLASS(klass)                                          \
  (G_TYPE_CHECK_CLASS_CAST ((klass),                                    \
                            FOO_TYPE_FADER,                             \
                            FooFaderClass))
#define FOO_IS_FADER(obj)                                               \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                   \
                               FOO_TYPE_FADER))
#define FOO_IS_FADER_CLASS(klass)                                       \
  (G_TYPE_CHECK_CLASS_TYPE ((klass),                                    \
                            FOO_TYPE_FADER))
#define FOO_FADER_GET_CLASS(obj)                                        \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                                    \
                              FOO_FADER,                                \
                              FooFaderClass))

typedef struct _FooFader        FooFader;
typedef struct _FooFaderClass   FooFaderClass;
typedef struct _FooFaderPrivate FooFaderPrivate;

struct _FooFaderClass
{
  ClutterActorClass parent_class;
};

struct _FooFader
{
  ClutterActor parent;

  FooFaderPrivate *priv;
};

GType
foo_fader_get_type (void) G_GNUC_CONST;

ClutterActor *
foo_fader_new (void);

void
foo_fader_set_texture_0 (FooFader *self,
                         CoglHandle texture);

void
foo_fader_set_texture_1 (FooFader *self,
                         CoglHandle texture);

void
foo_fader_set_progress (FooFader *self,
                        float progress);

float
foo_fader_get_progress (FooFader *self);

G_END_DECLS

#endif /* __FOO_FADER_H__ */

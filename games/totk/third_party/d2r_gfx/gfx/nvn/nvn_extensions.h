/* Optional compile-time graphics extension, selected by the outer build.
   Providers implement command tags, immutable binding transforms, draw layout,
   output encoding and capture policy. Their implementation depends on this
   renderer; the renderer has no application/game-specific dependency.

   RECORD / TYPES / UNIFORMS / DRAW / PRESENT / CAPTURE / DIAGNOSTICS mark the
   declaration phases. Operation hooks execute inline in the existing order.
   Binding transforms must preserve the arena/submission lifetime and mark
   changed argument buffers volatile. Native defaults compile to the old path. */
#ifndef MNG_GFX_EXTENSION_PATH
#define MNG_GFX_EXTENSION_PATH_(path) #path
#define MNG_GFX_EXTENSION_PATH(path) MNG_GFX_EXTENSION_PATH_(path)
#endif
#ifdef MNG_GFX_EXTENSION
#include MNG_GFX_EXTENSION_PATH(MNG_GFX_EXTENSION)
#endif
#ifndef MNG_GFX_EXTENSION_DEFAULTS
#define MNG_GFX_EXTENSION_DEFAULTS
#ifndef MNG_GFX_TAG_DRAW
#define MNG_GFX_TAG_DRAW(...) (__VA_ARGS__)
#endif
#ifndef MNG_GFX_TAG_DRAW_INDEXED
#define MNG_GFX_TAG_DRAW_INDEXED(...) (__VA_ARGS__)
#endif
#ifndef MNG_GFX_PRESENT_TEXTURES
#define MNG_GFX_PRESENT_TEXTURES 1
#endif
#ifndef MNG_GFX_PRESENT_FLOATS
#define MNG_GFX_PRESENT_FLOATS 12
#endif
#ifndef MNG_GFX_UNIFORM_ACTIVE
#define MNG_GFX_UNIFORM_ACTIVE() 0
#endif
#ifndef MNG_GFX_ARGUMENTS_VOLATILE
#define MNG_GFX_ARGUMENTS_VOLATILE(stage) 0
#endif
#ifndef MNG_GFX_TRANSFORM_UNIFORM
#define MNG_GFX_TRANSFORM_UNIFORM(stage,reg,size,buffer,address) ((void)0)
#endif
#ifndef MNG_GFX_TRANSFORM_TEXTURE
#define MNG_GFX_TRANSFORM_TEXTURE(stage,reg,texture,resource_id) ((void)0)
#endif
#ifndef MNG_GFX_CUSTOM_DRAW
#define MNG_GFX_CUSTOM_DRAW 0
#endif
#ifndef MNG_GFX_CUSTOM_LAYOUT
#define MNG_GFX_CUSTOM_LAYOUT 0
#endif
#ifndef MNG_GFX_BEGIN_DRAW
#define MNG_GFX_BEGIN_DRAW() ((void)0)
#endif
#ifndef MNG_GFX_PREPARE_DRAW
#define MNG_GFX_PREPARE_DRAW(program) ((void)0)
#endif
#ifndef MNG_GFX_BEFORE_TARGET_CHANGE
#define MNG_GFX_BEFORE_TARGET_CHANGE() ((void)0)
#endif
#ifndef MNG_GFX_PRESENT_VIEWPORT
#define MNG_GFX_PRESENT_VIEWPORT(width,height,canvas_width,canvas_height) ((void)0)
#endif
#ifndef MNG_GFX_PRESENT_LEGACY
#define MNG_GFX_PRESENT_LEGACY(command,encoder,source,destination,uniforms) ((void)0)
#endif
#ifndef MNG_GFX_PRESENT_METAL4
#define MNG_GFX_PRESENT_METAL4(arguments,encoder,source,destination,uniforms,slot) ((void)0)
#endif
#ifndef MNG_GFX_CAPTURE_TAKE
#define MNG_GFX_CAPTURE_TAKE(source) ((void)0)
#endif
#ifndef MNG_GFX_CAPTURE_REGION
#define MNG_GFX_CAPTURE_REGION(source,crop,rect) ((void)0)
#endif
#endif

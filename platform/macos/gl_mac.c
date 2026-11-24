/* macOS: the game's GLES2 calls over a desktop OpenGL 2.1 context.
 *
 * macOS has no OpenGL ES. The sources are the handheld port's (GLES2 and a few OES
 * extensions); a desktop 2.1 context runs them nearly as they are, but some names differ:
 * framebuffer objects, mipmap generation, blits, multisampling and vertex array objects are
 * EXT/APPLE extensions there (core only from 3.0), and GLSL 1.20 has no precision
 * qualifiers. build.py generates a trampoline for every gl* the sources call; this file
 * fills them in (wmmt_mac_gl_lookup) and does the two translations that a name change
 * cannot do: eglGetProcAddress (the OES/EXT entry points) and glShaderSource (the GLSL).
 */
#include <SDL.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <EGL/egl.h>
#include "gl.h"

/* the generated gl_load.c's fallback for a name no driver offered */
void wmmt_mac_gl_stub(void) {}

static void *find_sym(const char *name) {
  void *p = SDL_GL_GetProcAddress(name);
  if (!p) p = dlsym(RTLD_DEFAULT, name);
  return p;
}

/* ---- glShaderSource: GLSL ES 1.00 -> desktop GLSL 1.20 ---- */
static int ident_char(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

/* drops the ES-only bits of a shader: #version 100, `precision ...;` statements (1.20 has
 * no default precision), the lowp/mediump/highp qualifiers, and the built-in
 * `invariant gl_Position;` re-declaration (1.20 has invariance, but Apple's compiler
 * rejects this form). Everything else (attribute, varying, texture2D, gl_FragColor) is
 * 1.20 as well. */
static char *glsl_fix(const char *s) {
  size_t n = strlen(s), o = 0;
  char *out = malloc(n + 1);
  if (!out) return NULL;
  for (size_t i = 0; i < n;) {
    size_t j = i;
    while (j < n && s[j] != '\n') j++;
    const char *l = s + i;
    size_t len = j - i, k = 0, start = 0;
    while (k < len && (l[k] == ' ' || l[k] == '\t')) k++;
    int drop = (len - k >= 8 && !strncmp(l + k, "#version", 8)) || (len - k >= 9 && !strncmp(l + k, "invariant", 9));
    if (!drop && len - k >= 9 && !strncmp(l + k, "precision", 9)) {
      /* only the precision statement goes: the line may hold more (`precision mediump
       * float; uniform vec4 uColor;`) */
      const char *semi = memchr(l + k, ';', len - k);
      start = semi ? (size_t)(semi - l) + 1 : len;
      while (start < len && (l[start] == ' ' || l[start] == '\t')) start++;
    }
    if (!drop) {
      for (size_t x = start; x < len;) {
        if (ident_char(l[x]) && !(x > 0 && ident_char(l[x - 1]))) {
          size_t e = x;
          while (e < len && ident_char(l[e])) e++;
          size_t w = e - x;
          int qual = (w == 4 && !strncmp(l + x, "lowp", 4)) || (w == 5 && !strncmp(l + x, "highp", 5)) ||
                     (w == 7 && !strncmp(l + x, "mediump", 7));
          if (qual) {
            x = e;
            while (x < len && (l[x] == ' ' || l[x] == '\t')) x++;
            continue;
          }
          memcpy(out + o, l + x, w), o += w, x = e;
          continue;
        }
        out[o++] = l[x++];
      }
    }
    if (j < n) out[o++] = '\n';
    i = j + (j < n ? 1 : 0);
  }
  out[o] = 0;
  return out;
}

typedef void (*shader_source_fn)(GLuint, GLsizei, const GLchar *const *, const GLint *);
static shader_source_fn shader_source_real;

static void mac_glShaderSource(GLuint sh, GLsizei count, const GLchar *const *src, const GLint *len) {
  (void)len;
  if (!shader_source_real) shader_source_real = (shader_source_fn)find_sym("glShaderSource");
  if (!shader_source_real) return;
  const GLchar **fixed = malloc((size_t)(count > 0 ? count : 1) * sizeof *fixed);
  if (!fixed) return shader_source_real(sh, count, src, len);
  for (GLsizei i = 0; i < count; ++i) fixed[i] = src[i] ? glsl_fix(src[i]) : NULL;
  shader_source_real(sh, count, fixed, NULL);
  for (GLsizei i = 0; i < count; ++i) free((void *)fixed[i]);
  free(fixed);
}

/* ---- GLES2 and OES names -> the desktop 2.1 extension names ---- */
static const struct { const char *es, *desktop; } MAP[] = {
  { "glGenFramebuffers", "glGenFramebuffersEXT" },
  { "glBindFramebuffer", "glBindFramebufferEXT" },
  { "glDeleteFramebuffers", "glDeleteFramebuffersEXT" },
  { "glCheckFramebufferStatus", "glCheckFramebufferStatusEXT" },
  { "glFramebufferTexture2D", "glFramebufferTexture2DEXT" },
  { "glFramebufferRenderbuffer", "glFramebufferRenderbufferEXT" },
  { "glGenRenderbuffers", "glGenRenderbuffersEXT" },
  { "glBindRenderbuffer", "glBindRenderbufferEXT" },
  { "glDeleteRenderbuffers", "glDeleteRenderbuffersEXT" },
  { "glRenderbufferStorage", "glRenderbufferStorageEXT" },
  { "glGenerateMipmap", "glGenerateMipmapEXT" },
  { "glBlitFramebuffer", "glBlitFramebufferEXT" },
  { "glRenderbufferStorageMultisample", "glRenderbufferStorageMultisampleEXT" },
  { "glGenVertexArraysOES", "glGenVertexArraysAPPLE" },
  { "glBindVertexArrayOES", "glBindVertexArrayAPPLE" },
  { "glDeleteVertexArraysOES", "glDeleteVertexArraysAPPLE" },
};

static void *mac_lookup(const char *name) {
  if (!strcmp(name, "glShaderSource")) return (void *)mac_glShaderSource;
  for (size_t i = 0; i < sizeof MAP / sizeof MAP[0]; ++i) {
    if (!strcmp(name, MAP[i].es)) {
      void *p = find_sym(MAP[i].desktop);
      return p ? p : find_sym(MAP[i].es); /* (a 3.2+ context has the core name) */
    }
  }
  return find_sym(name);
}

/* what build.py's generated gl_load.c calls: a stub for a name this profile has not
 * (glDiscardFramebufferEXT and friends: no-ops are all the game needs of them) */
void *wmmt_mac_gl_lookup(const char *name) {
  void *p = mac_lookup(name);
  if (!p) fprintf(stderr, "GL: no %s on this driver (a no-op stands in)\n", name);
  return p ? p : (void *)wmmt_mac_gl_stub;
}

/* render.c's eglGetProcAddress: the OES/EXT functions, NULL where the profile has none
 * (render.c checks: no invalidate, no multisampled-render-to-texture, no HQ pass) */
__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char *name) {
  return (__eglMustCastToProperFunctionPointerType)mac_lookup(name);
}
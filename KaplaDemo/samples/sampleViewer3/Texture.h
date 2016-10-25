#ifndef TEXTURE_H
#define TEXTURE_H

GLuint loadImgTexture(const char *filename);
GLuint loadCubeMap(const char *string, char mode = 0, char nameset = 0, float sr = 0.0f, float sg = 0.0f, float sb = 0.0f, float startGrad = -1.0f, float endGrad = -1.0f, char dir = 1);
GLuint createTexture(GLenum target, GLint internalformat, int w, int h, GLenum type, GLenum format, void *data);

#endif // !TEXTURE_H






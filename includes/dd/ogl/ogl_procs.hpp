 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#pragma once

void* GLProcAddress(const char *gl_function_name) {
    void *proc = reinterpret_cast<void*>(::wglGetProcAddress(gl_function_name));
    if (proc == nullptr) {
        HMODULE gl_module = ::LoadLibrary("opengl32.dll");
        DD_ASSERT(gl_module != nullptr);
        proc = reinterpret_cast<void*>(::GetProcAddress(gl_module, gl_function_name));
    }
    DD_ASSERT(proc != nullptr);
    return proc;
}

#define DEFINE_GL_PROC(ptr_name, name) ptr_name pfn_##name
#define LOAD_GL_PROC(ptr_name, name) pfn_##name = reinterpret_cast<ptr_name>(GLProcAddress(#name));

/* wgl */
DEFINE_GL_PROC(PFNWGLCREATECONTEXTATTRIBSARBPROC, wglCreateContextAttribsARB);
DEFINE_GL_PROC(PFNWGLGETEXTENSIONSSTRINGARBPROC,  wglGetExtensionsStringARB);
DEFINE_GL_PROC(PFNWGLCHOOSEPIXELFORMATARBPROC,    wglChoosePixelFormatARB);

/* gl buffers */
DEFINE_GL_PROC(PFNGLGENBUFFERSPROC,    glGenBuffers);
DEFINE_GL_PROC(PFNGLDELETEBUFFERSPROC, glDeleteBuffers);
DEFINE_GL_PROC(PFNGLBINDBUFFERPROC,    glBindBuffer);
DEFINE_GL_PROC(PFNGLBUFFERDATAPROC,    glBufferData);

/* gl textures */
DEFINE_GL_PROC(PFNGLGENTEXTURESPROC,    glGenTextures);
DEFINE_GL_PROC(PFNGLDELETETEXTURESPROC, glDeleteTextures);
DEFINE_GL_PROC(PFNGLBINDTEXTUREPROC,    glBindTexture);
DEFINE_GL_PROC(PFNGLACTIVETEXTUREPROC,  glActiveTexture);
DEFINE_GL_PROC(PFNGLTEXPARAMETERIPROC,  glTexParameteri);
DEFINE_GL_PROC(PFNGLTEXIMAGE2DPROC,     glTexImage2D);
DEFINE_GL_PROC(PFNGLGENERATEMIPMAPPROC, glGenerateMipmap);

/* gl shaders */
DEFINE_GL_PROC(PFNGLCREATESHADERPROC,  glCreateShader);
DEFINE_GL_PROC(PFNGLDELETESHADERPROC,  glDeleteShader);
DEFINE_GL_PROC(PFNGLSHADERSOURCEPROC,  glShaderSource);
DEFINE_GL_PROC(PFNGLCOMPILESHADERPROC, glCompileShader);
DEFINE_GL_PROC(PFNGLGETSHADERIVPROC,   glGetShaderiv);

/* gl program */
DEFINE_GL_PROC(PFNGLCREATEPROGRAMPROC,      glCreateProgram);
DEFINE_GL_PROC(PFNGLDELETEPROGRAMPROC,      glDeleteProgram);
DEFINE_GL_PROC(PFNGLATTACHSHADERPROC,       glAttachShader);
DEFINE_GL_PROC(PFNGLLINKPROGRAMPROC,        glLinkProgram);
DEFINE_GL_PROC(PFNGLUSEPROGRAMPROC,         glUseProgram);
DEFINE_GL_PROC(PFNGLGETPROGRAMIVPROC,       glGetProgramiv);
DEFINE_GL_PROC(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM1FVPROC,  glProgramUniform1fv);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM2FVPROC,  glProgramUniform2fv);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM3FVPROC,  glProgramUniform3fv);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM4FVPROC,  glProgramUniform4fv);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM1DVPROC,  glProgramUniform1dv);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM2DVPROC,  glProgramUniform2dv);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM3DVPROC,  glProgramUniform3dv);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM4DVPROC,  glProgramUniform4dv);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM1IVPROC,  glProgramUniform1iv);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM2IVPROC,  glProgramUniform2iv);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM3IVPROC,  glProgramUniform3iv);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM4IVPROC,  glProgramUniform4iv);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM1UIVPROC, glProgramUniform1uiv);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM2UIVPROC, glProgramUniform2uiv);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM3UIVPROC, glProgramUniform3uiv);
DEFINE_GL_PROC(PFNGLPROGRAMUNIFORM4UIVPROC, glProgramUniform4uiv);

/* gl vertex arrays */
DEFINE_GL_PROC(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays);
DEFINE_GL_PROC(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays);
DEFINE_GL_PROC(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray);
DEFINE_GL_PROC(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);
DEFINE_GL_PROC(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);

/* gl draw */
DEFINE_GL_PROC(PFNGLCLEARCOLORPROC, glClearColor);
DEFINE_GL_PROC(PFNGLCLEARPROC, glClear);
DEFINE_GL_PROC(PFNGLDRAWARRAYSPROC, glDrawArrays);
DEFINE_GL_PROC(PFNGLDRAWELEMENTSPROC, glDrawElements);

/* gl viewport */
DEFINE_GL_PROC(PFNGLVIEWPORTPROC, glViewport);

void LoadGLProcs() {
    /* wgl */
    LOAD_GL_PROC(PFNWGLCREATECONTEXTATTRIBSARBPROC, wglCreateContextAttribsARB);
    LOAD_GL_PROC(PFNWGLGETEXTENSIONSSTRINGARBPROC,  wglGetExtensionsStringARB);
    LOAD_GL_PROC(PFNWGLCHOOSEPIXELFORMATARBPROC,    wglChoosePixelFormatARB);

    /* gl buffers */
    LOAD_GL_PROC(PFNGLGENBUFFERSPROC,    glGenBuffers);
    LOAD_GL_PROC(PFNGLDELETEBUFFERSPROC, glDeleteBuffers);
    LOAD_GL_PROC(PFNGLBINDBUFFERPROC,    glBindBuffer);
    LOAD_GL_PROC(PFNGLBUFFERDATAPROC,    glBufferData);
    
    /* gl textures */
    LOAD_GL_PROC(PFNGLGENTEXTURESPROC,    glGenTextures);
    LOAD_GL_PROC(PFNGLDELETETEXTURESPROC, glDeleteTextures);
    LOAD_GL_PROC(PFNGLBINDTEXTUREPROC,    glBindTexture);
    LOAD_GL_PROC(PFNGLACTIVETEXTUREPROC,  glActiveTexture);
    LOAD_GL_PROC(PFNGLTEXPARAMETERIPROC,  glTexParameteri);
    LOAD_GL_PROC(PFNGLTEXIMAGE2DPROC,     glTexImage2D);
    LOAD_GL_PROC(PFNGLGENERATEMIPMAPPROC, glGenerateMipmap);

    /* gl shaders */
    LOAD_GL_PROC(PFNGLCREATESHADERPROC,  glCreateShader);
    LOAD_GL_PROC(PFNGLDELETESHADERPROC,  glDeleteShader);
    LOAD_GL_PROC(PFNGLSHADERSOURCEPROC,  glShaderSource);
    LOAD_GL_PROC(PFNGLCOMPILESHADERPROC, glCompileShader);
    LOAD_GL_PROC(PFNGLGETSHADERIVPROC,   glGetShaderiv);

    /* gl program */
    LOAD_GL_PROC(PFNGLCREATEPROGRAMPROC,      glCreateProgram);
    LOAD_GL_PROC(PFNGLDELETEPROGRAMPROC,      glDeleteProgram);
    LOAD_GL_PROC(PFNGLATTACHSHADERPROC,       glAttachShader);
    LOAD_GL_PROC(PFNGLLINKPROGRAMPROC,        glLinkProgram);
    LOAD_GL_PROC(PFNGLUSEPROGRAMPROC,         glUseProgram);
    LOAD_GL_PROC(PFNGLGETPROGRAMIVPROC,       glGetProgramiv);
    LOAD_GL_PROC(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM1FVPROC,  glProgramUniform1fv);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM2FVPROC,  glProgramUniform2fv);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM3FVPROC,  glProgramUniform3fv);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM4FVPROC,  glProgramUniform4fv);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM1DVPROC,  glProgramUniform1dv);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM2DVPROC,  glProgramUniform2dv);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM3DVPROC,  glProgramUniform3dv);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM4DVPROC,  glProgramUniform4dv);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM1IVPROC,  glProgramUniform1iv);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM2IVPROC,  glProgramUniform2iv);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM3IVPROC,  glProgramUniform3iv);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM4IVPROC,  glProgramUniform4iv);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM1UIVPROC, glProgramUniform1uiv);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM2UIVPROC, glProgramUniform2uiv);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM3UIVPROC, glProgramUniform3uiv);
    LOAD_GL_PROC(PFNGLPROGRAMUNIFORM4UIVPROC, glProgramUniform4uiv);

    /* gl vertex arrays */
    LOAD_GL_PROC(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays);
    LOAD_GL_PROC(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays);
    LOAD_GL_PROC(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray);
    LOAD_GL_PROC(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);
    LOAD_GL_PROC(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);

    /* gl draw */
    LOAD_GL_PROC(PFNGLCLEARCOLORPROC, glClearColor);
    LOAD_GL_PROC(PFNGLCLEARPROC, glClear);
    LOAD_GL_PROC(PFNGLDRAWARRAYSPROC, glDrawArrays);
    LOAD_GL_PROC(PFNGLDRAWELEMENTSPROC, glDrawElements);

    /* gl viewport */
    LOAD_GL_PROC(PFNGLVIEWPORTPROC, glViewport);
}

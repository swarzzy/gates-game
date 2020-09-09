#pragma once
#if defined(__gl_h_)
#error gl/gl.h included somewhere
#endif

#include "glcorearb.h"

// NOTE: It was defined in soko_glcorearb.h
#if defined(APIENTRY_DEFINED)
#undef APIENTRY
#endif

struct OpenGL
{
    union Functions
    {
        struct _Functions
        {
            // 1.0
            PFNGLCULLFACEPROC glCullFace;
            PFNGLFRONTFACEPROC glFrontFace;
            PFNGLHINTPROC glHint;
            PFNGLLINEWIDTHPROC glLineWidth;
            PFNGLPOINTSIZEPROC glPointSize;
            PFNGLPOLYGONMODEPROC glPolygonMode;
            PFNGLSCISSORPROC glScissor;
            PFNGLTEXPARAMETERFPROC glTexParameterf;
            PFNGLTEXPARAMETERFVPROC glTexParameterfv;
            PFNGLTEXPARAMETERIPROC glTexParameteri;
            PFNGLTEXPARAMETERIVPROC glTexParameteriv;
            PFNGLTEXIMAGE1DPROC glTexImage1D;
            PFNGLTEXIMAGE2DPROC glTexImage2D;
            PFNGLDRAWBUFFERPROC glDrawBuffer;
            PFNGLCLEARPROC glClear;
            PFNGLCLEARCOLORPROC glClearColor;
            PFNGLCLEARSTENCILPROC glClearStencil;
            PFNGLCLEARDEPTHPROC glClearDepth;
            PFNGLSTENCILMASKPROC glStencilMask;
            PFNGLCOLORMASKPROC glColorMask;
            PFNGLDEPTHMASKPROC glDepthMask;
            PFNGLDISABLEPROC glDisable;
            PFNGLENABLEPROC glEnable;
            PFNGLFINISHPROC glFinish;
            PFNGLFLUSHPROC glFlush;
            PFNGLBLENDFUNCPROC glBlendFunc;
            PFNGLLOGICOPPROC glLogicOp;
            PFNGLSTENCILFUNCPROC glStencilFunc;
            PFNGLSTENCILOPPROC glStencilOp;
            PFNGLDEPTHFUNCPROC glDepthFunc;
            PFNGLPIXELSTOREFPROC glPixelStoref;
            PFNGLPIXELSTOREIPROC glPixelStorei;
            PFNGLREADBUFFERPROC glReadBuffer;
            PFNGLREADPIXELSPROC glReadPixels;
            PFNGLGETBOOLEANVPROC glGetBooleanv;
            PFNGLGETDOUBLEVPROC glGetDoublev;
            PFNGLGETERRORPROC glGetError;
            PFNGLGETFLOATVPROC glGetFloatv;
            PFNGLGETINTEGERVPROC glGetIntegerv;
            PFNGLGETSTRINGPROC glGetString;
            PFNGLGETTEXIMAGEPROC glGetTexImage;
            PFNGLGETTEXPARAMETERFVPROC glGetTexParameterfv;
            PFNGLGETTEXPARAMETERIVPROC glGetTexParameteriv;
            PFNGLGETTEXLEVELPARAMETERFVPROC glGetTexLevelParameterfv;
            PFNGLGETTEXLEVELPARAMETERIVPROC glGetTexLevelParameteriv;
            PFNGLISENABLEDPROC glIsEnabled;
            PFNGLDEPTHRANGEPROC glDepthRange;
            PFNGLVIEWPORTPROC glViewport;
            //  11
            PFNGLDRAWARRAYSPROC glDrawArrays;
            PFNGLDRAWELEMENTSPROC glDrawElements;
            PFNGLGETPOINTERVPROC glGetPointerv;
            PFNGLPOLYGONOFFSETPROC glPolygonOffset;
            PFNGLCOPYTEXIMAGE1DPROC glCopyTexImage1D;
            PFNGLCOPYTEXIMAGE2DPROC glCopyTexImage2D;
            PFNGLCOPYTEXSUBIMAGE1DPROC glCopyTexSubImage1D;
            PFNGLCOPYTEXSUBIMAGE2DPROC glCopyTexSubImage2D;
            PFNGLTEXSUBIMAGE1DPROC glTexSubImage1D;
            PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D;
            PFNGLBINDTEXTUREPROC glBindTexture;
            PFNGLDELETETEXTURESPROC glDeleteTextures;
            PFNGLGENTEXTURESPROC glGenTextures;
            PFNGLISTEXTUREPROC glIsTexture;
            //  12
            PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElements;
            PFNGLTEXIMAGE3DPROC glTexImage3D;
            PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D;
            PFNGLCOPYTEXSUBIMAGE3DPROC glCopyTexSubImage3D;
            //  13
            PFNGLACTIVETEXTUREPROC glActiveTexture;
            PFNGLSAMPLECOVERAGEPROC glSampleCoverage;
            PFNGLCOMPRESSEDTEXIMAGE3DPROC glCompressedTexImage3D;
            PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D;
            PFNGLCOMPRESSEDTEXIMAGE1DPROC glCompressedTexImage1D;
            PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glCompressedTexSubImage3D;
            PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glCompressedTexSubImage2D;
            PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC glCompressedTexSubImage1D;
            PFNGLGETCOMPRESSEDTEXIMAGEPROC glGetCompressedTexImage;
            //  14
            PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
            PFNGLMULTIDRAWARRAYSPROC glMultiDrawArrays;
            PFNGLMULTIDRAWELEMENTSPROC glMultiDrawElements;
            PFNGLPOINTPARAMETERFPROC glPointParameterf;
            PFNGLPOINTPARAMETERFVPROC glPointParameterfv;
            PFNGLPOINTPARAMETERIPROC glPointParameteri;
            PFNGLPOINTPARAMETERIVPROC glPointParameteriv;
            PFNGLBLENDCOLORPROC glBlendColor;
            PFNGLBLENDEQUATIONPROC glBlendEquation;
            // 15
            PFNGLGENQUERIESPROC glGenQueries;
            PFNGLDELETEQUERIESPROC glDeleteQueries;
            PFNGLISQUERYPROC glIsQuery;
            PFNGLBEGINQUERYPROC glBeginQuery;
            PFNGLENDQUERYPROC glEndQuery;
            PFNGLGETQUERYIVPROC glGetQueryiv;
            PFNGLGETQUERYOBJECTIVPROC glGetQueryObjectiv;
            PFNGLGETQUERYOBJECTUIVPROC glGetQueryObjectuiv;
            PFNGLBINDBUFFERPROC glBindBuffer;
            PFNGLDELETEBUFFERSPROC glDeleteBuffers;
            PFNGLGENBUFFERSPROC glGenBuffers;
            PFNGLISBUFFERPROC glIsBuffer;
            PFNGLBUFFERDATAPROC glBufferData;
            PFNGLBUFFERSUBDATAPROC glBufferSubData;
            PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData;
            PFNGLMAPBUFFERPROC glMapBuffer;
            PFNGLUNMAPBUFFERPROC glUnmapBuffer;
            PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv;
            PFNGLGETBUFFERPOINTERVPROC glGetBufferPointerv;
            //  20
            PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate;
            PFNGLDRAWBUFFERSPROC glDrawBuffers;
            PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate;
            PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate;
            PFNGLSTENCILMASKSEPARATEPROC glStencilMaskSeparate;
            PFNGLATTACHSHADERPROC glAttachShader;
            PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
            PFNGLCOMPILESHADERPROC glCompileShader;
            PFNGLCREATEPROGRAMPROC glCreateProgram;
            PFNGLCREATESHADERPROC glCreateShader;
            PFNGLDELETEPROGRAMPROC glDeleteProgram;
            PFNGLDELETESHADERPROC glDeleteShader;
            PFNGLDETACHSHADERPROC glDetachShader;
            PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
            PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
            PFNGLGETACTIVEATTRIBPROC glGetActiveAttrib;
            PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
            PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders;
            PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
            PFNGLGETPROGRAMIVPROC glGetProgramiv;
            PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
            PFNGLGETSHADERIVPROC glGetShaderiv;
            PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
            PFNGLGETSHADERSOURCEPROC glGetShaderSource;
            PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
            PFNGLGETUNIFORMFVPROC glGetUniformfv;
            PFNGLGETUNIFORMIVPROC glGetUniformiv;
            PFNGLGETVERTEXATTRIBDVPROC glGetVertexAttribdv;
            PFNGLGETVERTEXATTRIBFVPROC glGetVertexAttribfv;
            PFNGLGETVERTEXATTRIBIVPROC glGetVertexAttribiv;
            PFNGLGETVERTEXATTRIBPOINTERVPROC glGetVertexAttribPointerv;
            PFNGLISPROGRAMPROC glIsProgram;
            PFNGLISSHADERPROC glIsShader;
            PFNGLLINKPROGRAMPROC glLinkProgram;
            PFNGLSHADERSOURCEPROC glShaderSource;
            PFNGLUSEPROGRAMPROC glUseProgram;
            PFNGLUNIFORM1FPROC glUniform1f;
            PFNGLUNIFORM2FPROC glUniform2f;
            PFNGLUNIFORM3FPROC glUniform3f;
            PFNGLUNIFORM4FPROC glUniform4f;
            PFNGLUNIFORM1IPROC glUniform1i;
            PFNGLUNIFORM2IPROC glUniform2i;
            PFNGLUNIFORM3IPROC glUniform3i;
            PFNGLUNIFORM4IPROC glUniform4i;
            PFNGLUNIFORM1FVPROC glUniform1fv;
            PFNGLUNIFORM2FVPROC glUniform2fv;
            PFNGLUNIFORM3FVPROC glUniform3fv;
            PFNGLUNIFORM4FVPROC glUniform4fv;
            PFNGLUNIFORM1IVPROC glUniform1iv;
            PFNGLUNIFORM2IVPROC glUniform2iv;
            PFNGLUNIFORM3IVPROC glUniform3iv;
            PFNGLUNIFORM4IVPROC glUniform4iv;
            PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv;
            PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
            PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
            PFNGLVALIDATEPROGRAMPROC glValidateProgram;
            PFNGLVERTEXATTRIB1DPROC glVertexAttrib1d;
            PFNGLVERTEXATTRIB1DVPROC glVertexAttrib1dv;
            PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f;
            PFNGLVERTEXATTRIB1FVPROC glVertexAttrib1fv;
            PFNGLVERTEXATTRIB1SPROC glVertexAttrib1s;
            PFNGLVERTEXATTRIB1SVPROC glVertexAttrib1sv;
            PFNGLVERTEXATTRIB2DPROC glVertexAttrib2d;
            PFNGLVERTEXATTRIB2DVPROC glVertexAttrib2dv;
            PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f;
            PFNGLVERTEXATTRIB2FVPROC glVertexAttrib2fv;
            PFNGLVERTEXATTRIB2SPROC glVertexAttrib2s;
            PFNGLVERTEXATTRIB2SVPROC glVertexAttrib2sv;
            PFNGLVERTEXATTRIB3DPROC glVertexAttrib3d;
            PFNGLVERTEXATTRIB3DVPROC glVertexAttrib3dv;
            PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f;
            PFNGLVERTEXATTRIB3FVPROC glVertexAttrib3fv;
            PFNGLVERTEXATTRIB3SPROC glVertexAttrib3s;
            PFNGLVERTEXATTRIB3SVPROC glVertexAttrib3sv;
            PFNGLVERTEXATTRIB4NBVPROC glVertexAttrib4Nbv;
            PFNGLVERTEXATTRIB4NIVPROC glVertexAttrib4Niv;
            PFNGLVERTEXATTRIB4NSVPROC glVertexAttrib4Nsv;
            PFNGLVERTEXATTRIB4NUBPROC glVertexAttrib4Nub;
            PFNGLVERTEXATTRIB4NUBVPROC glVertexAttrib4Nubv;
            PFNGLVERTEXATTRIB4NUIVPROC glVertexAttrib4Nuiv;
            PFNGLVERTEXATTRIB4NUSVPROC glVertexAttrib4Nusv;
            PFNGLVERTEXATTRIB4BVPROC glVertexAttrib4bv;
            PFNGLVERTEXATTRIB4DPROC glVertexAttrib4d;
            PFNGLVERTEXATTRIB4DVPROC glVertexAttrib4dv;
            PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f;
            PFNGLVERTEXATTRIB4FVPROC glVertexAttrib4fv;
            PFNGLVERTEXATTRIB4IVPROC glVertexAttrib4iv;
            PFNGLVERTEXATTRIB4SPROC glVertexAttrib4s;
            PFNGLVERTEXATTRIB4SVPROC glVertexAttrib4sv;
            PFNGLVERTEXATTRIB4UBVPROC glVertexAttrib4ubv;
            PFNGLVERTEXATTRIB4UIVPROC glVertexAttrib4uiv;
            PFNGLVERTEXATTRIB4USVPROC glVertexAttrib4usv;
            PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
            //  21
            PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x3fv;
            PFNGLUNIFORMMATRIX3X2FVPROC glUniformMatrix3x2fv;
            PFNGLUNIFORMMATRIX2X4FVPROC glUniformMatrix2x4fv;
            PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix4x2fv;
            PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv;
            PFNGLUNIFORMMATRIX4X3FVPROC glUniformMatrix4x3fv;
            //  30
            PFNGLCOLORMASKIPROC glColorMaski;
            PFNGLGETBOOLEANI_VPROC glGetBooleani_v;
            PFNGLGETINTEGERI_VPROC glGetIntegeri_v;
            PFNGLENABLEIPROC glEnablei;
            PFNGLDISABLEIPROC glDisablei;
            PFNGLISENABLEDIPROC glIsEnabledi;
            PFNGLBEGINTRANSFORMFEEDBACKPROC glBeginTransformFeedback;
            PFNGLENDTRANSFORMFEEDBACKPROC glEndTransformFeedback;
            PFNGLBINDBUFFERRANGEPROC glBindBufferRange;
            PFNGLBINDBUFFERBASEPROC glBindBufferBase;
            PFNGLTRANSFORMFEEDBACKVARYINGSPROC glTransformFeedbackVaryings;
            PFNGLGETTRANSFORMFEEDBACKVARYINGPROC glGetTransformFeedbackVarying;
            PFNGLCLAMPCOLORPROC glClampColor;
            PFNGLBEGINCONDITIONALRENDERPROC glBeginConditionalRender;
            PFNGLENDCONDITIONALRENDERPROC glEndConditionalRender;
            PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
            PFNGLGETVERTEXATTRIBIIVPROC glGetVertexAttribIiv;
            PFNGLGETVERTEXATTRIBIUIVPROC glGetVertexAttribIuiv;
            PFNGLVERTEXATTRIBI1IPROC glVertexAttribI1i;
            PFNGLVERTEXATTRIBI2IPROC glVertexAttribI2i;
            PFNGLVERTEXATTRIBI3IPROC glVertexAttribI3i;
            PFNGLVERTEXATTRIBI4IPROC glVertexAttribI4i;
            PFNGLVERTEXATTRIBI1UIPROC glVertexAttribI1ui;
            PFNGLVERTEXATTRIBI2UIPROC glVertexAttribI2ui;
            PFNGLVERTEXATTRIBI3UIPROC glVertexAttribI3ui;
            PFNGLVERTEXATTRIBI4UIPROC glVertexAttribI4ui;
            PFNGLVERTEXATTRIBI1IVPROC glVertexAttribI1iv;
            PFNGLVERTEXATTRIBI2IVPROC glVertexAttribI2iv;
            PFNGLVERTEXATTRIBI3IVPROC glVertexAttribI3iv;
            PFNGLVERTEXATTRIBI4IVPROC glVertexAttribI4iv;
            PFNGLVERTEXATTRIBI1UIVPROC glVertexAttribI1uiv;
            PFNGLVERTEXATTRIBI2UIVPROC glVertexAttribI2uiv;
            PFNGLVERTEXATTRIBI3UIVPROC glVertexAttribI3uiv;
            PFNGLVERTEXATTRIBI4UIVPROC glVertexAttribI4uiv;
            PFNGLVERTEXATTRIBI4BVPROC glVertexAttribI4bv;
            PFNGLVERTEXATTRIBI4SVPROC glVertexAttribI4sv;
            PFNGLVERTEXATTRIBI4UBVPROC glVertexAttribI4ubv;
            PFNGLVERTEXATTRIBI4USVPROC glVertexAttribI4usv;
            PFNGLGETUNIFORMUIVPROC glGetUniformuiv;
            PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;
            PFNGLGETFRAGDATALOCATIONPROC glGetFragDataLocation;
            PFNGLUNIFORM1UIPROC glUniform1ui;
            PFNGLUNIFORM2UIPROC glUniform2ui;
            PFNGLUNIFORM3UIPROC glUniform3ui;
            PFNGLUNIFORM4UIPROC glUniform4ui;
            PFNGLUNIFORM1UIVPROC glUniform1uiv;
            PFNGLUNIFORM2UIVPROC glUniform2uiv;
            PFNGLUNIFORM3UIVPROC glUniform3uiv;
            PFNGLUNIFORM4UIVPROC glUniform4uiv;
            PFNGLTEXPARAMETERIIVPROC glTexParameterIiv;
            PFNGLTEXPARAMETERIUIVPROC glTexParameterIuiv;
            PFNGLGETTEXPARAMETERIIVPROC glGetTexParameterIiv;
            PFNGLGETTEXPARAMETERIUIVPROC glGetTexParameterIuiv;
            PFNGLCLEARBUFFERIVPROC glClearBufferiv;
            PFNGLCLEARBUFFERUIVPROC glClearBufferuiv;
            PFNGLCLEARBUFFERFVPROC glClearBufferfv;
            PFNGLCLEARBUFFERFIPROC glClearBufferfi;
            PFNGLGETSTRINGIPROC glGetStringi;
            PFNGLISRENDERBUFFERPROC glIsRenderbuffer;
            PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
            PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
            PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
            PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
            PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv;
            PFNGLISFRAMEBUFFERPROC glIsFramebuffer;
            PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
            PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
            PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
            PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
            PFNGLFRAMEBUFFERTEXTURE1DPROC glFramebufferTexture1D;
            PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
            PFNGLFRAMEBUFFERTEXTURE3DPROC glFramebufferTexture3D;
            PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
            PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv;
            PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
            PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
            PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
            PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer;
            PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
            PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRange;
            PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
            PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
            PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
            PFNGLISVERTEXARRAYPROC glIsVertexArray;
        } fn;
        void* raw[sizeof(Functions::_Functions) / sizeof(void*)];
    } functions;

    static const uint32_t FunctionCount = sizeof(Functions::_Functions) / sizeof(void*);

    static const inline  char* FunctionNames[] =
    {
        "glCullFace",
        "glFrontFace",
        "glHint",
        "glLineWidth",
        "glPointSize",
        "glPolygonMode",
        "glScissor",
        "glTexParameterf",
        "glTexParameterfv",
        "glTexParameteri",
        "glTexParameteriv",
        "glTexImage1D",
        "glTexImage2D",
        "glDrawBuffer",
        "glClear",
        "glClearColor",
        "glClearStencil",
        "glClearDepth",
        "glStencilMask",
        "glColorMask",
        "glDepthMask",
        "glDisable",
        "glEnable",
        "glFinish",
        "glFlush",
        "glBlendFunc",
        "glLogicOp",
        "glStencilFunc",
        "glStencilOp",
        "glDepthFunc",
        "glPixelStoref",
        "glPixelStorei",
        "glReadBuffer",
        "glReadPixels",
        "glGetBooleanv",
        "glGetDoublev",
        "glGetError",
        "glGetFloatv",
        "glGetIntegerv",
        "glGetString",
        "glGetTexImage",
        "glGetTexParameterfv",
        "glGetTexParameteriv",
        "glGetTexLevelParameterfv",
        "glGetTexLevelParameteriv",
        "glIsEnabled",
        "glDepthRange",
        "glViewport",
        //  11
        "glDrawArrays",
        "glDrawElements",
        "glGetPointerv",
        "glPolygonOffset",
        "glCopyTexImage1D",
        "glCopyTexImage2D",
        "glCopyTexSubImage1D",
        "glCopyTexSubImage2D",
        "glTexSubImage1D",
        "glTexSubImage2D",
        "glBindTexture",
        "glDeleteTextures",
        "glGenTextures",
        "glIsTexture",
        //  12
        "glDrawRangeElements",
        "glTexImage3D",
        "glTexSubImage3D",
        "glCopyTexSubImage3D",
        //  13
        "glActiveTexture",
        "glSampleCoverage",
        "glCompressedTexImage3D",
        "glCompressedTexImage2D",
        "glCompressedTexImage1D",
        "glCompressedTexSubImage3D",
        "glCompressedTexSubImage2D",
        "glCompressedTexSubImage1D",
        "glGetCompressedTexImage",
        //  14
        "glBlendFuncSeparate",
        "glMultiDrawArrays",
        "glMultiDrawElements",
        "glPointParameterf",
        "glPointParameterfv",
        "glPointParameteri",
        "glPointParameteriv",
        "glBlendColor",
        "glBlendEquation",
        // 15
        "glGenQueries",
        "glDeleteQueries",
        "glIsQuery",
        "glBeginQuery",
        "glEndQuery",
        "glGetQueryiv",
        "glGetQueryObjectiv",
        "glGetQueryObjectuiv",
        "glBindBuffer",
        "glDeleteBuffers",
        "glGenBuffers",
        "glIsBuffer",
        "glBufferData",
        "glBufferSubData",
        "glGetBufferSubData",
        "glMapBuffer",
        "glUnmapBuffer",
        "glGetBufferParameteriv",
        "glGetBufferPointerv",
        //  20
        "glBlendEquationSeparate",
        "glDrawBuffers",
        "glStencilOpSeparate",
        "glStencilFuncSeparate",
        "glStencilMaskSeparate",
        "glAttachShader",
        "glBindAttribLocation",
        "glCompileShader",
        "glCreateProgram",
        "glCreateShader",
        "glDeleteProgram",
        "glDeleteShader",
        "glDetachShader",
        "glDisableVertexAttribArray",
        "glEnableVertexAttribArray",
        "glGetActiveAttrib",
        "glGetActiveUniform",
        "glGetAttachedShaders",
        "glGetAttribLocation",
        "glGetProgramiv",
        "glGetProgramInfoLog",
        "glGetShaderiv",
        "glGetShaderInfoLog",
        "glGetShaderSource",
        "glGetUniformLocation",
        "glGetUniformfv",
        "glGetUniformiv",
        "glGetVertexAttribdv",
        "glGetVertexAttribfv",
        "glGetVertexAttribiv",
        "glGetVertexAttribPointerv",
        "glIsProgram",
        "glIsShader",
        "glLinkProgram",
        "glShaderSource",
        "glUseProgram",
        "glUniform1f",
        "glUniform2f",
        "glUniform3f",
        "glUniform4f",
        "glUniform1i",
        "glUniform2i",
        "glUniform3i",
        "glUniform4i",
        "glUniform1fv",
        "glUniform2fv",
        "glUniform3fv",
        "glUniform4fv",
        "glUniform1iv",
        "glUniform2iv",
        "glUniform3iv",
        "glUniform4iv",
        "glUniformMatrix2fv",
        "glUniformMatrix3fv",
        "glUniformMatrix4fv",
        "glValidateProgram",
        "glVertexAttrib1d",
        "glVertexAttrib1dv",
        "glVertexAttrib1f",
        "glVertexAttrib1fv",
        "glVertexAttrib1s",
        "glVertexAttrib1sv",
        "glVertexAttrib2d",
        "glVertexAttrib2dv",
        "glVertexAttrib2f",
        "glVertexAttrib2fv",
        "glVertexAttrib2s",
        "glVertexAttrib2sv",
        "glVertexAttrib3d",
        "glVertexAttrib3dv",
        "glVertexAttrib3f",
        "glVertexAttrib3fv",
        "glVertexAttrib3s",
        "glVertexAttrib3sv",
        "glVertexAttrib4Nbv",
        "glVertexAttrib4Niv",
        "glVertexAttrib4Nsv",
        "glVertexAttrib4Nub",
        "glVertexAttrib4Nubv",
        "glVertexAttrib4Nuiv",
        "glVertexAttrib4Nusv",
        "glVertexAttrib4bv",
        "glVertexAttrib4d",
        "glVertexAttrib4dv",
        "glVertexAttrib4f",
        "glVertexAttrib4fv",
        "glVertexAttrib4iv",
        "glVertexAttrib4s",
        "glVertexAttrib4sv",
        "glVertexAttrib4ubv",
        "glVertexAttrib4uiv",
        "glVertexAttrib4usv",
        "glVertexAttribPointer",
        //  21
        "glUniformMatrix2x3fv",
        "glUniformMatrix3x2fv",
        "glUniformMatrix2x4fv",
        "glUniformMatrix4x2fv",
        "glUniformMatrix3x4fv",
        "glUniformMatrix4x3fv",
        //  30
        "glColorMaski",
        "glGetBooleani_v",
        "glGetIntegeri_v",
        "glEnablei",
        "glDisablei",
        "glIsEnabledi",
        "glBeginTransformFeedback",
        "glEndTransformFeedback",
        "glBindBufferRange",
        "glBindBufferBase",
        "glTransformFeedbackVaryings",
        "glGetTransformFeedbackVarying",
        "glClampColor",
        "glBeginConditionalRender",
        "glEndConditionalRender",
        "glVertexAttribIPointer",
        "glGetVertexAttribIiv",
        "glGetVertexAttribIuiv",
        "glVertexAttribI1i",
        "glVertexAttribI2i",
        "glVertexAttribI3i",
        "glVertexAttribI4i",
        "glVertexAttribI1ui",
        "glVertexAttribI2ui",
        "glVertexAttribI3ui",
        "glVertexAttribI4ui",
        "glVertexAttribI1iv",
        "glVertexAttribI2iv",
        "glVertexAttribI3iv",
        "glVertexAttribI4iv",
        "glVertexAttribI1uiv",
        "glVertexAttribI2uiv",
        "glVertexAttribI3uiv",
        "glVertexAttribI4uiv",
        "glVertexAttribI4bv",
        "glVertexAttribI4sv",
        "glVertexAttribI4ubv",
        "glVertexAttribI4usv",
        "glGetUniformuiv",
        "glBindFragDataLocation",
        "glGetFragDataLocation",
        "glUniform1ui",
        "glUniform2ui",
        "glUniform3ui",
        "glUniform4ui",
        "glUniform1uiv",
        "glUniform2uiv",
        "glUniform3uiv",
        "glUniform4uiv",
        "glTexParameterIiv",
        "glTexParameterIuiv",
        "glGetTexParameterIiv",
        "glGetTexParameterIuiv",
        "glClearBufferiv",
        "glClearBufferuiv",
        "glClearBufferfv",
        "glClearBufferfi",
        "glGetStringi",
        "glIsRenderbuffer",
        "glBindRenderbuffer",
        "glDeleteRenderbuffers",
        "glGenRenderbuffers",
        "glRenderbufferStorage",
        "glGetRenderbufferParameteriv",
        "glIsFramebuffer",
        "glBindFramebuffer",
        "glDeleteFramebuffers",
        "glGenFramebuffers",
        "glCheckFramebufferStatus",
        "glFramebufferTexture1D",
        "glFramebufferTexture2D",
        "glFramebufferTexture3D",
        "glFramebufferRenderbuffer",
        "glGetFramebufferAttachmentParameteriv",
        "glGenerateMipmap",
        "glBlitFramebuffer",
        "glRenderbufferStorageMultisample",
        "glFramebufferTextureLayer",
        "glMapBufferRange",
        "glFlushMappedBufferRange",
        "glBindVertexArray",
        "glDeleteVertexArrays",
        "glGenVertexArrays",
        "glIsVertexArray",
    };
};

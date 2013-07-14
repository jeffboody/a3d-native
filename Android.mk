# don't include LOCAL_PATH for submodules

include $(CLEAR_VARS)
LOCAL_MODULE    := a3d
LOCAL_CFLAGS    := -Wall -D$(A3D_CLIENT_VERSION)
LOCAL_SRC_FILES := a3d/a3d_log.c a3d/a3d_glsm.c a3d/a3d_time.c a3d/a3d_list.c \
                   a3d/a3d_tex.c a3d/a3d_texfont.c a3d/a3d_texstring.c \
                   a3d/math/a3d_mat3f.c a3d/math/a3d_mat4f.c a3d/math/a3d_stack4f.c a3d/math/a3d_regionf.c a3d/math/a3d_vec3f.c a3d/math/a3d_vec4f.c \
                   a3d/math/a3d_quaternion.c a3d/a3d_GL.c

ifeq ($(A3D_CLIENT_VERSION),A3D_GLESv1_CM)
	LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) a3d/a3d_GLESv1_CM.c
endif

ifeq ($(A3D_CLIENT_VERSION),A3D_GLESv2)
	LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) a3d/a3d_GLESv2.c a3d/a3d_shader.c
endif

LOCAL_LDLIBS    := -Llibs/armeabi \
                   -llog -lz -ldl

include $(BUILD_SHARED_LIBRARY)

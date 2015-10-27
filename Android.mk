# don't include LOCAL_PATH for submodules

include $(CLEAR_VARS)
LOCAL_MODULE    := a3d
LOCAL_CFLAGS    := -Wall -D$(A3D_CLIENT_VERSION)
LOCAL_SRC_FILES := a3d/a3d_log.c a3d/a3d_glsm.c a3d/a3d_unit.c a3d/a3d_time.c a3d/a3d_list.c \
                   a3d/a3d_tex.c a3d/a3d_texfont.c a3d/a3d_texstring.c a3d/a3d_workq.c a3d/a3d_cache.c \
                   a3d/math/a3d_mat3f.c a3d/math/a3d_mat4f.c a3d/math/a3d_stack4f.c a3d/math/a3d_regionf.c a3d/math/a3d_vec2f.c a3d/math/a3d_vec3f.c a3d/math/a3d_vec4f.c \
                   a3d/math/a3d_quaternion.c a3d/math/a3d_orientation.c a3d/math/a3d_sphere.c a3d/math/a3d_plane.c a3d/math/a3d_fplane.c a3d/a3d_GL.c \
                   a3d/math/a3d_ray.c a3d/math/a3d_rect4f.c \
                   a3d/widget/a3d_screen.c a3d/widget/a3d_layer.c a3d/widget/a3d_listbox.c a3d/widget/a3d_text.c a3d/widget/a3d_textbox.c a3d/widget/a3d_widget.c a3d/widget/a3d_font.c a3d/widget/a3d_radiolist.c \
                   a3d/widget/a3d_radiobox.c a3d/widget/a3d_checkbox.c a3d/widget/a3d_viewbox.c a3d/widget/a3d_bulletbox.c a3d/widget/a3d_sprite.c

LOCAL_LDLIBS := -Llibs/armeabi
LOCAL_SHARED_LIBRARIES := libpak libtexgz

ifeq ($(A3D_CLIENT_VERSION),A3D_GLESv1_CM_TRACE)
	LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) a3d/a3d_GLESv1_CM.c
endif

ifeq ($(A3D_CLIENT_VERSION),A3D_GLESv2_TRACE)
	LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) a3d/a3d_GLESv2.c a3d/a3d_shader.c
endif

ifeq ($(A3D_CLIENT_VERSION),A3D_GLESv1_CM)
	LOCAL_LDLIBS += -lGLESv1_CM
endif

ifeq ($(A3D_CLIENT_VERSION),A3D_GLESv2)
	LOCAL_SRC_FILES := $(LOCAL_SRC_FILES) a3d/a3d_GLESv2.c a3d/a3d_shader.c
	LOCAL_LDLIBS    += -lGLESv2
endif

LOCAL_LDLIBS += -llog -lz -ldl

include $(BUILD_SHARED_LIBRARY)

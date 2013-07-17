TARGET   = liba3d.a
A3D      = a3d_log a3d_texfont a3d_GL a3d_list a3d_tex a3d_time a3d_glsm a3d_shader a3d_texstring
A3D_MATH = a3d_mat3f a3d_mat4f a3d_regionf a3d_stack4f a3d_vec3f a3d_vec4f a3d_quaternion
SOURCE   = $(A3D:%=%.c) $(A3D_MATH:%=math/%.c)
OBJECTS  = $(SOURCE:.c=.o)
HFILES   = $(A3D:%=%.h) $(A3D_MATH:%=math/%.h)
OPT      = -O2 -Wall
CFLAGS   = $(OPT) -I. -DA3D_GL2
LDFLAGS  = -lGLEW -lGL -lm -L/usr/lib
AR       = ar

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

clean:
	rm -f $(OBJECTS) *~ \#*\# $(TARGET)

$(OBJECTS): $(HFILES)

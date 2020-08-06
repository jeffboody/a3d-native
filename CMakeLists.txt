cmake_minimum_required(VERSION 3.4.1)

# Compiler options
add_compile_options(-Wall)

# Submodule library
add_library(a3d
            STATIC

            # Source
            a3d_log.c
            a3d_glsm.c
            a3d_time.c
            a3d_list.c
            a3d_tex.c
            a3d_texfont.c
            a3d_texstring.c
            a3d_workq.c
            a3d_GL.c
            a3d_GLESv2.c
            a3d_shader.c
            math/a3d_mat3f.c
            math/a3d_mat4f.c
            math/a3d_stack4f.c
            math/a3d_regionf.c
            math/a3d_vec3f.c
            math/a3d_vec4f.c
            math/a3d_quaternion.c
            math/a3d_orientation.c
            math/a3d_sphere.c
            math/a3d_plane.c
            math/a3d_fplane.c)

# Linking
target_link_libraries(a3d

                      # NDK libraries
                      z
                      log)

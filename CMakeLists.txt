cmake_minimum_required(VERSION 3.4.1)

# Compiler options
add_compile_options(-Wall -DA3D_GLESv2)

# Submodule library
add_library(a3d
            STATIC

            # Source
            a3d_log.c
            a3d_glsm.c
            a3d_unit.c
            a3d_timestamp.c
            a3d_list.c
            a3d_hashmap.c
            a3d_multimap.c
            a3d_texfont.c
            a3d_texstring.c
            a3d_workq.c
            a3d_cache.c
            a3d_GL.c
            a3d_GLESv2.c
            a3d_shader.c
            math/a3d_mat3f.c
            math/a3d_mat4f.c
            math/a3d_stack4f.c
            math/a3d_regionf.c
            math/a3d_vec2f.c
            math/a3d_vec3f.c
            math/a3d_vec4f.c
            math/a3d_quaternion.c
            math/a3d_orientation.c
            math/a3d_sphere.c
            math/a3d_plane.c
            math/a3d_fplane.c
            math/a3d_ray.c
            math/a3d_rect4f.c
            widget/a3d_screen.c
            widget/a3d_layer.c
            widget/a3d_listbox.c
            widget/a3d_text.c
            widget/a3d_textbox.c
            widget/a3d_widget.c
            widget/a3d_font.c
            widget/a3d_radiolist.c
            widget/a3d_radiobox.c
            widget/a3d_checkbox.c
            widget/a3d_viewbox.c
            widget/a3d_bulletbox.c
            widget/a3d_sprite.c
            widget/a3d_hline.c)

# Linking
target_link_libraries(a3d

                      # Submodule libraries
                      pak
                      texgz

                      # NDK libraries
                      z
                      log
                      GLESv2)

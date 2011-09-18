project(showcase)

include(CheckPrototypeExists)

check_prototype_exists(DPMSCapable "X11/Xlib.h;X11/extensions/dpms.h" HAVE_DPMSCAPABLE_PROTO)
macro_bool_to_01(X11_dpms_FOUND HAVE_DPMS) # showcase


include_directories(
    ${KODFTEXT_INCLUDES}
    ${KOMAIN_INCLUDES}
)

add_subdirectory( part )
add_subdirectory( pics )
add_subdirectory( templates )
add_subdirectory( plugins )
add_subdirectory( styles )

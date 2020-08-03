/**
* Created by Fredia Huya-Kouadio.
*/

#ifndef GODOT_ARCORE_H
#define GODOT_ARCORE_H

#include <gdnative_api_struct.gen.h>

#ifdef __cplusplus
extern "C" {
#endif

void GDN_EXPORT godot_arcore_gdnative_init(godot_gdnative_init_options *options);
void GDN_EXPORT godot_arcore_gdnative_singleton();
void GDN_EXPORT godot_arcore_nativescript_init(void *handle);
void GDN_EXPORT godot_arcore_nativescript_terminate(void *handle);
void GDN_EXPORT godot_arcore_gdnative_terminate(godot_gdnative_terminate_options *options);

#ifdef __cplusplus
};
#endif

#endif // GODOT_ARCORE_H

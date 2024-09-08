#ifndef UTILS_H
#define UTILS_H

#include "godot_cpp/variant/projection.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include <android/log.h>
#include <jni.h>
#include <godot_cpp/variant/string.hpp>

#define LOG_TAG "ARCoreExtension"

#define ALOG_ASSERT(_cond, ...) \
	if (!(_cond))               \
	__android_log_assert("conditional", LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

/** Auxiliary macros */
#define __JNI_METHOD_BUILD(package, class_name, method) \
	Java_##package##_##class_name##_##method
#define __JNI_METHOD_EVAL(package, class_name, method) \
	__JNI_METHOD_BUILD(package, class_name, method)

/**
 * Expands the JNI signature for a JNI method.
 *
 * Requires to redefine the macros JNI_PACKAGE_NAME and JNI_CLASS_NAME.
 * Not doing so will raise preprocessor errors during build.
 *
 * JNI_PACKAGE_NAME must be the JNI representation Java class package name.
 * JNI_CLASS_NAME must be the JNI representation of the Java class name.
 *
 * For example, for the class com.example.package.SomeClass:
 * JNI_PACKAGE_NAME: com_example_package
 * JNI_CLASS_NAME: SomeClass
 *
 * Note that underscores in Java package and class names are replaced with "_1"
 * in their JNI representations.
 */
#define JNI_METHOD(method) \
	__JNI_METHOD_EVAL(JNI_PACKAGE_NAME, JNI_CLASS_NAME, method)

/**
 * Expands a Java class name using slashes as package separators into its
 * JNI type string representation.
 *
 * For example, to get the JNI type representation of a Java String:
 * JAVA_TYPE("java/lang/String")
 */
#define JAVA_TYPE(class_name) "L" class_name ";"

/**
 * Default definitions for the macros required in JNI_METHOD.
 * Used to raise build errors if JNI_METHOD is used without redefining them.
 */
#define JNI_CLASS_NAME "Error: JNI_CLASS_NAME not redefined"
#define JNI_PACKAGE_NAME "Error: JNI_PACKAGE_NAME not redefined"

/**
 * Converts JNI jstring to Godot String.
 * @param source Source JNI string. If null an empty string is returned.
 * @param env JNI environment instance.
 * @return Godot string instance.
 */
static inline godot::String jstring_to_string(JNIEnv *env, jstring source) {
	if (env && source) {
		const char *const source_utf8 = env->GetStringUTFChars(source, NULL);
		if (source_utf8) {
			godot::String result(source_utf8);
			env->ReleaseStringUTFChars(source, source_utf8);
			return result;
		}
	}
	return godot::String();
}

static inline void transpose(const float p_matrix[16], float o_res[16]) {
	// 0  1  2  3
	// 4  5  6  7
	// 8  9  10 11
	// 12 13 14 15

	// 0  4  8  12
	// 1  5  9  13
	// 2  6  10 14
	// 3  7  11 15
	o_res[0] = p_matrix[0];
	o_res[1] = p_matrix[4];
	o_res[2] = p_matrix[8];
	o_res[3] = p_matrix[12];
	o_res[4] = p_matrix[1];
	o_res[5] = p_matrix[5];
	o_res[6] = p_matrix[9];
	o_res[7] = p_matrix[13];
	o_res[8] = p_matrix[2];
	o_res[9] = p_matrix[6];
	o_res[10] = p_matrix[10];
	o_res[11] = p_matrix[14];
	o_res[12] = p_matrix[3];
	o_res[13] = p_matrix[7];
	o_res[14] = p_matrix[11];
	o_res[15] = p_matrix[15];
}

static inline godot::Projection to_godot_projection(const float p_matrix[16]) {
	return godot::Projection(
			godot::Vector4(p_matrix[0], p_matrix[1], p_matrix[2], p_matrix[3]),
			godot::Vector4(p_matrix[4], p_matrix[5], p_matrix[6], p_matrix[7]),
			godot::Vector4(p_matrix[8], p_matrix[9], p_matrix[10], p_matrix[11]),
			godot::Vector4(p_matrix[12], p_matrix[13], p_matrix[14], p_matrix[15]));
}

static inline godot::Transform3D to_godot_transform(const float p_matrix[16]) {
	// Right-handed column major, right?
	return godot::Transform3D(
			godot::Vector3(p_matrix[0], p_matrix[1], p_matrix[2]),
			godot::Vector3(p_matrix[4], p_matrix[5], p_matrix[6]),
			godot::Vector3(p_matrix[8], p_matrix[9], p_matrix[10]),
			godot::Vector3(p_matrix[12], p_matrix[13], p_matrix[14]));
}

#endif // UTILS_H

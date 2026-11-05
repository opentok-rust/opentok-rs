#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#ifndef _WIN32
#include <sys/time.h>
#include <dlfcn.h>
#endif

#ifdef WEBRTC_IOS
#include <TargetConditionals.h>
#endif

#if defined(_WIN32)
#define LIBOPENTOK_LIBRARY_EXPORTED __declspec(dllexport)
#elif defined(SWIG)
#define LIBOPENTOK_LIBRARY_EXPORTED
#else
#define LIBOPENTOK_LIBRARY_EXPORTED __attribute__((__visibility__("default")))
#endif

#if defined(__cplusplus)
#define OTC_DECL(type) LIBOPENTOK_LIBRARY_EXPORTED type
#define OTC_DEF(type) type
#define OTC_BEGIN_DECL extern "C" {
#define OTC_END_DECL }
#else
#define OTC_DECL(type) extern LIBOPENTOK_LIBRARY_EXPORTED type
#define OTC_DEF(type) type
#define OTC_BEGIN_DECL
#define OTC_END_DECL
#endif

#if defined(WEBRTC_MAC) && !defined(WEBRTC_IOS)
#define IS_MAC_OS 1
#else
#define IS_MAC_OS 0
#endif
#if defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID)
#define IS_LINUX 1
#else
#define IS_LINUX 0
#endif

#ifdef _WIN32
/*
This prevents a warning treated as error that appears when building windows with
SDL checks ON.
VS suggests replacing strncpy calls with the safer alternative strncpy_s.
Unfortunately the only way to completely ensure safety is to replace all strncpy
functions with std::string alternatives.
*/
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

#if defined(__clang__) || defined(__GNUC__)
#define OPENTOK_DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
#define OPENTOK_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#define OPENTOK_DEPRECATED(msg)
#endif

#endif  // CONFIG_H

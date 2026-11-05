/** @file media_transformer.h
    @brief Media transformer functions.

    This file includes functions for working with custom media transformers and
    Vonage Media Processor library transformers. For more informmation, see
    <a href="https://tokbox.com/developer/guides/vonage-media-processor/">this developer guide</a>.

    <b>Note:</b> This functionality only applies to the Vonage Video API macOS SDK. It does not work in the Vonage Video API Linux SDK. 
*/
#ifndef MEDIA_TRANSFORMER_H
#define MEDIA_TRANSFORMER_H

#include "audio_data.h"
#include "video_frame.h"

OTC_BEGIN_DECL

#define OTC_MEDIA_TRANSFORMER_ERROR_SPACE 0x0A000000

enum otc_media_transformer_error_code {
  OTC_MEDIA_TRANSFORMER_INTERNAL_ERROR = OTC_MEDIA_TRANSFORMER_ERROR_SPACE,
  OTC_MEDIA_TRANSFORMER_INVALID_TYPE,
  OTC_MEDIA_TRANSFORMER_NAME_NOT_PROVIDED,
  OTC_MEDIA_TRANSFORMER_CUSTOM_CALLBACK_NOT_PROVIDED,
  OTC_MEDIA_TRANSFORMER_PROPERTIES_NOT_PROVIDED,
  OTC_MEDIA_TRANSFORMER_MODEL_NOT_FOUND,
  OTC_MEDIA_TRANSFORMER_OPENTOK_TRANSFORMERS_LIBRARY_NOT_LOADED,
  OTC_MEDIA_TRANSFORMER_OPENTOK_TRANSFORMERS_SYMBOL_NOT_FOUND
};

/** OpenTok video transformer.
 */
typedef struct otc_video_transformer otc_video_transformer;

/** OpenTok audio transformer.
 */
typedef struct otc_audio_transformer otc_audio_transformer;

/**
 * @brief Available types of media transformers.
 */
enum otc_media_transformer_type {
    OTC_MEDIA_TRANSFORMER_TYPE_CUSTOM = 0,
    OTC_MEDIA_TRANSFORMER_TYPE_VONAGE
};

/**
 * Creates an instance of a Vonage Media Library video transformer or a custom video transformer. This function is
 * supported in the Vonge Video client SDK for macOS (not the Linux SDK). For more information, see <a
 * href="https://tokbox.com/developer/guides/vonage-media-processor/">Using the Vonage Media Processor library</a>. <p>
 * <b>Important:</b> In version 2.27.3+ of the Vonage Video client SDK for macOS,
 * you need to add the Vonage Media Library separately from the
 * Vonage Video macOS SDK. For details, see
 * <a href="https://tokbox.com/developer/guides/vonage-media-processor/mac/#vonage-media-library-integration">Vonage
 * Media Library integration</a>. Check additional information in the Vonage Media Library README.md file. <p> If an
 * error occurs, NULL is returned and "errno" is set.
 * @param type The type of transformer. Set the to `OTC_MEDIA_TRANSFORMER_TYPE_CUSTOM` for
 * a custom transformer. Set the to `OTC_MEDIA_TRANSFORMER_TYPE_VONAGE` for
 * a Vonage Media Library transformer.
 * @param name The name of the Vonage Media Library transformer or a unique name for
 * the custom video transformer.
 * @param properties A JSON string with the properties of the Vonage video transformer.
 * @param[in] cb The callback that will perform a custom transformation (disregarded for Vonage transformers)
 * @param[in] user_data Opaque data passed by the user and returned in the callback.
 * @return video_transformer opaque pointer video transfomer to create.
 * NULL if error occurs (which can be checked via "errno").
 */
OTC_DECL(struct otc_video_transformer*)
otc_video_transformer_create(
    enum otc_media_transformer_type type,
    const char* name,
    const char* properties,
    video_transform_callback cb,
    void* user_data);

/**
 * Creates an instance of a custom audio transformer.
 *
 * <b>Note:</b> This function only applies to the Vonage Video API macOS SDK.
 * It does not work in the Vonage Video API Linux SDK.
 * If an error occurs, NULL is returned and "errno" is set.
 * @param type The type of transformer. Set the to `OTC_MEDIA_TRANSFORMER_TYPE_CUSTOM` for
 * a custom transformer.
 * @param name A name for the custom tranformer.
 * @param properties A JSON string with the properties of the Vonage audio transformer.
 * @param[in] cb The callback that will perform a custom transformation (disregarded for Vonage transformers)
 * @param[in] user_data Opaque data passed by the user and returned in the callback.
 * @return audio_transformer opaque pointer audio transfomer to create.
 * NULL if error occurs (which can be checked via "errno").
 */
OTC_DECL(struct otc_audio_transformer*)
otc_audio_transformer_create(
    enum otc_media_transformer_type type,
    const char* name,
    const char* properties,
    audio_transform_callback cb,
    void* user_data);

/**
 * @brief Deletes a video transformer.
 *
 * <b>Note:</b> This function only applies to the Vonage Video API macOS SDK.
 * It does not work in the Vonage Video API Linux SDK.
 *
 * @param[in] video_transformer opaque pointer video transfomer to delete.
 */
OTC_DECL(otc_status)
otc_video_transformer_delete(
    struct otc_video_transformer* video_transformer);

/**
 * @brief Deletes an audio transformer.
 *
 * <b>Note:</b> This function only applies to the Vonage Video API macOS SDK.
 * It does not work in the Vonage Video API Linux SDK.
 *
 * @param[in] audio_transformer opaque pointer audio transfomer to delete.
 */
OTC_DECL(otc_status)
otc_audio_transformer_delete(
    struct otc_audio_transformer* audio_transformer);

/**
    Gets the error message associated to provided media transformers error code.
    @param error The media transformers error code.
    @return The associated error message or null if no error description is found for this code.
 */
OTC_DECL(const char*) otc_get_media_transformers_error_message(int error);

OTC_END_DECL

#endif /* MEDIA_TRANSFORMER_H */

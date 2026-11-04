/** @file publisher.h
    @brief OpenTok Publisher.

    This file includes the type definition for an OpenTok publisher along with
    several function declarations.
*/
#ifndef PUBLISHER_H
#define PUBLISHER_H

#include "base.h"
#include "config.h"
#include "stream.h"
#include "video_frame.h"
#include "camera_capture_resolution.h"
#include "media_transformer.h"
#include "video_capturer.h"
#include "media_utils.h"

OTC_BEGIN_DECL

#ifndef OTC_SESSION_FWD_DECL
#define OTC_SESSION_FWD_DECL
typedef struct otc_session otc_session;
#endif

#define OTC_MAX_VIDEO_LAYERS 3

/** OpenTok publisher type definition.

    A type representing a publisher of an audio-video stream to an OpenTok
   session.
 */
typedef struct otc_publisher otc_publisher;

/** OpenTok publisher settings type definition.
 */
typedef struct otc_publisher_settings otc_publisher_settings;

/** Error code enumeration for OpenTok publishers.

    This enumeration represents several error codes associated with a publisher.
 */
enum otc_publisher_error_code {
  OTC_PUBLISHER_INTERNAL_ERROR = 2000, /**< Internal error. */
  OTC_PUBLISHER_SESSION_DISCONNECTED =
      1010, /**< Error attempting to start captions in a session
               that you are not connected to. */
  OTC_PUBLISHER_TIMED_OUT = 1541, /**< Time-out attempting to publish. */
  OTC_PUBLISHER_UNABLE_TO_PUBLISH = 1500, /**< Unable to publish error. */
  OTC_PUBLISHER_WEBRTC_ERROR = 1610,      /**< WebRTC error. */
  OTC_PUBLISHER_ENCRYPTION_INTERNAL_ERROR =
      6001, /**< Internal error while encrypting a packet */
  OTC_PUBLISHER_ENCRYPTION_UNSUPPORTED_CODEC = 6005, /**< Unsupported media codec for encryption */
};

/** Publisher video type enumeration.

    This enumeration represents the different video types supported.
 */
enum otc_publisher_video_type {
  OTC_PUBLISHER_VIDEO_TYPE_CAMERA = 1, /**< Camera video stream. */
  OTC_PUBLISHER_VIDEO_TYPE_SCREEN = 2  /**< Screen capture video stream. */
};

/**
 * @brief Media type.
 */
enum otc_media_type {
  OTC_MEDIA_TYPE_VIDEO = 0,
  OTC_MEDIA_TYPE_AUDIO
};

/** Represents the statistics for a single published video layer.
 *
 *  In a simulcast configuration, each entry corresponds to one independent video encoding
 *  (e.g., low, medium, high resolution streams sent in parallel). For SVC (Scalable Video Coding),
 *  the publisher typically sends a single video encoding, with scalability (temporal, spatial, or both)
 *  internally encoded and described via the `scalability_mode` field.
 */
struct otc_publisher_video_layer_stats {
  int32_t width;                /**< Encoded video frame width */
  int32_t height;               /**< Encoded video frame height */
  double encoded_frame_rate;    /**< Encoded frame rate (fps) */
  int64_t bitrate;              /**< Bitrate (bps) */
  int64_t total_bitrate;        /**< Bitrate (bps), including RTP headers, padding and payload  */
  const char* scalability_mode; /**< Scalability mode string (e.g., "L1T1", "L3T3") for this video layer */
  enum otc_video_reason quality_limitation_reason; /**< Quality degradation reason */
  const char* codec;                               /**< Current codec for this video layer */
};

/** Publisher audio stats.

    This structure represents the publisher audio stats.
    The otc_publisher_callbacks struct includes a function pointer to
    an on_audio_stats function, which is called periodically to report
    audio stats.
 */
struct otc_publisher_audio_stats {
  const char* connection_id; /**< The connection ID of the client subscribing to
                                the stream. */
  const char* subscriber_id; /**< The subscriber ID of the client subscribing to
                                the stream (in a relayed session). */
  int64_t
      packets_lost; /**< The total number of audio packets that did not reach
                       the subscriber (or the OpenTok Media Router). */
  int64_t packets_sent; /**< The total number of audio packets sent to the
                           subscriber (or to the OpenTok Media Router). */
  int64_t bytes_sent;   /**< The total number of audio bytes sent to the
                           subscriber (or to the OpenTok Media Router). */
  float audio_level;    /**< The audio level value, from 0 to 1.0. */
  double timestamp; /**< The timestamp, in milliseconds since the Unix epoch,
                       for when these stats were gathered. */
  double
      start_time; /**< The timestamp, in milliseconds since the Unix epoch, from
                     which the cumulative totals started accumulating. */
  otc_transport_stats transport; /**< Transport statistics for this publisher */
};

/** Publisher video stats.

    This structure represents the publisher video stats.
    The otc_publisher_callbacks struct includes a function pointer to
    an on_video_stats function, which is called periodically to report
    video stats.
 */
struct otc_publisher_video_stats {
  const char* connection_id; /**< The connection ID of the client subscribing to
                                the stream. */
  const char* subscriber_id; /**< The subscriber ID of the client subscribing to
                                the stream (in a relayed session). */
  int64_t
      packets_lost; /**< The total number of video packets packets that did not
                       reach the subscriber (or the OpenTok Media Router). */
  int64_t packets_sent; /**< The total number of video packets sent sent to the
                           subscriber (or to the OpenTok Media Router). */
  int64_t bytes_sent;   /**< The total number of video bytes bytes sent to the
                           subscriber (or to the OpenTok Media Router). */
  double timestamp; /**< The timestamp, in milliseconds since the Unix epoch,
                       for when these stats were gathered. */
  double
      start_time; /**< The timestamp, in milliseconds since the Unix epoch, from
                     which the cumulative totals started accumulating. */
  size_t number_of_video_layers;                                             /**< Number of video layers */
  struct otc_publisher_video_layer_stats video_layers[OTC_MAX_VIDEO_LAYERS]; /**< Array of video layers statistics */
  otc_transport_stats transport; /**< Transport statistics for this publisher */
};

/** Publisher media link statistics.

    This structure represents the publisher media link statistics,
    providing a unified view of the transport-level network conditions.
    Since video and audio share the same underlying transport, this struct
    consolidates the local transport statistics into a single object.

    Reported periodically via the `on_media_link_stats` callback in
    `otc_publisher_callbacks`.
 */
struct otc_publisher_media_link_stats {
  const char* subscriber_id; /**< The subscriber ID associated with this stats report. Only relevant in relayed sessions where per-subscriber stats are reported. */
  const char* connection_id; /**< The connection ID associated with this stats report. */
  otc_transport_stats transport; /**< Local transport/network statistics for this publisher. */
};

/** Publisher callback functions.

    This structure is a set of function pointers to callback functions that can
   be called in response to events related to an OpenTok publisher.

    All callbacks will not be made on the application or main thread but on an
    internal thread. The application should return the callback as quickly as
    possible to avoid blocking the internal thread.

    Data passed into a callback function (other than `publisher` and
   `user_data`) will be released after the callback is called. Make a copy of
   the data if you need to retain it.
 */
struct otc_publisher_callbacks {
  /**
      Called when the publisher's stream is created.

      @param publisher A pointer to the publisher.
      @param user_data A pointer to the user_data you set for the publisher.
      @param stream A pointer to the stream.
   */
  void (*on_stream_created)(otc_publisher* publisher,
                            void* user_data,
                            const otc_stream* stream);

  /**
      Called when the publisher's stream is destroyed.

      @param publisher A pointer to the publisher.
      @param user_data A pointer to the user_data you set for the publisher.
      @param stream A pointer to the stream.
   */
  void (*on_stream_destroyed)(otc_publisher* publisher,
                              void* user_data,
                              const otc_stream* stream);

  /**
      Called when there is a new frame ready to be rendered by the publisher.
      <i>Do not</i> call any OpenTok API functions (except those in video_frame.h)
      on the same thread that invokes this callback.
      If you need to call an OpenTok API function (for example, functions in
      publisher.h or subscriber.h),
      reschedule its execution to a different thread.

      @param publisher A pointer to the publisher.
      @param user_data A pointer to the user_data you set for the publisher.
      @param frame A pointer to the new video frame.
   */
  void (*on_render_frame)(otc_publisher* publisher,
                          void* user_data,
                          const otc_video_frame* frame);

  /**
      Called periodically to report the audio level of the publisher.

      @param publisher A pointer to the publisher.
      @param user_data A pointer to the user_data you set for the publisher.
      @param audio_level The audio level value, from 0 to 1.0.
   */
  void (*on_audio_level_updated)(otc_publisher* publisher,
                                 void* user_data,
                                 float audio_level);

    /**
        Called when a moderator has forced this publisher to mute audio.

        @param publisher A pointer to the publisher.
        @param user_data A pointer to the user_data you set for the publisher.
     */
    void (*on_publisher_mute_forced)(otc_publisher *publisher,
                                     void *user_data);

  /**
      Called periodically to report audio statistics for the publisher.

      @param publisher A pointer to the publisher.
      @param user_data A pointer to the user_data you set for the publisher.
      @param audio_stats An array of publisher audio stats.
      @param number_of_stats The number of audio stats in the array.
   */
  void (*on_audio_stats)(otc_publisher* publisher,
                         void* user_data,
                         struct otc_publisher_audio_stats audio_stats[],
                         size_t number_of_stats);

  /**
      Called periodically to report video statistics for the publisher.

      @param publisher A pointer to the publisher.
      @param user_data A pointer to the user_data you set for the publisher.
      @param video_stats An array of publisher video stats.
      @param number_of_stats The number of video stats in the array.
   */
  void (*on_video_stats)(otc_publisher* publisher,
                         void* user_data,
                         struct otc_publisher_video_stats video_stats[],
                         size_t number_of_stats);
  /**
     Called when the publisher fails.

      @param publisher A pointer to the publisher.
      @param user_data A pointer to the user_data you set for the publisher.
      @param error_string A string containing the error message.
      @param error_code An error code enum value.
   */
  void (*on_error)(otc_publisher* publisher,
                   void* user_data,
                   const char* error_string,
                   enum otc_publisher_error_code error_code);

  /**
      Called when the publisher's video is disabled.

      @param publisher The instance invoking this call.
      @param user_data Pointer to user custom data bound to this struct.
      @param reason Gives more details about why the video has been disabled.
  */
  void (*on_video_disabled)(otc_publisher* publisher,
                            void* user_data,
                            enum otc_video_reason reason);

  /**
      Called when the publisher's video is enabled.

      @param publisher The instance invoking this call.
      @param user_data Pointer to user custom data bound to this struct.
      @param reason Gives more details about why the video has been enabled.
  */
  void (*on_video_enabled)(otc_publisher* publisher,
                           void* user_data,
                           enum otc_video_reason reason);

  /**
      Called when the publisher determines that the network congestion level
      has degraded and the video will be disabled if the quality degrades
      further.

      @param publisher The instance invoking this call.
      @param user_data Pointer to user custom data bound to this struct.
  */
  void (*on_video_disable_warning)(otc_publisher* publisher, void* user_data);

  /**
      Called when the publisher determines that the network congestion level
      has improved to the point at which the video being disabled is not an
      immediate risk.

      @param publisher The instance invoking this call.
      @param user_data Pointer to user custom data bound to this struct.
  */
  void (*on_video_disable_warning_lifted)(otc_publisher* publisher,
                                          void* user_data);

  /**
      This callback is invoked whenever there is a change in the quality state of the publisher's video stream.
      It provides information about the event, updated statistics and a reason code indicating why the event occurred.

      The `reason` parameter indicates the cause of the event and is considered with the following priority order:
      bandwidth degradation, CPU degradation, other quality degradation reasons, codec changes, resolution or layer
      changes.

      This event differs from `on_video_disabled` and `on_video_enabled` callbacks in that it reports video quality
      relevant changes. `on_video_disabled` and `on_video_enabled only indicate that the subscriber’s video track has
      been turned on or off (for example, due to a stream property update or audio-fallback). In contrast, this event
      provides diagnostic information about quality degradation causes—such as CPU or bandwidth limitations—and is also
      triggered when the video remains enabled.

      Applications can use this callback to monitor quality changes and react accordingly, such as adjusting UI
      elements or triggering alerts.

      @param publisher The instance invoking this call.
      @param stats Updated statistics including details about resolution or video layers change
      @param user_data Optional user data pointer passed when registering the callback.
      @param reason The reason code for the quality event, based on `otc_video_reason` enum.
   */
  void (*on_video_quality_changed)(struct otc_publisher* publisher,
                                   void* user_data,
                                   const struct otc_publisher_video_stats* stats,
                                   enum otc_video_reason reason);

  /**
      This callback is invoked when a network condition change is detected
      for the publisher.

      The `reason` parameter indicates the cause of the event and is currently
      always `OTC_NETWORK_REASON_NETWORK_CONDITION_CHANGE`.

      This event differs from `on_video_disabled` and `on_video_enabled` callbacks
      in that it reports network health changes rather than changes in
      the publisher’s video track state. Applications can use this callback
      to respond to network issues, such as updating UI, logging telemetry,
      or triggering adaptive behavior.

      @param publisher The publisher instance triggering the event.
      @param user_data Optional user data pointer passed when registering the callback.
      @param stats A pointer to the media link statistics for the publisher.
      @param reason The reason for the network condition change.
   */
  void (*on_network_condition_changed)(otc_publisher* publisher,
                                       void* user_data,
                                       const struct otc_publisher_media_link_stats* stats,
                                       enum otc_network_reason reason);

  /**
      Called periodically to report media link statistics for the publisher.

      Media link statistics provide a unified view of the transport-level
      network conditions.

      @param publisher The instance invoking this call.
      @param user_data Pointer to user custom data bound to this struct.
      @param media_link_stats A pointer to the media link stats struct.
   */
  void (*on_media_link_stats)(otc_publisher* publisher,
                              void* user_data,
                              struct otc_publisher_media_link_stats media_link_stats[],
                              size_t number_of_stats);

  /**
    A pointer to data you set related to the publisher.
   */
  void* user_data;

  /**
      A void pointer to a memory area holding reserved resources used for the
      internal implementation.
   */
  void* reserved;
};

/**
    Creates a new otc_publisher instance.

    @param name The name of the publisher. Other clients can get the names
                    for streams in the session.
    @param capturer Use this parameter if you want to provide a custom video
                    capturer. If it is set to null, the publisher uses a default
                    video capturer using the system's camera.
    @param callbacks A pointer to the structure with the publisher callback
   function pointers.
    @return A new otc_publisher instance. If there is an error, this is null.
 */
OTC_DECL(otc_publisher*)
otc_publisher_new(const char* name,
                  const struct otc_video_capturer_callbacks* capturer,
                  const struct otc_publisher_callbacks* callbacks);

/**
    Creates a new otc_publisher_settings instance.

    @return A new otc_publisher_settings instance
 */
OTC_DECL(otc_publisher_settings*) otc_publisher_settings_new();

/**
    Deletes an otc_publisher_settings instance.

    @param settings The otc_publisher_settings instance to be deleted.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_settings_delete(otc_publisher_settings* settings);

/**
    Sets the desired name for a publisher to be constructed with
    {@link otc_publisher_new_with_settings}.

    @param settings The otc_publisher_settings instance to be affected.
    @param name The name for the publisher. Other clients can get the names
                    for streams in the session.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_settings_set_name(otc_publisher_settings* settings,
                                const char* name);

/**
    Sets the video capturer for a publisher to be constructed with
    {@link otc_publisher_new_with_settings}.

    @param settings The otc_publisher_settings instance to be affected.
    @param capturer The video capturer.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_settings_set_video_capturer(
    otc_publisher_settings* settings,
    const struct otc_video_capturer_callbacks* capturer);

/**
    Enables an audio track for a publisher to be constructed with
    {@link otc_publisher_new_with_settings}.

    @param settings The otc_publisher_settings instance to be affected.
    @param enabled Enable/disable the existence of an audio track for
                   a publisher to be constructed with
                   {@link otc_publisher_new_with_settings}.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_settings_set_audio_track(otc_publisher_settings* settings,
                                       otc_bool enabled);

/**
    Enables a video track for a publisher to be constructed with
    {@link otc_publisher_new_with_settings}.

    @param settings The otc_publisher_settings instance to be affected.
    @param enabled Enable/disable the existence of an video track for
                   a publisher to be constructed with
                   {@link otc_publisher_new_with_settings}.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_settings_set_video_track(otc_publisher_settings* settings,
                                       otc_bool enabled);

/**
    Enables a sender statistics channel for a publisher to be constructed with
    {@link otc_publisher_new_with_settings}. Currently, this feature only
    supports calls with more than two participants. By default the sender
    statistics channel is disabled.

    @param settings The otc_publisher_settings instance to be affected.
    @param enabled Enable/disable the existence of a sender statistics channel for
                a publisher to be constructed with
                {@link otc_publisher_new_with_settings}.
    @return Return value indicating either error or success.
*/
OTC_DECL(otc_status)
otc_publisher_settings_set_sender_stats_track(otc_publisher_settings* settings, otc_bool enabled);

/**
    Enables stereo audio for a publisher to be constructed with
    {@link otc_publisher_new_with_settings}.

    @param settings The otc_publisher_settings instance to be affected.
    @param enabled Enable/disable stereo audio support for a publisher
                   to be constructed with {@link otc_publisher_new_with_settings}.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_settings_set_stereo(otc_publisher_settings* settings,
                                  otc_bool enabled);

/**
    Enables <a target="_top" href="https://datatracker.ietf.org/doc/html/rfc7587#section-3.1.3">
    Opus DTX</a> for a publisher to be constructed with
    {@link otc_publisher_new_with_settings}. Enabling Opus DTX can reduce bandwidth
    usage in streams that have long periods of silence.

    @param settings The otc_publisher_settings instance to be affected.
    @param enabled Whether to enable (true) or disable (false, the default) Opus DTX support.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_settings_set_opus_dtx(otc_publisher_settings* settings,
                                    otc_bool enabled);
/**
    Enables audio automatic gain control for a publisher to be constructed with
    {@link otc_publisher_new_with_settings}.

    It does not work in the Vonage Video API Linux SDK.

    @param settings The otc_publisher_settings instance to be affected.
    @param enabled Whether to enable (true, the default) or disable (false)
   automatic gain control.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_settings_set_auto_gain_control(otc_publisher_settings* settings,
                                             otc_bool enabled);

/**
    Enables noise suppression for a publisher to be
    constructed with {@link otc_publisher_new_with_settings}.

    <b>Note:</b> This function only applies to the Vonage Video API macOS SDK.
    It does not work in the Vonage Video API Linux SDK.

    @param settings The otc_publisher_settings instance to be affected.
    @param enabled Whether to enable (true, the default) or disable (false)
   noise suppression.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_settings_set_noise_suppression(otc_publisher_settings* settings,
                                             otc_bool enabled);

/**
    Enables acoustic echo cancellation for a publisher to be constructed with
    {@link otc_publisher_new_with_settings}.

    <b>Note:</b> This function only applies to the Vonage Video API macOS SDK.
    It does not work in the Vonage Video API Linux SDK.

    @param settings The otc_publisher_settings instance to be affected.
    @param enabled Whether to enable (true, the default) or disable (false)
                   acoustic echo cancellation support.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_settings_set_echo_cancellation(otc_publisher_settings* settings,
                                             otc_bool enabled);
/**
    When `otc_publisher_settings_set_disable_audio_processing` is set to
    `true`, the individual settings for acoustic echo cancellation,
    automatic gain control, and noise-suppression will be ignored and
    audio will not be processed at all. The default value is `false`.
    See {@link otc_publisher_new_with_settings}.

    @param settings The otc_publisher_settings instance to be affected.
    @param enabled Whether to enable (true) or disable (false, the default)
                   audio processing support.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_settings_set_disable_audio_processing
 (otc_publisher_settings* settings, otc_bool disabled);

/**
    Sets a camera capture resolution for default video capturers.

    Custom video capturers should not honor the capturing resolution set by 
    `otc_publisher_settings_set_default_camera_capture_resolution`, 
    the use of this is only for default video capturers.

    @param camera_capture_resolution The camera capture resolution for default video capturers.
    @param settings The otc_publisher_settings instance to be affected.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_settings_set_default_camera_capture_resolution(otc_publisher_settings* settings,
                                                             const enum otc_camera_capture_resolution camera_capture_resolution);

/**
    Allow use of scalable video
    <a target="_top" href="https://tokbox.com/developer/guides/scalable-video/">scalable video</a>
    for a publisher that has the <c>otc_publisher_video_type</c> set to <c>OTC_PUBLISHER_VIDEO_TYPE_SCREEN</c>
    with {@link otc_publisher_new_with_settings}.

    @param settings The otc_publisher_settings instance to be affected.
    @param enabled Enable/disable the use of scalable video for screen-sharing streams.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_settings_set_scalable_screenshare(otc_publisher_settings* settings,
                                                const otc_bool enabled);

/**
    When `otc_publisher_settings_set_allow_audio_capture_while_muted` is set to `false`,
    the microphone will be automatically switched off when all publishers
    have muted their audio. Mic will be automatically switched on again when at least one
    publisher unmutes their audio.
    If set to `true`, mic will always remain switched on even when all publishers have
    muted their audio.
    Be aware that when the mic is switched off, callbacks like audio_level or audio_data
    will no longer be invoked for the publishers.
    The default value is `true`. See {@link otc_session_new_with_settings}.

    @param settings Pointer to the session settings object.
    @param enable Boolean flag to disable (OTC_FALSE, the default value) or
    enable (OTC_TRUE) this flag.
    @return otc_status Returns the status of the operation.
 */
OTC_DECL(otc_status)
otc_publisher_settings_set_allow_audio_capture_while_muted(otc_publisher_settings* settings, const otc_bool enabled);

/**
    Creates a new otc_publisher instance.

    @param callbacks A pointer to the struct with publisher callback function
   pointers.
    @param settings The settings struct containing the desired settings for the
   publisher.
    @return A new otc_publisher instance. If there is an error, this is null.
 */
OTC_DECL(otc_publisher*)
otc_publisher_new_with_settings(const struct otc_publisher_callbacks* callbacks,
                                otc_publisher_settings* settings);

/**
    Releases a publisher instance, including all hardware resources bound to it.

    @param publisher The publisher instance to be deleted.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status) otc_publisher_delete(otc_publisher* publisher);

/**
    Returns the stream associated with the publisher.
    No callbacks in otc_publisher_callbacks will be invoked as a result of invoking this method (or afterwards).

    @param publisher The publisher instance to get the stream from.
    @return The stream associated with the publisher. If there is an error, this
   is null.
 */
OTC_DECL(otc_stream*) otc_publisher_get_stream(otc_publisher* publisher);

/**
    Whether to publish video or not. By default, streams publish both audio and
   video.

    @param publisher The publisher to be affected.
    @param publish_video Whether to publish video or not.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_set_publish_video(otc_publisher* publisher,
                                otc_bool publish_video);

/**
    Whether to publish audio or not. By default, streams publish both audio and
   video.

    @param publisher The publisher to be affected.
    @param publish_audio Whether to publish audio or not.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_set_publish_audio(otc_publisher* publisher,
                                otc_bool publish_audio);

/**
    Whether to make captions active or not. By default, it is off.

    @param publisher The publisher to be affected.
    @param publish_captions Whether to enable captioning  or not.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_set_publish_captions(otc_publisher* publisher,
                                   otc_bool publish_captions);

/**
    Returns whether the publisher is publishing video or not.

    @param publisher The publisher instance.
    @return  Whether the publisher is publishing video or not.
 */
OTC_DECL(otc_bool)
otc_publisher_get_publish_video(const otc_publisher* publisher);

/**
    Returns whether the Publisher is publishing audio or not.

    @param publisher The publisher instance.
    @return  Whether the publisher is publishing audio or not.
 */
OTC_DECL(otc_bool)
otc_publisher_get_publish_audio(const otc_publisher* publisher);

/**
    Returns whether the Publisher has captions active.

    @param publisher The publisher instance.
    @return  Whether the publisher has captions active or not.
 */
OTC_DECL(otc_bool)
otc_publisher_get_publish_captions(const otc_publisher* publisher);

/**
    Returns the {@link otc_session} associated this publisher instance.

    @param publisher The publisher instance.
    @return The {@link otc_session} associated this instance. If there is an
   error, this is null.
 */
OTC_DECL(otc_session*)
otc_publisher_get_session(const otc_publisher* publisher);

/**
    Sets the max audio bitrate for the publisher.

    @param publisher The publisher to be affected.
    @param bitrate The desired bitrate in kbps.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_set_max_audio_bitrate(otc_publisher* publisher, uint32_t bitrate);

/**
  Represents RTC statistics for a media stream published by the publisher.

  @see otc_publisher_get_rtc_stats_report
  @see otc_publisher_set_rtc_stats_report_cb
 */
struct otc_publisher_rtc_stats {
  /**
   For a relayed session (in which a publisher sends individual media streams
   to each subscriber), this is the unique ID of the client's connection.
  */
  const char* connection_id;
  /**
  A JSON array of RTC stats reports for the media stream.
  The structure of the JSON array is similar to the format of
  the RtcStatsReport object implemented in web browsers (see
  <a href="https://developer.mozilla.org/en-US/docs/Web/API/RTCStatsReport">Mozilla
  docs</a>). Also see <a href="https://w3c.github.io/webrtc-stats/#summary">this
  W3C documentation</a>.

  Here is a partial sample:

  <pre>
   [
      {
          "audioLevel": 0,
          "id": "RTCAudioSource_1",
          "kind": "audio",
          "timestamp": 1603448671532842,
          "totalAudioEnergy": 0,
          "totalSamplesDuration": 4.249999999999954,
          "trackIdentifier": "4nP5yeIDzbUm6IJho5jkTps1lnfabsFvTXjH00",
          "type": "media-source"
      },
      {
          "base64Certificate": "MIIBFjCB...QIhAMIfr/NgvhNp16zaoHxGQGrID1STFmBSSSB4V1bxBrTU",
          "fingerprint": "E7:5F:...:FA:5A:1F:A7:E0:55:60",
          "fingerprintAlgorithm": "sha-256",
          "id": "RTCCertificate_E7:5F:E5...F:1D:FA:5A:1F:A7:E0:55:60",
          "timestamp": 1603448671532842,
          "type": "certificate"
      },
      {
          "channels": 1,
          "clockRate": 8000,
          "id": "RTCCodec_audio_Inbound_0",
          "mimeType": "audio/PCMU",
          "payloadType": 0,
          "timestamp": 1603448671532842,
          "type": "codec"
      },
      {
          "channels": 2,
          "clockRate": 48000,
          "id": "RTCCodec_audio_Inbound_111",
          "mimeType": "audio/opus",
          "payloadType": 111,
          "sdpFmtpLine": "minptime=10;useinbandfec=1",
          "timestamp": 1603448671532842,
          "type": "codec"
      },
   ]
   </pre>
  */
  const char* json_array_of_reports;
};

/**
 Defines the struct for setting the publisher RTC stats report callack.

 @see otc_publisher_set_rtc_stats_report_cb.
 */
struct otc_publisher_rtc_stats_report_cb {
  /**
   Pointer to user custom data bound to this struct.
   */
  void *user_data;
  /**
   Defines the callback function for getting publisher RTC stats reports.

   @param publisher The instance invoking this call.

   @param stats A pointer to an array defining the RTC statistics for the publisher.
     For a routed session (a seesion that uses the
     <a href="https://tokbox.com/developer/guides/create-session/#media-mode">OpenTok
     Media Router</a>), this array includes one element, defining the statistics
     for the single video media stream that is sent to the OpenTok Media Router.
     In a relayed session, the array includes an object for each subscriber
     to the published stream.

   @param entries The size of the stats array.
   */
  void(*on_rtc_stats_report)(otc_publisher* publisher,
                             void *user_data,
                             const struct otc_publisher_rtc_stats* stats,
                             size_t entries);
  void *reserved;
};

/**
    Sets the RTC stats report callback the publisher.

    @param publisher The publisher instance.

    @param cb The otc_publisher_rtc_stats_report_cb struct that includes the callback function
    for the RTC stats report.

    @see otc_publisher_get_rtc_stats_report.
 */
OTC_DECL(otc_status) otc_publisher_set_rtc_stats_report_cb(otc_publisher *publisher,
                                                           const struct otc_publisher_rtc_stats_report_cb cb);
/**
    Gets the RTC stats report for the publisher. This is an asynchronous operation.
    create an otc_publisher_rtc_stats_report_cb struct and pass it into the
    {@link otc_publisher_set_rtc_stats_report_cb} function prior to calling
    this function. When the stats are available, the
    {@link otc_publisher_set_rtc_stats_report_cb} callback function is called.

    Also see {@link otc_publisher_callbacks.on_audio_stats},
    {@link otc_publisher_callbacks.on_video_stats}, and
    {@link otc_subscriber_get_rtc_stats_report}.

    @param publisher The publisher.
 */
OTC_DECL(otc_status) otc_publisher_get_rtc_stats_report(const otc_publisher *publisher);

/**
    Gets a unique identifier for a publisher.

    @param publisher The publisher instance.
    @return A unique identifier for the publisher. If there is an error, this is
   null.
 */
OTC_DECL(const char*)
otc_publisher_get_publisher_id(const otc_publisher* publisher);

/**
    Sets the publisher video type. By default, videos have the video type set
    to OTC_PUBLISHER_VIDEO_TYPE_CAMERA (indicating the source of the video is a
   camera). Set this to OTC_PUBLISHER_VIDEO_TYPE_SCREEN to indicate that the
   video source is screen sharing. Other clients can detect the video type for
   streams in the session (to determine the video source type).

    @param publisher The publisher to be affected.
    @param video_type The video type for the publisher.
    @return Return value indicating either error or success.
 */
OTC_DECL(otc_status)
otc_publisher_set_video_type(struct otc_publisher* publisher,
                             enum otc_publisher_video_type video_type);
/**
    Gets the publisher video type.
    See the {@link otc_publisher_set_video_type} function.

    @param publisher The publisher instance.
    @return The publisher video type.
 */
OTC_DECL(enum otc_publisher_video_type)
otc_publisher_get_video_type(const struct otc_publisher* publisher);

/**
    Gets user data for a given publisher.

    @param publisher The publisher instance.
    @return A pointer to the user data in memory.
 */
OTC_DECL(void*)
otc_publisher_get_user_data(const struct otc_publisher* publisher);

/**
    Gets the name for a given publisher.

    @param publisher The publisher instance.
    @return The name.
 */
OTC_DECL(const char*)
otc_publisher_get_name(const struct otc_publisher* publisher);

/**
 * @brief Sets an array of video transformers to this publisher's video stream. If null resets transformers.
 *
 * The array can be composed by vonage video transformers provided by ml-library and/or
 * custom transfomers. Transformers contain a pointer to callback invoked on each video frame.
 *
 * <b>Note:</b> This function only applies to the Vonage Video API macOS SDK.
 * It does not work in the Vonage Video API Linux SDK.
 *
 * @param[in] publisher The publisher instance.
 * @param[in] transformers Array of opaque pointers otc_video_transformer in the order to be applied to the stream.
 * @param[in] size size of the array.
 * @return an error in the event of an error otherwise OTC_SUCCESS.
 */
OTC_DECL(otc_status)
otc_publisher_set_video_transformers(
    otc_publisher* publisher,
    otc_video_transformer** transformers,
    uint8_t size);

/**
 * @brief Sets an array of audio transformers to this publisher's audio stream. If null resets transformers.
 *
 * The array can be composed by vonage audio transformers provided by ml-library and/or
 * custom transfomers. Transformers contain a pointer to callback invoked on each audio frame.
 *
 * <b>Note:</b> This function only applies to the Vonage Video API macOS SDK.
 * It does not work in the Vonage Video API Linux SDK.
 *
 * @param[in] publisher The publisher instance.
 * @param[in] transformers Array of opaque pointers otc_audio_transformer in the order to be applied to the stream.
 * @param[in] size size of the array.
 * @return an error in the event of an error otherwise OTC_SUCCESS.
 */
OTC_DECL(otc_status)
otc_publisher_set_audio_transformers(
    otc_publisher* publisher,
    otc_audio_transformer** transformers,
    uint8_t size);

/**
 * @brief Sets the target scalability mode (SVC - Scalable Video Coding) for this publisher.
 * The scalability mode controls the number of spatial (resolution) and temporal (frame rate) layers
 * that WebRTC encodes. The mode is applied to the routed (publisher ↔ SFU) peer connection only, regardless of which
 * leg is currently active.
 *
 * This method may be called at any time — it does not require the routed-leg negotiation
 * to have completed. The mode will be applied once the routed leg is established.
 *
 * If a non-SVC codec (VP8, H.264) is negotiated on the routed leg, the closest applicable
 * mode is used: spatial layers are forced to 1, preserving the temporal dimension
 * (e.g. L2T2 → L1T2 when VP8 is negotiated).
 *
 * This is a beta feature.
 *
 * Errors:
 * - Returns OTC_ERROR if the requested mode is not a valid scalability mode
 *   (i.e. not one of L1T1, L1T2, L1T3, L2T1, L2T2, L2T3, L3T1, L3T2, L3T3).
 *
 * @param publisher The publisher instance.
 * @param scalability_mode The target scalability mode.
 * @return OTC_SUCCESS if the operation succeeds; an error code otherwise.
 */
OTC_DECL(otc_status)
otc_publisher_set_target_scalability_mode(otc_publisher* publisher, const char* scalability_mode);

/**
 * @brief Gets the target scalability mode that was explicitly set via
 * {@link otc_publisher_set_target_scalability_mode}.
 * Returns the last value passed to the setter, or NULL if the setter has never been called.
 * This reflects the user's intent, not the mode actually applied (which may differ due to
 * codec-dependent fallback).
 *
 * This is a beta feature.
 *
 * @param publisher The publisher instance.
 * @return The target scalability mode set by the caller, or NULL if not explicitly set.
 */
OTC_DECL(const char*)
otc_publisher_get_target_scalability_mode(otc_publisher* publisher);

/**
 * @brief Gets the maximum video bitrate set for this publisher's video stream.
 *
 * Returns the current maximum video bitrate in bits per second (bps).
 * If no bitrate has been explicitly set via  otc_publisher_set_max_video_bitrate(),
 * or if a bitrate preset is active (including the default preset), this function returns 0.
 *
 * <b>Note:</b> A return value of 0 indicates that no manual bitrate cap is in effect,
 * and the stream is operating under a preset configuration.
 *
 * @param[in] publisher The publisher instance.
 * @return The current maximum video bitrate in bits per second, or 0 if unset or preset is used.
 */
OTC_DECL(otc_status)
otc_publisher_set_max_video_bitrate(otc_publisher* publisher, int bitrate_bps);

/**
 * @brief Gets the maximum video bitrate set for this publisher's video stream.
 *
 * Returns the currently configured maximum video bitrate in bits per second (bps).
 * If no manual bitrate has been set via otc_publisher_set_max_video_bitrate(),
 * or if a preset is active (including the default), this function returns 0.
 *
 * <b>Note:</b> A return value of 0 indicates that no explicit bitrate limit is applied
 * and the stream is using preset-based behavior instead.
 *
 * @param[in] publisher The publisher instance.
 * @return The current maximum video bitrate in bits per second, or 0 if not manually set.
 */
OTC_DECL(int)
otc_publisher_get_max_video_bitrate(const otc_publisher* publisher);

/**
 * @enum otc_video_bitrate_preset
 * @brief Preset strategies for controlling video bitrate in a publisher's stream.
 *
 * These presets allow developers to apply bandwidth strategies without manually
 * specifying bitrates. Presets are mutually exclusive with manual bitrate configuration.
 *
 * For details, see
 * <a target="_top" href="https://tokbox.com/developer/guides/publisher-max-bitrate/">Publisher Max Bitrate</a>.
 */
enum otc_video_bitrate_preset {
  /**
   * @brief Default preset. No extra restrictions are applied.
   *
   * This is the default mode.
   */
  OTC_VIDEO_BITRATE_PRESET_DEFAULT,

  /**
   * @brief Bandwidth saver preset.
   *
   * Applies moderate constraints to reduce bandwidth usage while
   * maintaining reasonable video quality.
   */
  OTC_VIDEO_BITRATE_PRESET_BW_SAVER,

  /**
   * @brief Extra bandwidth saver preset.
   *
   * Applies aggressive constraints to minimize bandwidth usage.
   */
  OTC_VIDEO_BITRATE_PRESET_EXTRA_BW_SAVER,

  /**
   * @brief Custom bitrate.
   *
   * Represents a manually configured bitrate using `otc_publisher_set_max_video_bitrate()`.
   * This value is returned by the preset getter only for internal tracking and
   * cannot be passed to the preset setter.
   */
  OTC_VIDEO_BITRATE_PRESET_CUSTOM
};

/**
 * @brief Sets a video bitrate preset for this publisher's video stream.
 *
 * Applies a predefined video bitrate strategy to control encoder behavior.
 * These presets are intended to balance video quality with bandwidth usage,
 * depending on the needs of the application.
 *
 * <b>Note:</b> The preset OTC_VIDEO_BITRATE_PRESET_CUSTOM cannot be set directly.
 * Calling otc_publisher_set_max_video_bitrate() will override the preset and
 * cause the preset getter to return OTC_VIDEO_BITRATE_PRESET_CUSTOM.
 *
 * @param[in] publisher The publisher instance.
 * @param[in] preset A value from `otc_video_bitrate_preset` to apply.
 * @return an error in the event of an error otherwise OTC_SUCCESS.
 */
OTC_DECL(otc_status)
otc_publisher_set_video_bitrate_preset(otc_publisher* publisher, enum otc_video_bitrate_preset preset);

/**
 * @brief Gets the currently active video bitrate preset for this publisher's stream.
 *
 * If a custom bitrate was previously set using otc_publisher_set_max_video_bitrate(),
 * this method returns OTC_VIDEO_BITRATE_PRESET_CUSTOM.
 *
 * @param[in] publisher The publisher instance.
 * @return The active bitrate preset.
 */
OTC_DECL(enum otc_video_bitrate_preset)
otc_publisher_get_video_bitrate_preset(const otc_publisher* publisher);

/**
 * @enum otc_degradation_preference
 * @brief Describes the policy that the video engine will try to follow when adapting frame rate and resolution to
 * limited bandwidth and CPU. This is a best effort policy. There are no guarantees on the frame rate and some frames can
 * still be skipped for other reasons.
 *
 * For details, see
 * <a target="_top" href="https://w3c.github.io/webrtc-pc/#idl-def-rtcdegradationpreference/">Degradation
 * Preference</a>.
 */
enum otc_degradation_preference {
  /**
   * @brief Default value. The video engine will decide the optimal degradation preference.
   */
  OTC_DEGRADATION_PREFERENCE_NOT_SET = -1,
  /**
   * @brief The video engine will not reduce the resolution and will try to keep the frame rate steady.
   * In some versions of the standard this option is referred to as "DISABLED", meaning that no degradation will be
   * applied at all when under limited resources.
   */
  OTC_DEGRADATION_PREFERENCE_MAINTAIN_FRAMERATE_AND_RESOLUTION = 0,
  /**
   * @brief The video engine will try to keep the frame rate steady but might reduce resolution if necessary.
   */
  OTC_DEGRADATION_PREFERENCE_MAINTAIN_FRAMERATE = 1,
  /**
   * @brief The video engine will not reduce the resolution but might reduce the frame rate if necessary.
   */
  OTC_DEGRADATION_PREFERENCE_MAINTAIN_RESOLUTION = 2,
  /**
   * @brief The video engine will try to reach a balance between reducing resolution and frame rate when necessary.
   */
  OTC_DEGRADATION_PREFERENCE_BALANCED = 3
};

/**
 * @brief Sets the preferred policy that the video engine will follow when adjusting frame rate and resolution to
 * limited bandwidth and CPU conditions. See otc_degradation_preference for a description of the possible values.
 * @param[in] publisher The publisher instance.
 * @param[in] degradation_preference The degradation preference.
 * @return OTC_SUCCESS or an error.
 */
OTC_DECL(otc_status)
otc_publisher_set_degradation_preference(otc_publisher* publisher, enum otc_degradation_preference degradation_preference);

/**
 * @brief Gets the preferred policy that the video engine will follow when adjusting frame rate and resolution to
 * limited bandwidth and CPU conditions. See otc_degradation_preference for a description of the possible values.
 * @param[in] publisher The publisher instance.
 * @return The current degradation preference.
 */
OTC_DECL(enum otc_degradation_preference)
otc_publisher_get_degradation_preference(const otc_publisher* publisher);

/**
    Enables or disables the subscriber audio-fallback feature (for subscribers
    to the published stream)

    The subscriber audio-fallback feature is available in sessions that use
    the Vonage Video Media Router. With the audio-fallback feature enabled (the
    default), when the Vonage Video Media Router determines that a stream's quality
    has degraded significantly for a specific subscriber to the stream, it
    disables the video in that subscriber in order to preserve audio quality.

    The default setting is OTC_TRUE (the audio-fallback feature is enabled).
    When using the screen video type, the subscriber audio-fallback setting
    is set to OTC_FALSE by default, so that the video does not drop out
    in subscribers.

    @param settings The publisher settings to be affected.
    @param enabled Whether we want to enable the audio-fallback feature or not.
    @return Return value indicating either error or success.
*/
OTC_DECL(otc_status)
otc_publisher_settings_set_subscriber_audio_fallback_enabled(
    otc_publisher_settings* settings,
    const otc_bool enabled);

/**
    Enables or disables the publisher audio-fallback feature.

    With the publisher audio-fallback feature enabled, when the Publisher
    determines that a stream's quality has degraded significantly, it disables
    the video stream in order to preserve audio quality and continues to send
    the audio stream. The capturer corresponding to this publisher will not be
    disabled, and the captured frames will still be seen on the publisher side.

    The default setting is OTC_FALSE (the audio-fallback feature is
    disabled). When using the screen video type in a session that uses the
    OpenTok Media Server, the publisher audio-fallback setting is set to
    OTC_FALSE by default, so that the video does not drop out in subscribers.

    @param settings The publisher settings to be affected.
    @param enabled Whether we want to enable the audio-fallback feature or not.
    @return Return value indicating either error or success.
*/
OTC_DECL(otc_status)
otc_publisher_settings_set_publisher_audio_fallback_enabled(
    otc_publisher_settings* settings,
    const otc_bool enabled);

/**
 * @brief Sets the preferred video codecs to use for this publisher's stream.
 *
 * This function allows specifying an explicit, ordered list of video codecs to prioritize
 * during negotiation. The codecs provided will be placed first in the SDP offer (in the given order),
 * but other supported codecs may still be included with lower priority.
 *
 * This does not restrict negotiation to only these codecs.
 *
 * Passing an empty list will result in an OT_INVALID_PARAMETER error.
 * If this function is not called, the SDK falls back to the default behavior
 * defined in the Dashboard project settings.
 *
 * @param[in,out] settings The publisher settings instance to modify.
 * @param[in] codecs An array of video codecs to prioritize (must be non-empty).
 * @param[in] num_codecs The number of elements in the codecs array.
 * @return OTC_SUCCESS if the operation succeeds; OT_INVALID_PARAMETER if the list is empty or invalid.
 */
OTC_DECL(otc_status)
otc_publisher_settings_set_preferred_video_codecs(
    otc_publisher_settings* settings,
    const otc_video_codec_type* codecs,
    size_t num_codecs);

/**
 * @brief Enables automatic codec prioritization for this publisher's stream.
 *
 * In automatic mode, the SDK determines the most appropriate codec priority order,
 * overriding the default project settings configured in the Dashboard.
 *
 * This provides a simple way to delegate codec preference to the SDK without specifying
 * a manual list.
 *
 * @param[in,out] settings The publisher settings instance to modify.
 * @return OTC_SUCCESS if the operation succeeds; an error code otherwise.
 */
OTC_DECL(otc_status)
otc_publisher_settings_set_preferred_video_codecs_automatic(
    otc_publisher_settings* settings);

OTC_END_DECL

#endif  // PUBLISHER_H

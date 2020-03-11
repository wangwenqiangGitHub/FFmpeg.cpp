#include <iostream>
#include <fpp/context/InputFormatContext.hpp>
#include <fpp/context/OutputFormatContext.hpp>
#include <fpp/codec/DecoderContext.hpp>
#include <fpp/codec/EncoderContext.hpp>
#include <fpp/refi/ResamplerContext.hpp>
#include <fpp/core/Utils.hpp>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavdevice/avdevice.h>
}

/*
video=HP Wide Vision FHD Camera
video=Webcam C170
video=USB2.0 PC CAMERA
rtsp://admin:admin@192.168.10.189:554/ch01.264
rtsp://admin:Admin2019@192.168.10.12:554
rtsp://admin:admin@192.168.10.3:554 (1080p)
*/

void startYoutubeStream() {

    /* create source */
    fpp::InputFormatContext camera { "rtsp://admin:admin@192.168.10.3:554" };

    /* create sink */
    const std::string stream_key { "apaj-8d8z-0ksj-6qxe" };
    fpp::OutputFormatContext youtube {
        "rtmp://a.rtmp.youtube.com/live2/"
        + stream_key
    };

    /* open source */
    camera.open();

    /* copy source's streams to sink */
    for (const auto& input_stream : camera.streams()) {
        const auto input_params { input_stream->params };
        const auto output_params {
            input_params->isVideo()
                ? fpp::utils::make_youtube_video_params()
                : fpp::utils::make_youtube_audio_params()
        };
        output_params->completeFrom(input_params);
        youtube.createStream(output_params);
    }

    /* create decoders */
    fpp::DecoderContext video_decoder { camera.stream(fpp::MediaType::Video) };
    fpp::DecoderContext audio_decoder { camera.stream(fpp::MediaType::Audio) };

    /* create resampler */
    fpp::ResamplerContext resampler {
        { camera.stream(fpp::MediaType::Audio)->params
        , youtube.stream(fpp::MediaType::Audio)->params }
    };

    /* create encoder's options */
    fpp::Dictionary video_options;
    video_options.setOption("threads",      "1"     );
    video_options.setOption("thread_type",  "slice" );
    video_options.setOption("preset",       "fast"  );
    video_options.setOption("crf",          "30"    );
    video_options.setOption("profile",      "main"  );
    video_options.setOption("tune",         "zerolatency");

    fpp::Dictionary audio_options;
    audio_options.setOption("preset", "low" );

    /* create encoders */
    fpp::EncoderContext video_encoder { youtube.stream(fpp::MediaType::Video), std::move(video_options) };
    fpp::EncoderContext audio_encoder { youtube.stream(fpp::MediaType::Audio), std::move(audio_options) };

    /* open sink */
    youtube.open();

    const auto transcode_video {
        fpp::utils::transcoding_required(
            { camera.stream(fpp::MediaType::Video)->params
            , youtube.stream(fpp::MediaType::Video)->params })
    };

    youtube.stream(fpp::MediaType::Video)->setEndTimePoint(10 * 60 * 1000); // 10 min

    fpp::Packet input_packet { fpp::MediaType::Unknown };
    while ([&](){ input_packet = camera.read(); return !input_packet.isEOF(); }()) {
        if (input_packet.isVideo()) {
            if (transcode_video) {
                const auto video_frames { video_decoder.decode(input_packet) };
                for (const auto& v_frame : video_frames) {
                    const auto video_packets { video_encoder.encode(v_frame) };
                    for (const auto& v_packet : video_packets) {
                        youtube.write(v_packet);
                    }
                }
            }
            else {
                youtube.write(input_packet);
            }
        }
        else if (input_packet.isAudio()) {
            input_packet.setDts(AV_NOPTS_VALUE);
            input_packet.setPts(AV_NOPTS_VALUE);
            const auto audio_frames { audio_decoder.decode(input_packet) };
            for (const auto& a_frame : audio_frames) {
                const auto resampled_frames { resampler.resample(a_frame) };
                for (auto& ra_frame : resampled_frames) {
                    auto audio_packets { audio_encoder.encode(ra_frame) };
                    for (auto& a_packet : audio_packets) {
                        youtube.write(a_packet);
                    }
                }
            }
        }
        else {
            throw std::runtime_error { "Unknown packet! "};
        }
    }

}

int main() {

    try {

        #pragma warning( push )
        #pragma warning( disable : 4974)
        ::av_register_all();
        #pragma warning( pop )
        ::avformat_network_init();
        ::avdevice_register_all();

        startYoutubeStream();

    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    } catch (...) {
        std::cout << "Unknown exception\n";
    }

    std::cout << "Finished\n";

    return 0;

}

#include "InputFormatContext.hpp"
#include <cstdio>
#include <fpp/core/Utils.hpp>
#include <fpp/core/FFmpegException.hpp>
#include <iostream>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace fpp {

InputFormatContext::InputFormatContext(const std::string_view mrl, const std::string_view format)
    : _input_format { findInputFormat(format) } {
    setMediaResourceLocator(mrl);
	// printf("test:%s\n");
	printf("<%s %d>\n",__func__, __LINE__);
	std::cout << mrl << std::endl;
    createContext();
}

InputFormatContext::InputFormatContext(InputContext* input_ctx, const std::string_view format)
    : _input_format { findInputFormat(format) } {
    setMediaResourceLocator("Custom input buffer");
    createContext();
    raw()->pb = input_ctx->raw();
    setFlag(AVFMT_FLAG_CUSTOM_IO);
    setFlag(AVFMT_NOFILE);
}

InputFormatContext::~InputFormatContext() {
    try {
        close();
    }
    catch (...) {
        utils::handle_exceptions(this);
    }

    avformat_license();
}

bool InputFormatContext::seek(int stream_index, std::int64_t timestamp, SeekPrecision seek_precision) {
    const auto flags {
        [&]() -> int {
            switch (seek_precision) {
                case SeekPrecision::Forward:  return 0;
                case SeekPrecision::Backward: return AVSEEK_FLAG_BACKWARD;
                case SeekPrecision::Any:      return AVSEEK_FLAG_ANY;
            }
        }()
    };
    ffmpeg_api_non_strict(av_seek_frame, raw(), stream_index, timestamp, flags);
    log_info() << "Success seek to " << utils::time_to_string(timestamp, DEFAULT_TIME_BASE);
    return true;
}

Packet InputFormatContext::read() {
    setInterruptTimeout(getTimeout(TimeoutProcess::Reading));
    auto packet { readFromSource() };
    if (packet.isEOF()) {
        return packet;
    }
    if (!processPacket(packet)) {
        return Packet { Media::Type::EndOF };
    }
    return packet;
}

void InputFormatContext::createContext() {
    reset(
        [&]() {
            AVFormatContext* fmt_ctx {
                ::avformat_alloc_context()
            };
            setInterruptCallback(fmt_ctx);
            setInterruptTimeout(getTimeout(TimeoutProcess::Opening));
            fmt_ctx->iformat = inputFormat();
            return fmt_ctx;
        }()
        , [](auto* ctx) { ::avformat_close_input(&ctx); }
    );
}

bool InputFormatContext::openContext(const Options& options) {
    if (!inputFormat()) {
        guessInputFromat();
    }

    Dictionary dictionary { options };
    auto fmt_ctx { raw() };

    ffmpeg_api_non_strict(avformat_open_input
        , &fmt_ctx
        , mediaResourceLocator().data()
        , inputFormat()
        , dictionary.get()
    );

    setInputFormat(raw()->iformat);
    retrieveStreams(options);
    return true;
}

std::string InputFormatContext::formatName() const {
    return std::string { raw()->iformat->name };
}

void InputFormatContext::closeContext() {
    reset();
    setInputFormat(nullptr);
}

void InputFormatContext::retrieveStreams(const Options& options) {
    Dictionary dictionary { options };
    if (const auto ret {
            ::avformat_find_stream_info(raw(), dictionary.get())
        }; ret < 0 ) {
        throw FFmpegException {
            "Failed to retrieve input stream information"
        };
    }
    StreamVector result;
    for (auto i { 0u }; i < raw()->nb_streams; ++i) {
        result.push_back(Stream::make_input_stream(raw()->streams[i]));
        result.back()->params->setFormatFlags(inputFormat()->flags);
    }
    setStreams(result);
}

void InputFormatContext::guessInputFromat() {
    const auto short_name {
        utils::guess_format_short_name(mediaResourceLocator())
    };
    setInputFormat(findInputFormat(short_name));
}

AVInputFormat* InputFormatContext::findInputFormat(const std::string_view short_name) const {
    if (!short_name.empty()) {
        if (std::string { short_name } == "dshow") {
            utils::device_register_all();
        }
        else if (std::string { short_name } == "gdigrab") {
            utils::device_register_all();
        }
        else if (std::string { short_name } == "lavfi") {
            utils::device_register_all();
        }
    }
    return ::av_find_input_format(short_name.data());
}

AVInputFormat* InputFormatContext::inputFormat() {
    return _input_format;
}

void InputFormatContext::setInputFormat(AVInputFormat* in_fmt) {
    _input_format = in_fmt;
}

Packet InputFormatContext::readFromSource() {
    Packet packet;
    if (const auto ret { ::av_read_frame(raw(), packet.ptr()) }; ret < 0) {
        if (ERROR_EOF == ret) {
            return Packet { Media::Type::EndOF };
        }
        throw FFmpegException {
            "Cannot read source: " + utils::quoted(mediaResourceLocator()) + std::to_string(ret)
        };
    }
    return packet;
}

} // namespace fpp

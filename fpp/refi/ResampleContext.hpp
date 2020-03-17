#pragma once
#include <fpp/base/Frame.hpp>
#include <fpp/stream/AudioParameters.hpp>

extern "C" {
    #include <libswresample/swresample.h>
}

namespace fpp {

    class ResampleContext : public SharedFFmpegObject<SwrContext> {

    public:

        ResampleContext(IOParams parameters);
        virtual ~ResampleContext() override = default;

        FrameList           resample(const Frame source_frame);

        const IOParams      params;

    private:

        void                init();
        Frame               createFrame() const;
        void                stampFrame(const Frame& source_frame, Frame& output_frame);

    private:

        int64_t             _samples_count;

    };

} // namespace fpp

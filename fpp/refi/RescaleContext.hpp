#pragma once
#include <fpp/core/wrap/SharedFFmpegObject.hpp>
#include <fpp/stream/VideoParameters.hpp>
#include <fpp/base/Frame.hpp>

extern "C" {
    #include <libswscale/swscale.h>
}

namespace fpp {

    class RescaleContext : public SharedFFmpegObject<SwsContext> {

    public:

        RescaleContext(InOutParams parameters);

        Frame               scale(const Frame source_frame);

        const InOutParams      params;

    private:

        void                init();
        Frame               createFrame() const;

    };

} // namespace fpp

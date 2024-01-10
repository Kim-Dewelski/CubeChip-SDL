
#include "FrameLimiter.hpp"

FrameLimiter::FrameLimiter(
    const double framerate,
    const bool   firstpass,
    const bool   lostframe
) {
    setFreq(framerate, firstpass, lostframe);
}

void FrameLimiter::setFreq(
    const double framerate,
    const bool   firstpass,
    const bool   lostframe
) {
    timeFrequency = 1000.0 / std::clamp(framerate, 0.5, 1000.0);
    skipFirstPass = firstpass;
    skipLostFrame = lostframe;
}

bool FrameLimiter::operator()(const bool state) {
    if (isValidFrame()) return true;

    if (state == SLEEP && remains() >= 2.0)
        std::this_thread::sleep_for(millis(1));
    return false;
}

bool FrameLimiter::isValidFrame() {
    using namespace std::chrono;

    if (!initTimeCheck) [[unlikely]] {
        timePastFrame = steady_clock::now();
        initTimeCheck = true;
    }

    if (skipFirstPass) [[unlikely]] {
        skipFirstPass = false;
        ++validFrameCnt;
        return true;
    }

    timeVariation = {
        timeOvershoot + duration<double, std::milli>
        (steady_clock::now() - timePastFrame).count()
    };

    if (timeVariation < timeFrequency) [[likely]]
        return false;

    if (skipLostFrame)
        timeOvershoot = std::fmod(timeVariation, timeFrequency);
    else
        timeOvershoot = timeVariation - timeFrequency;

    timePastFrame = steady_clock::now();
    ++validFrameCnt;
    return true;
}

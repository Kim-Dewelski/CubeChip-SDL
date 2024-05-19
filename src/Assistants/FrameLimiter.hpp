/*
	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <chrono>
#include <ratio>
#include <thread>
#include <cstdint>
#include <optional>
#include <algorithm>

class FrameLimiter final {
	using chrono = std::chrono::steady_clock::time_point;
	using millis = std::chrono::milliseconds;
	using uint64 = std::uint64_t;

	bool   initTimeCheck{}; // forces timestamp update on first check only
	bool   skipFirstPass{}; // forces valid frame return on first check only
	bool   skipLostFrame{}; // forces frameskip if timeOvershoot > timeFrequency
	bool   lastFrameLost{}; // missed frame indicator when frameskip is enabled

	double timeFrequency{}; // holds time (ms) per unit Hertz
	double timeOvershoot{}; // holds time remainder (ms) from last successful check
	double timeVariation{}; // holds time difference between last check and now
	chrono timePastFrame{}; // holds timestamp of the last frame's check
	uint64 validFrameCnt{}; // counter of successful frame checks performed

	bool isValidFrame();

public:
	FrameLimiter(
		double framerate = 60.0, // 0.5 ... 1000 range
		bool   firstpass = true, // skipFirstPass flag
		bool   lostframe = true  // skipLostFrame flag
	) {
		setLimiter(framerate, firstpass, lostframe);
	}

	FrameLimiter(const FrameLimiter& other)
		: timeFrequency{ other.timeFrequency }
		, skipFirstPass{ other.skipFirstPass }
		, skipLostFrame{ other.skipLostFrame }
	{}

	void setLimiter(
		double              framerate,
		std::optional<bool> firstpass = std::nullopt,
		std::optional<bool> lostframe = std::nullopt
	);

	enum : bool { SPINLOCK, SLEEP };
	bool   check(bool mode = SPINLOCK);

	uint64 count()   const { return validFrameCnt; }
	double elapsed() const { return timeVariation; }
	double remains() const { return timeFrequency - timeVariation; }
	double percent() const { return timeVariation / timeFrequency; }
	bool   paced()   const { return timeOvershoot < timeFrequency && !lastFrameLost; }
};

/*
	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

class HomeDirManager;
class BasicVideoSpec;
class BasicAudioSpec;

class FrameLimiter;
class VM_Guest;

class VM_Host final {
	bool _doBench{};
	s32  _cycles{};

	HomeDirManager& HDM;
	BasicVideoSpec& BVS;
	BasicAudioSpec& BAS;

	[[nodiscard]]
	bool doBench() const noexcept;
	void doBench(bool) noexcept;

	void prepareGuest(VM_Guest&, FrameLimiter&);
	bool eventLoopSDL(VM_Guest&, FrameLimiter&);

public:
	explicit VM_Host(
		const char* const,
		HomeDirManager&,
		BasicVideoSpec&,
		BasicAudioSpec&
	);
	~VM_Host();

	bool runHost();
};

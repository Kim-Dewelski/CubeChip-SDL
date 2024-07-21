/*
	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <SDL3/SDL_main.h>
#include "Includes.hpp"

#include "GuestClass/RomCheck.hpp"

#include <iostream>
#include <iomanip>
#include <chrono>

int32_t SDL_main(int32_t argc, char* argv[]) {

	atexit(SDL_Quit);

#ifdef _DEBUG
	{
		SDL_Version compiled{}; SDL_VERSION(&compiled);
		SDL_Version linked{};   SDL_GetVersion(&linked);

		printf("Compiled with SDL version %d.%d.%d\n", compiled.major, compiled.minor, compiled.patch);
		printf("Linked with SDL version %d.%d.%d\n", linked.major, linked.minor, linked.patch);
	}
#endif

	SDL_SetHint(SDL_HINT_WINDOWS_RAW_KEYBOARD, "0");
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d");
	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "0"); // until the UI is independent
	SDL_SetHint(SDL_HINT_APP_NAME, "CubeChip");

	std::unique_ptr<HomeDirManager> HDM{};
	std::unique_ptr<BasicVideoSpec> BVS{};
	std::unique_ptr<BasicAudioSpec> BAS{};
	std::unique_ptr<VM_Guest>       Guest{};

	try {
		HDM = std::make_unique<HomeDirManager>("CubeChip_SDL");
		BVS = std::make_unique<BasicVideoSpec>();
		BAS = std::make_unique<BasicAudioSpec>();
	} catch (...) { return EXIT_FAILURE; }

	VM_Host      Host(HDM.get(), BVS.get(), BAS.get());
	FrameLimiter Frame(60.0, true, true);
	SDL_Event    Event;

	Host.isReady(
		argc <= 1 ? false :
		HDM->verifyFile(RomFile::validate, argv[1])
	);

reset_all:
	Guest = nullptr;
	kb.updateCopy();
	mb.updateCopy();

	if (Host.isReady()) {
		Guest = std::make_unique<VM_Guest>(HDM.get(), BVS.get(), BAS.get());

		if (Guest->setupMachine()) {
			Frame.setLimiter(Guest->fetchFramerate());
			BVS->changeTitle(HDM->file.c_str());
		} else {
			Frame.setLimiter(30.0);
			Host.isReady(false);
			HDM->reset();
		}
	}

	while (true) {
		while (SDL_PollEvent(&Event)) {
			switch (Event.type) {
				case SDL_EVENT_QUIT:
					return EXIT_SUCCESS;

				case SDL_EVENT_DROP_FILE:
					BVS->raiseWindow();
					if (HDM->verifyFile(RomFile::validate, Event.drop.data)) {
						Host.isReady(true);
						goto reset_all;
					} else {
						blog.stdLogOut("File drop denied: "s + RomFile::error);
						break;
					}

				case SDL_EVENT_WINDOW_MINIMIZED:
					if (Guest) { Guest->isSystemPaused(true); }
					break;

				case SDL_EVENT_WINDOW_RESTORED:
					if (Guest) { Guest->isSystemPaused(false); }
					break;
			}
		}

		if (!Frame.check(Host.doBench()
			? FrameLimiter::SPINLOCK
			: FrameLimiter::SLEEP
		)) continue;

		if (kb.isPressed(KEY(RIGHT))) {
			BAS->changeVolume(+15);
		}
		if (kb.isPressed(KEY(LEFT))) {
			BAS->changeVolume(-15);
		}

		if (Host.isReady()) {
			if (kb.isPressed(KEY(ESCAPE))) {
				Host.isReady(false);
				Host.doBench(false);
				BVS->resetWindow();
				goto reset_all;
			}
			if (kb.isPressed(KEY(BACKSPACE))) {
				goto reset_all;
			}
			if (kb.isPressed(KEY(RSHIFT))) {
				Host.doBench(!Host.doBench());
				std::cout << "\33[1;1H\33[2J\33[?25l"
					<< "Cycle time:      ms |     μs";
			}

			if (kb.isPressed(KEY(PAGEDOWN))) {
				BVS->changeFrameMultiplier(-1);
			}
			if (kb.isPressed(KEY(PAGEUP))) {
				BVS->changeFrameMultiplier(+1);
			}

			if (Host.doBench()) {
				using namespace std::chrono;

				std::cout << "\33[2;1H" << std::dec << std::setfill(' ') << std::setprecision(6)
					<< "\nframe: " << Guest->getTotalFrames() << "   "
					<< "\ncycle: " << Guest->getTotalCycles() << "   "
					<< "\nipf:   " << std::abs(Guest->fetchIPF()) << "   "
					<< (Frame.paced() ? "\n\n > keeping up pace."s : "\n\n > cannot keep up!!"s)
					<< "\n\nelapsed since last: " << Frame.elapsed() << std::endl;

				auto start = high_resolution_clock::now();

				Guest->processFrame();

				auto end = high_resolution_clock::now();

				auto duration = end - start;
				auto ms = duration_cast<std::chrono::milliseconds>(duration);
				auto mu = duration_cast<std::chrono::microseconds>(duration - ms);

				std::cout
					<< "\33[1;13H" << std::setw(4) << ms.count()
					<< "\33[1;23H" << std::setw(3) << mu.count();
			} else { Guest->processFrame(); }
		} else {
			if (kb.isPressed(KEY(ESCAPE))) {
				return EXIT_SUCCESS;
			}
		}

		BVS->renderPresent();

		kb.updateCopy();
		mb.updateCopy();
	}
}

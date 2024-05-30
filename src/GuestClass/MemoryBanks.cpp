/*
	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <utility>

#include "Guest.hpp"
#include "MemoryBanks.hpp"

MemoryBanks::MemoryBanks(VM_Guest* parent)
	: vm{ parent }
{}

void MemoryBanks::modifyViewport(const BrushType type) {
	if (!vm->State.xochip_color) {
		displayBuffer[0].wipeAll();
		return;
	}

	for (auto P{ 0 }; std::cmp_less(P, 4); ++P) {
		if (std::cmp_not_equal(vm->Plane.selected & (1 << P), 0)) {
			switch (type) {
				case BrushType::CLR:
					vm->Mem->displayBuffer[P].wipeAll();
					break;
				case BrushType::XOR:
					for (auto& row : displayBuffer[P]) row ^= 1;
					break;
				case BrushType::SUB:
					for (auto& row : displayBuffer[P]) row &= ~1;
					break;
				case BrushType::ADD:
					for (auto& row : displayBuffer[P]) row |= 1;
					break;
			}
		}
	}
}

void MemoryBanks::flushBuffers(const bool firstFlush) {
	if (firstFlush) {
		for (auto& elem : megaPalette) { elem = {}; }
	} else {
		foregroundBuffer.copyLinear(backgroundBuffer);
	}

	backgroundBuffer.wipeAll();
	collisionPalette.wipeAll();

	vm->flushDisplay();
}

void MemoryBanks::loadPalette(std::uint32_t index, const std::uint32_t count) {
	for (std::size_t idx{ 0 }; std::cmp_less(idx, count); index += 4) {
		megaPalette[++idx] = vm->mrw(index + 0) << 24
			               | vm->mrw(index + 1) << 16
			               | vm->mrw(index + 2) << 8
			               | vm->mrw(index + 3);
	}
}

void MemoryBanks::clearPages(std::uint32_t H) {
	while (std::cmp_less(H++, vm->Plane.H)) {
		displayBuffer[0][H].wipeAll();
	}
}

/*
	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <fstream>

#include "HomeDirManager.hpp"
#include "../Assistants/BasicLogger.hpp"
#include "../Assistants/PathExceptionClass.hpp"
#include "../Assistants/SHA1.hpp"

using namespace blogger;

HomeDirManager::HomeDirManager(const char* homeName) try
	: BasicHome{ homeName }
{
	try {
		blog.setStdLogFile("program.log", getHome());
		blog.setDbgLogFile("debug.log", getHome());
		addDirectory();
	} catch (const std::exception& e) {
		BasicHome::showErrorBox(e.what(), "Fatal Initialization Error");
		throw;
	}
} catch (const std::exception& e) {
	BasicHome::showErrorBox(e.what(), "Fatal Initilization Error");
	throw;
}

void HomeDirManager::reset() noexcept {
	path = name = type = sha1 = {};
	size = 0;
}

void HomeDirManager::addDirectory() {
	permRegs = getHome() / "permRegs";
	std::filesystem::create_directories(permRegs);
	if (!std::filesystem::exists(permRegs)) {
		throw PathException("Could not create subdir: ", permRegs);
	}
}

bool HomeDirManager::verifyFile(
	bool(*validate)(std::uint64_t fsize, std::string_view type, std::string_view sha1),
	const char* filepath
) {
	if (!filepath) { return false; }
	namespace fs = std::filesystem;

	const fs::path fspath{ filepath };
	blog.stdLogOut("Attempting to access file: " + fspath.string());

	if (!fs::exists(fspath) || !fs::is_regular_file(fspath)) {
		blog.dbgLogOut("Unable to use locate path: " + fspath.string());
		return false;
	}

	std::ifstream fileStream(fspath);
	if (!fileStream.is_open()) {
		blog.dbgLogOut("Failed to open file: " + fspath.string());
		return false;
	}
	if (!fileStream.good()) {
		blog.dbgLogOut("File is not readable: " + fspath.string());
		return false;
	}

	std::error_code error;
	const std::uint64_t fileSize{ fs::file_size(fspath, error) };
	if (error) {
		blog.dbgLogOut("Unable to access file: " + fspath.string());
		return false;
	}
	if (fileSize == 0) {
		blog.dbgLogOut("File is empty: " + fspath.string());
		return false;
	}

	auto tempPath{ fspath.string() };
	auto tempType{ fspath.extension().string() };
	auto tempSHA1{ SHA1::from_file(tempPath) };

	const bool result{ validate(fileSize, tempType, tempSHA1) };

	if (result) {
		path = tempPath;
		file = fspath.filename().string();
		name = fspath.stem().string();
		type = tempType;
		sha1 = tempSHA1;
		size = fileSize;
	}

	return result;
}

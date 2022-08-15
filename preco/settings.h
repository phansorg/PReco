#pragma once

#include <nlohmann/json.hpp>

// Singleton
class settings
{
	settings() = default;

	static settings* instance_;

public:
	static settings* get_instance() {
		if (instance_ == nullptr) instance_ = new settings();
		return instance_;
	}

	nlohmann::json json;
	static bool debug;

	void init();
	[[nodiscard]] std::string dump() const;
};

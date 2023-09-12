#pragma once

#include <nlohmann/json.hpp>

// Singleton
class Settings
{
public:
	static constexpr int mse_init_ = 10000;
	static constexpr int history_max_ = 5;

private:
	Settings() = default;

	static Settings* instance_;

public:
	static Settings* get_instance() {
		if (instance_ == nullptr) instance_ = new Settings();
		return instance_;
	}

	nlohmann::json json_;
	static bool debug_;

	void init();
	[[nodiscard]] std::string dump() const;
};

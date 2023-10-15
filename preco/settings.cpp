#include "settings.h"

#include <fstream>

#include "capture_thread.h"
#include "game_thread.h"

constexpr auto settings_file_name = "settings.json";
constexpr auto json_indent = 4;

Settings* Settings::instance_ = nullptr;
#ifdef _DEBUG
bool Settings::debug_ = true;
#else
bool Settings::debug_ = false;
#endif


void Settings::init()
{
    /*
    // �ݒ�t�@�C�������݂���ꍇ�A�ǂݍ���
	if (std::filesystem::exists(settings_file_name)) {
	    std::ifstream ifs(settings_file_name);
        json = nlohmann::json::parse(ifs);
	    return;
	}
	*/

    // �ݒ�t�@�C�������݂��Ȃ��ꍇ�A�f�t�H���g�l�ō쐬
	json_["capture_mode"] = CaptureMode::kJpeg;
    json_["capture_path"] = "D:/puyo/movie/image_target";
    json_["capture_start_no"] = 1;
    //json_["capture_start_no"] = 3572;
    //json_["capture_start_no"] = 5924;
    json_["capture_last_no"] = 105435;

    json_["log_path"] = "D:/puyo/movie/logs/log.txt";

    json_["player_history_dir"] = "D:/puyo/movie/history";

	// �Ղ�̍��[
    json_["player_field_x"] = { 220, 886 };
    // �t�B�[���h�̏�[
    json_["player_field_y"] = 309;
    // �Ղ�̉E�[�܂�
    json_["player_field_w"] = 264;
    // �t�B�[���h�̉��[�܂�
    json_["player_field_h"] = 486;

    // �l�N�X�g�̍��[
	json_["player_nxt1_x"] = { 522, 805 };
    // �l�N�X�g�̏�[
    json_["player_nxt1_y"] = 312;

    // �l�N�l�N�̍��[
    json_["player_nxt2_x"] = { 558, 779 };
    // �l�N�l�N�̏�[
    json_["player_nxt2_y"] = 401;

    // �X�R�A�ŏ��0�̍��[
	json_["player_score_x"] = { 274, 883 };
    // �X�R�A�ŏ��0�̏�[
	json_["player_score_y"] = 799;

    json_["game_debug_write"] = true;
    json_["game_debug_path"] = "D:/puyo/movie/image_debug";
    json_["game_start_mode"] = GameMode::kWaitCharacterSelection;
    //json_["game_start_mode"] = GameMode::kWaitGameReset;

    json_["cell_debug_path"] = "D:/puyo/movie/image_debug_cell";

    std::ofstream ofs(settings_file_name);
    ofs << std::setw(json_indent) << json_ << std::endl;
}

std::string Settings::dump() const
{
    return json_.dump(json_indent);
}

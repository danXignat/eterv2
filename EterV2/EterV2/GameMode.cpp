#include "GameMode.h"

#include <unordered_set>

namespace base {

	GameMode::GameMode() : m_current_player{ 0 } {

	}

	//--------------------------------------------------Methods-----------------------------------

	void GameMode::_switchPlayer() {
		m_current_player = !m_current_player;
	}

	Player& GameMode::_curentPlayer() {
		return (m_current_player == 0) ? m_player1.value() : m_player2.value();
	}


	////----------------------------------------------Win Manager-----------------------------------------------

	GameMode::WinManager::WinManager(uint16_t size) : size{ size } {

	}

	bool GameMode::WinManager::won(const Coord& coord, const Player& player, const Board& board) {
		const auto& [x, y] = coord;

		int16_t increment = (player.getColor() == color::ColorType::BLUE) ? 1 : -1;

		rows[y] += increment;
		cols[x] += increment;

		if (abs(rows[y]) == size || abs(cols[x]) == size) {
			return true;
		}

		if (board.isFixed()) {
			//board.diagsCount() -> pair <nr elemente diag1, nr element diag2>
		}

		return false;
	}

	//-------------------------------------------Input Manager---------------------------------------------

	void GameMode::Input::read() {
		std::unordered_set<char> available_moves{ '1', '2', '3', '4' };

		std::string input;
		std::getline(std::cin, input);

		std::istringstream stream{ input };
		std::string token;
		char del{ ' ' };

		std::getline(stream, token, del);

		std::cout << token;
		if (utils::isValidPos(token)) {
			x = std::stoi(token);
		}
		else {
			throw std::invalid_argument("number is needed");
		}

		std::getline(stream, token, del);
		if (utils::isValidPos(token)) {
			y = std::stoi(token);
		}
		else {
			throw std::invalid_argument("number is needed");
		}

		std::getline(stream, token, del);
		if (token.size() == 1 && available_moves.contains(token[0])) {
			card_type = token[0];
		}
		else {
			throw std::invalid_argument("not valid choice");
		}

		std::getline(stream, token, del);
		if (!token.empty() && token.size() == 1 && token[0] == 'i') {
			illusion = true;
		}
		else {
			illusion = false;
		}
	}
}

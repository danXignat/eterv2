#include "TrainingMode.h"

#include <format>

namespace base {
	//---------------------------------------Constructor-------------------------------------
	TrainingMode::TrainingMode(bool illusion, bool explosion, const std::string& player1_name, const std::string& player2_name)
		: m_board{ 3 },
		m_win_manager{ 3 }
	{	
		m_player1.emplace(player1_name, color::ColorType::RED);
		m_player2.emplace(player2_name, color::ColorType::BLUE );

		if (illusion) {
			m_illusion_service = IllusionService();
		}

		if (explosion) {
			m_explosion_service = ExplosionService();
		}
	}

	//---------------------------------------Events----------------------------------------------

	void TrainingMode::run() {
		bool win = false;
		m_board.renderBoard();

		while (win == false) {
			Input input; //asta da handle la inputu de la tastatura
			input.read();

			CombatCardType type = _fromCharToType(input.card_type);
			Player& current_player = _curentPlayer();

			auto card = current_player.getCard(type, input.illusion); //optional cu cartea se extrage cartea din invetaru playerului

			if (card) {
				m_board.appendMove(
					{ input.x, input.y }, std::move(*card)
				);

				win = m_win_manager.won(
					{ input.x, input.y }, current_player, m_board
				);

				_switchPlayer();
			}
			else {
				
			}

			system("cls");
			m_board.renderBoard();
		}

		std::cout << std::format("Player {}",
			(_curentPlayer().getColor() == color::ColorType::BLUE) ? "BLUE" : "RED"
		);
		std::cin.get();
	}

	////------------------------------------------------Methods-------------------------------------------------

	CombatCardType TrainingMode::_fromCharToType(char ch) {
		using enum CombatCardType;

		std::unordered_map<char, CombatCardType> switch_map{
				{'1', ONE},
				{'2', TWO},
				{'3', THREE},
				{'4', FOUR}
		};

		return switch_map[ch];
	}
}
#include "TrainingMode.h"

#include <format>

#include "InputHandler.h"
#include "logger.h"

using namespace logger;

namespace base {
	//---------------------------------------Constructor-------------------------------------
	TrainingMode::TrainingMode(const std::vector<ServiceType>& services, const std::pair<std::string, std::string>& player_names) :
		BaseGameMode{ GameSizeType::SMALL, player_names }
	{
	
		for (ServiceType service : services) {
			switch (service) {
				using enum ServiceType;
			case ILLUSION:
				m_illusion_service.emplace(m_board, m_win_manager);
				break;
			case EXPLOSION:
				m_explosion_service.emplace(m_board, m_player_red, m_player_blue);
				break;
			default:
				break;
			}
		}
	}

	//---------------------------------------Events----------------------------------------------

	void TrainingMode::run() {
		this->render();

		while (m_win_manager.won() == false) {
			InputHandler input;
			try {
				input.read();								//asta da handle la inputu de la tastatura
			}
			catch (const std::runtime_error& err) {
				Logger::log(Level::ERROR, err.what());
				system("cls");
				this->render();
				continue;
			}

			if (_handleEvent(input)) {
				Logger::log(Level::INFO, "succesful action");
				_switchPlayer();
			}
			else {
				Logger::log(Level::INFO, "failed action");
			}

			this->render();
		}

		if (m_curr_player.get().getColor() == color::ColorType::BLUE) {
			std::cout << "Player RED has won";
		}
		else {
			std::cout << "Player BLUE has won";
		}

		std::cin.get();
	}

	bool TrainingMode::_handleSpecialEvent(const InputHandler& input) {

		switch (input.service_type) {
			using enum ServiceType;

		case ILLUSION: {
			if (m_illusion_service) {
				Coord coord = input.coord;
				CombatCardType card_type = input.card_type;

				CombatCard card = m_curr_player.get().getCard(card_type);
				card.flip();

				bool has_illusion = m_illusion_service->hasIllusion(m_curr_player.get());
				bool valid_move = m_illusion_service->isValidPlaceCard(coord, card);

				if (has_illusion && valid_move) {
					m_illusion_service->placeIllusion(coord, std::move(card));

					return true;
				}
				else {
					card.flip();

					m_curr_player.get().addCard(std::move(card));
				}
			}
			break;
		}

		case EXPLOSION:{
			if (m_explosion_service) {
				m_explosion_service->setting();
				m_explosion_service->apply();

				return true;
			}
			break;
		}

		default:
			break;
		}

		return false;
	}

	////------------------------------------------------Methods-------------------------------------------------

	void TrainingMode::render() {
		system("cls");

		m_board.render();
		m_board.sideViewRender();
		m_player_red.renderCards();
		m_player_blue.renderCards();

		if (m_curr_player.get().getColor() == color::ColorType::RED) {
			std::cout << color::to_string(color::ColorType::RED);
			utils::printAtCoordinate("RED turn", { 1, 12 });
			std::cout << color::to_string(color::ColorType::DEFAULT);
		}
		else {
			std::cout << color::to_string(color::ColorType::BLUE);
			utils::printAtCoordinate("BLUE turn", { 1, 12 });
			std::cout << color::to_string(color::ColorType::DEFAULT);
		}

		if (m_explosion_service) {
			m_explosion_service->render_explosion();
		}
	}
	
}
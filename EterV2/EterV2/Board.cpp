﻿#include "Board.h"

#include <queue>
#include <ranges>

#include "logger.h"

using namespace logger;

namespace base {
	//----------------------------Constructors--------------------------------

	Board::Board(uint16_t size, Player& player1, Player& player2) :
		m_size{ size },
		m_bounding_rect{ size },
		m_available_spaces{ Coord{10, 5} },
		m_adjacency_offsets{ {
			{2, 0}, {2, -1}, {0, -1}, {-2, -1},
			{-2, 0}, {-2, 1}, {0, 1}, {2, 1}
		} },
		m_player1{ player1 }, m_player2{ player2 } {

	}

	Board::Board(GameSizeType size_type, Player& player1, Player& player2) :
		m_size{ static_cast<uint16_t>(size_type) },
		m_bounding_rect{ static_cast<uint16_t>(size_type) },
		m_available_spaces{Board::START_POS},
		m_adjacency_offsets{ {
			{2, 0}, {2, -1}, {0, -1}, {-2, -1},
			{-2, 0}, {-2, 1}, {0, 1}, {2, 1}
		} },
		m_player1{ player1 }, m_player2{ player2 } {

	}
	

	//----------------------------MODIFIERS--------------------------------

	void Board::appendMove(const Coord& coord, CombatCard&& card) {
		if (this->isValidPlaceCard(coord, card) == false) {
			throw std::runtime_error("move not valid");
		}

		m_bounding_rect.add(coord);

		m_combat_cards[coord].emplace_back(std::move(card));

		_updateAvailableSpaces(coord);

		Logger::log(
			Level::INFO,
			"card({}) appended at ({}, {})",
			combatCardToChar(m_combat_cards[coord].back().getType()), coord.first, coord.second
		);
	}

	void Board::moveRow(uint16_t from_y, uint16_t to_y) {
		if (from_y == to_y) {
			Logger::log(Level::WARNING, "Source and destination rows are the same.");
			return;
		}
		std::vector<std::pair<Coord, std::vector<CombatCard>>> rowData;
		for (auto it = m_combat_cards.begin(); it != m_combat_cards.end();) {
			if (it->first.second == from_y) {
				rowData.emplace_back(it->first, std::move(it->second));
				it = m_combat_cards.erase(it);
			}
			else {
				++it;
			}
		}
		for (auto& pair : rowData) {
			Coord newCoord = { pair.first.first, to_y };
			m_combat_cards[newCoord] = std::move(pair.second);
		}
		_reinitialise();
		Logger::log(Level::INFO, "Moved row {} to {}", from_y, to_y);
	}


	void Board::moveColumn(uint16_t from_x, uint16_t to_x) {
		if (from_x == to_x) {
			Logger::log(Level::WARNING, "Source and destination columns are the same.");
			return;
		}

		std::vector<Coord> from_coords;

		for (auto it = m_combat_cards.begin(); it != m_combat_cards.end(); ) {
			if (it->first.first == from_x) {
				from_coords.push_back(it->first);
				it = m_combat_cards.erase(it);
			}
			else {
				++it;
			}
		}

		for (const auto& [x, y] : from_coords) {
			Coord new_coord{ to_x, y };

			m_combat_cards[new_coord] = std::move(
				m_combat_cards.extract({ x, y }).mapped()
			);
		}

		_reinitialise();

		Logger::log(Level::INFO, "Moved column {} to {}", from_x, to_x);
	}



	void Board::moveStack(const Coord& from_coord, const Coord& to_coord) {
		/*if (m_combat_cards.contains(to_coord)) {
			Logger::log(Level::ERROR, "Destination must be an empty space!\n", from_coord.first, from_coord.second);
			return false;
		}
		if (!m_available_spaces.contains(to_coord)) {
			Logger::log(Level::ERROR, "Destination is not a valid space!\n", from_coord.first, from_coord.second);
			return false;
		}
		auto it = m_combat_cards.find(from_coord);
		if (it == m_combat_cards.end()) {
			Logger::log(Level::ERROR, "No stack at ({},{})\n", from_coord.first, from_coord.second);
			return false;
		}

		m_combat_cards[to_coord] = std::move(it->second);
		m_combat_cards.erase(it);

		_reinitialise();

		Logger::log(Level::INFO, "Stack moved from ({}, {}) to ({}, {})",
			from_coord.first, from_coord.second, to_coord.first, to_coord.second);
		return true;*/
	}


	void Board::swapStacks(const Coord& from_coord, const Coord& to_coord) {
		auto from_it = m_combat_cards.find(from_coord);
		if (from_it == m_combat_cards.end()) {
			Logger::log(Level::ERROR, "No stack at ({},{})\n", from_coord.first, from_coord.second);
			return;
		}

		auto to_it = m_combat_cards.find(to_coord);
		if (to_it == m_combat_cards.end()) {
			Logger::log(Level::ERROR, "No stack at ({},{})\n", to_coord.first, to_coord.second);
			return;
		}

		auto& from_stack = from_it->second;
		auto& to_stack = to_it->second;

		std::vector<CombatCard> temp = std::move(from_stack);
		from_stack = std::move(to_stack);
		to_stack = std::move(temp);

		_reinitialise();

		Logger::log(Level::INFO, "Stacks swapped between ({}, {}) and ({}, {})",
			from_coord.first, from_coord.second, to_coord.first, to_coord.second);
	}

	CombatCard&& Board::popTopCardAt(const Coord& coord) {
		if (m_combat_cards.contains(coord) == false) {
			throw std::runtime_error("not a valid pos");
		}

		if (m_combat_cards[coord].size() == 1 && this->isValidRemoveStack(coord) == false) {
			throw std::runtime_error("not a valid pos");
		}

		CombatCard card = std::move(m_combat_cards[coord].back());
		m_combat_cards[coord].pop_back();

		if (m_combat_cards[coord].empty()) {
			m_combat_cards.erase(coord);

			_reinitialise();
		}

		return std::move(card);
	}

	CombatCard&& Board::popCardFromStackAt(const Coord& coord, const CombatCard& card_to_remove) {
		if (m_combat_cards.contains(coord) == false) {
			throw std::runtime_error("not a valid pos");
		}

		auto& stack = m_combat_cards[coord];
		auto card_it = std::find(stack.begin(), stack.end(), card_to_remove);

		if (card_it == stack.end()) {
			throw std::runtime_error("card not found");
		}

		if (m_combat_cards[coord] == 1) {
			throw std::runtime_error("not conex anymore");
		}

		CombatCard card = std::move(*card_it);

		stack.erase(card_it);

		if (stack.empty()) {
			m_combat_cards.erase(coord);

			_reinitialise();
		}

		Logger::log(Level::INFO, "Removed card from the stack at ({}, {})", coord.first, coord.second);

		return std::move(card);
	}

	Board::Stack Board::popStack(const Coord& coord) {
		if (m_combat_cards.contains(coord) == false) {
			throw std::runtime_error("not a valid pos");
		}

		if (this->isValidRemoveStack(coord) == false) {
			throw std::runtime_error("not conex anymore");
		}

		Stack stack = std::move(m_combat_cards[coord]);
		m_combat_cards.erase(coord);

		_reinitialise();

		Logger::log(Level::INFO, "stack removed at ({}, {})", coord.first, coord.second);

		return stack;
	}
	std::vector<Board::Stack> Board::popRow(uint16_t y) {

	}

	std::vector<Board::Stack> Board::popColumn(uint16_t x) {

	}

	void Board::_addUsedCard(CombatCard&& card) {
		if (card.getColor() == color::ColorType::RED) {
			m_player1.addUsedCard(std::move(card));
		}
		else {
			m_player2.addUsedCard(std::move(card));
		}
	}

	void Board::removeTopCardAt(const Coord& coord) {
		CombatCard card = popTopCardAt(coord);

		_addUsedCard(std::move(card));
	}

	void Board::removeCardFromStackAt(const Coord& coord, const CombatCard& card_to_remove) {
		CombatCard card = popCardFromStackAt(coord);

		_addUsedCard(std::move(card));
	}

	void Board::removeStack(const Coord& coord) {
		
	}

	void Board::removeRow(uint16_t y) {
		if (m_bounding_rect.corner1.second <= y && y <= m_bounding_rect.corner2.second) {
			for (auto it = m_combat_cards.begin(); it != m_combat_cards.end(); ) {
				if (it->first.second == y) {
					it = m_combat_cards.erase(it);
				}
				else {
					it++;
				}
			}
			_reinitialise();
		}
		else {
			Logger::log(Level::ERROR, "no card on row{}", y);
		}
	}

	void Board::removeColumn(uint16_t x) {
		if (m_bounding_rect.corner1.first <= x && x <= m_bounding_rect.corner2.first) {
			for (auto it = m_combat_cards.begin(); it != m_combat_cards.end(); ) {
				if (it->first.first == x) {
					it = m_combat_cards.erase(it);
				}
				else {
					it++;
				}
			}

			_reinitialise();
		}
		else {
			Logger::log(Level::ERROR, "no card on row{}", x);
		}
	}

	

	const std::unordered_set<Coord, CoordFunctor>& Board::availableSpaces() const {
		return m_available_spaces;
	}

	//-----------------------------------------------------CHECKERS-----------------------------------------------------


	bool Board::isValidPlaceCard(const Coord& coord, const CombatCard& card) {

		if (m_combat_cards.contains(coord)) {
			if (card.getType() == CombatCardType::ETER) {
				Logger::log(Level::WARNING, "This card must be played on an empty space");
				return false;
			}

			if (this->getTopCard(coord)->get() >= card ) {
				Logger::log(Level::WARNING, "Card too small");
				return false;
			}
		}
		else if (m_available_spaces.contains(coord) == false) {
			Logger::log(Level::WARNING, "not available place");
			return false;
		}

		return true;
	}

	bool Board::isValidRemoveStack(const Coord& remove_coord) const {
		if (m_combat_cards.contains(remove_coord) == false) {
			Logger::log(Level::WARNING, "not available place to remove");
			return false;
		}

		std::unordered_set<Coord, CoordFunctor> new_config{
			std::ranges::begin(m_combat_cards | std::views::keys),
			std::ranges::end(m_combat_cards | std::views::keys)
		};

		new_config.erase(remove_coord);

		return _isConex(new_config);
	}

	bool Board::isValidMoveStack(const Coord& from_coord, const Coord& to_coord) const {
		if (m_combat_cards.contains(from_coord) == false) {
			Logger::log(Level::WARNING, "not available stack to move");
			return false;
		}

		if (m_available_spaces.contains(to_coord) == false) {
			Logger::log(Level::WARNING, "not available palce to move stack");
			return false;
		}

		if (from_coord == to_coord) {
			Logger::log(Level::WARNING, "choose a valid place");
			return false;
		}

		std::unordered_set<Coord, CoordFunctor> new_config{
			std::ranges::begin(m_combat_cards | std::views::keys),
			std::ranges::end(m_combat_cards | std::views::keys)
		};

		new_config.erase(from_coord);
		new_config.insert(to_coord);

		return _isConex(new_config);
	}

	bool Board::_isConex(const std::unordered_set<Coord, CoordFunctor>& board_coords) const { // bfs 
		std::unordered_set<Coord, CoordFunctor> visited;
		std::queue<Coord> queue;

		Coord first_coord{ *board_coords.begin() };
		queue.push(first_coord);
		visited.insert(first_coord);


		for (; queue.empty() == false; queue.pop()) {
			Coord curr = queue.front();

			for (const auto& [x, y] : m_adjacency_offsets) {
				Coord adjacent_coord{ curr.first + x, curr.second + y };

				bool not_visited = visited.contains(adjacent_coord) == false;
				bool is_on_board = board_coords.contains(adjacent_coord);

				if (not_visited && is_on_board) {
					visited.insert(adjacent_coord);
					queue.push(adjacent_coord);
				}
			}
		}

		return visited.size() == board_coords.size(); // conex
	}

	///------------------------------Setter Getter------------------------------------

	uint16_t Board::size() const {
		return m_size;
	}

	bool Board::isFixed() const {
		return m_bounding_rect.isFixed();
	}

	bool Board::fixedWidth() const {
		return m_bounding_rect.fixed_width;
	}

	bool Board::fixedHeight() const {
		return m_bounding_rect.fixed_height;
	}

	bool Board::hasStack(const Coord& coord) const {
		return m_combat_cards.contains(coord);
	}

	bool Board::isCardOfColorAt(color::ColorType color, const Coord& coord) const {
		auto& stack = m_combat_cards.at(coord);

		auto is_color = [color](const CombatCard& card) { return card.getColor() == color; };

		auto it = std::find_if(stack.begin(), stack.end(), is_color);

		if (it != stack.end()) {
			return true;
		}

		return false;
	}

	uint16_t Board::getSize() const {
		return m_size;
	}

	const std::vector<CombatCard>& Board::operator[](const Coord& coord) {
		return m_combat_cards.at(coord);
	}

	std::optional<CombatCardRef> Board::getTopCard(const Coord& pos) {
		if (m_combat_cards.contains(pos)) {
			return std::ref(m_combat_cards[pos].back());
		}
		return std::nullopt;
	}

	Coord Board::getLeftCorner() const {
		return m_bounding_rect.corner1;
	}

	std::pair<Coord, Coord> Board::getBoudingRect() const {
		return { m_bounding_rect.corner1, m_bounding_rect.corner2 };
	}

	std::pair<uint16_t, uint16_t> Board::getBoundingRectSize() const {
		return {
			(m_bounding_rect.corner2.first - m_bounding_rect.corner1.first + 2) / 2,
			m_bounding_rect.corner2.second - m_bounding_rect.corner1.second + 1
		};
	}

	///----------------------------Private Methods--------------------------------


	void Board::_updateAvailableSpaces(const Coord& coord) {
		m_available_spaces.erase(coord);

		for (const auto& offset : m_adjacency_offsets) {
			Coord new_point{ coord.first + offset.first, coord.second + offset.second };

			if (!m_combat_cards.contains(new_point)) {
				m_available_spaces.emplace(new_point);
			}
		}

		if (m_bounding_rect.fixed_width) {
			std::erase_if(m_available_spaces, [&](Coord coord) {
				return !m_bounding_rect.withinWidth(coord);
				});
		}

		if (m_bounding_rect.fixed_height) {
			std::erase_if(m_available_spaces, [&](Coord coord) {
				return !m_bounding_rect.withinHeight(coord);
				});
		}
	}

	void Board::_reinitialise() {
		m_available_spaces.clear();

		if (m_combat_cards.empty()) {
			m_bounding_rect = BoundingRect(m_size);
			m_available_spaces.insert({ 10, 5 });
		}
		else {
			m_bounding_rect = BoundingRect(m_size, m_combat_cards.begin()->first);

			for (const auto& [coord, stack] : m_combat_cards) {
				m_bounding_rect.add(coord);

				_updateAvailableSpaces(coord);
			}
		}
	}

	/// ---------------------------------------------------CONSOLE-RENDER--------------------------------------------------


	void Board::render() const {
		for (const auto& [coord, cards] : m_combat_cards) {
			utils::printAtCoordinate(cards.back(), coord.first, coord.second);
		}

		for (const auto& [x, y] : m_available_spaces) {
			utils::printAtCoordinate("*", x, y);
		}

	}
	void Board::sideViewRender() {
		Coord print_pos{ 30, 1 };

		std::vector<std::pair<Coord, std::vector<CombatCardRef>>> cards;

		for (auto& [coord, stack] : m_combat_cards) {
			std::vector<CombatCardRef> card_refs;

			for (CombatCard& card : stack) {
				card_refs.emplace_back(std::ref(card));
			}

			cards.emplace_back(coord, card_refs);
		}


		std::sort(cards.begin(), cards.end(), [](const auto& el1, const auto& el2) {
			return el1.first < el2.first; // Sort based on coordinates
			});

		utils::printAtCoordinate("List of cards", print_pos.first, print_pos.second);
		uint16_t i = 1;
		for (const auto& [coord, stack] : cards) {
			std::string pos = std::format("({}, {}) -> ", coord.first, coord.second);

			utils::printAtCoordinate(pos, print_pos.first, print_pos.second + i);
			uint16_t j = 0;
			for (const auto& card : stack) {
				utils::printAtCoordinate(card.get(),  // Dereference the unique_ptr
					print_pos.first + uint16_t(pos.size()) + j,
					print_pos.second + i
				);
				j += 2;
			}

			i++;
		}
	}

	///--------------------------------Inner Classes-------------------------------------

	Board::BoundingRect::BoundingRect()
		: size{ 0 },
		corner1{ 0, 0 }, corner2{ 0, 0 },
		fixed_width{ false }, fixed_height{ false } {

	}

	Board::BoundingRect::BoundingRect(uint16_t size) :
		size{ size },
		corner1{ 10, 5 }, corner2{ 10, 5 },
		fixed_width{ false }, fixed_height{ false } {

	}

	Board::BoundingRect::BoundingRect(uint16_t size, const Coord& coord) :
		size{ size },
		corner1{ coord.first, coord.second }, corner2{ coord.first, coord.second },
		fixed_width{ false }, fixed_height{ false } {

	}

	void Board::BoundingRect::add(const Coord& coord) {
		if (!fixed_width) {
			corner1.first = std::min(corner1.first, coord.first);
			corner2.first = std::max(corner2.first, coord.first);
		}

		if (!fixed_height) {
			corner1.second = std::min(corner1.second, coord.second);
			corner2.second = std::max(corner2.second, coord.second);
		}

		fixed_width = ((corner2.first - corner1.first) / 2 + 1 == size) ? true : false;
		fixed_height = (corner2.second - corner1.second + 1 == size) ? true : false;
	}

	void Board::BoundingRect::remove(const Coord& coord) {

	}

	bool Board::BoundingRect::withinWidth(const Coord& coord) const {
		return corner1.first <= coord.first && coord.first <= corner2.first;
	}

	bool Board::BoundingRect::withinHeight(const Coord& coord) const {
		return corner1.second <= coord.second && coord.second <= corner2.second;
	}

	bool Board::BoundingRect::within(const Coord& coord) const {
		return withinHeight(coord) && withinWidth(coord);
	}

	bool Board::BoundingRect::isFixed() const {
		return fixed_width && fixed_height;
	}


	///-----------------------------------------Iterator-------------------------------

	Board::const_iterator Board::begin() const {
		return m_combat_cards.begin();
	}

	Board::const_iterator Board::end() const {
		return m_combat_cards.end();
	}

}
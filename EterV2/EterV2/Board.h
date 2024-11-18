#pragma once

#include <unordered_map>
#include <unordered_set>

#include "CombatCard.h"

#include "utils.h"
#include "colors.h"

using namespace utils;

namespace base {
	
	class Board {
	public:
		Board(uint16_t);
		void appendMove(Coord, CardPtr&&);
		void renderBoard() const;
		std::pair<int16_t, int16_t> getCountDiag();
		uint16_t getSize() const;
		bool isFixed() const;

	private:
		void _updateAvailableSpaces(Coord);
		bool _isValidPos(Coord) const;
		bool _isValidMove(Coord, const CombatCard&);

	private:
		struct CoordFunctor {
			size_t operator()(const Coord&) const;
		};

		struct BoundingRect {
			uint16_t size;
			Coord corner1, corner2;
			bool fixed_width, fixed_height;

			BoundingRect(uint16_t);
			void add(Coord);
			void remove(Coord);
			bool withinWidth(Coord) const;
			bool withinHeight(Coord) const;
			bool within(Coord) const;
			bool isFixed() const;
		};

	private:
		uint16_t m_size;
		BoundingRect m_bounding_rect;

		std::unordered_map<Coord, std::vector<CardPtr>, CoordFunctor> m_combat_cards;
		std::unordered_set<Coord, CoordFunctor> m_available_spaces;
	};	
}
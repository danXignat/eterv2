#pragma once

#include <iostream>
#include <cstdint>

#include "colors.h"

namespace base {
	enum class CombatCardType : uint16_t {
		ONE = 1,
		TWO = 2,
		THREE = 3,
		FOUR = 4,
		ETER = 5
	};

	class CombatCard {
	public:
		CombatCard(CombatCardType type,	color::ColorType color);
		CombatCardType getType() const;

		color::ColorType getColor() const;
		void revealIllusion();
		void setIllusion();
		bool isIllusion() const;

		bool attemptCover(const CombatCard& opponentCard) const;

		bool operator<(const CombatCard& other) const;
		bool operator==(const CombatCard& other) const;

		friend std::ostream& operator<<(std::ostream& out, const CombatCard& other);

	private:
		CombatCardType m_type;
		bool m_illusion;
		color::ColorType m_color;

	};
}
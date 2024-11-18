#pragma once
#include <unordered_map>
#include <memory>
#include <string_view>
#include <optional>


#include "CombatCard.h"

#include "colors.h"
#include "typedefs.h"

namespace base {

    class Player {
    public:
        Player(std::string_view, color::ColorType);

        std::string getName() const;
        color::ColorType getColor() const;
        std::optional<CardPtr> getCard(CombatCardType, bool = false);
        bool hasCards() const;

        void setName(std::string_view name);
        void addCard(std::unique_ptr<CombatCard>&&);

        /*void setMageCard(std::unique_ptr<MageCard>&& mageCard);
        MageCard* getMageCard();
        bool useMageCard();
        bool hasUnusedMageCard() const;*/

    private:
        void _initializeCards();
        void _addCard(CombatCardType, uint16_t);

    private:
        color::ColorType m_color;
        std::string m_name;
        std::unordered_multimap<CombatCardType, CardPtr> m_cards;
        bool m_illusion_used;

        /*std::unique_ptr<MageCard> m_mage_card;
        bool m_mage_card_used;*/
    };
}
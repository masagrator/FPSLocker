#include "langs/en.hpp"
#include "langs/de.hpp"
#include "langs/fr.hpp"
#include "langs/ru.hpp"
#include "langs/zhcn.hpp"
#include "langs/ptbr.hpp"
#include <switch.h>
#include "Langs.hpp"

static_assert(Lang::Id_total_number == ENG::strings.size());
static_assert(Lang::Id_total_number == GER::strings.size());
static_assert(Lang::Id_total_number == ZHCN::strings.size());
static_assert(Lang::Id_total_number == FRA::strings.size());
static_assert(Lang::Id_total_number == RUS::strings.size());
static_assert(Lang::Id_total_number == PTBR::strings.size());

SetLanguage language = SetLanguage_ENUS;

const char* getStringID(std::size_t id) {

    switch(language) {
        case SetLanguage_DE:
            return GER::strings[id];
        case SetLanguage_ZHCN:
        case SetLanguage_ZHHANS:
            return ZHCN::strings[id];
        case SetLanguage_FR:
            return FRA::strings[id];
        case SetLanguage_RU:
            return RUS::strings[id];
        case SetLanguage_PTBR:
            return PTBR::strings[id];
        case SetLanguage_ZHTW:
        case SetLanguage_ZHHANT:
        case SetLanguage_IT:
        case SetLanguage_JA:
        case SetLanguage_ES:
        case SetLanguage_KO:
        case SetLanguage_NL:
        case SetLanguage_PT:
        case SetLanguage_FRCA:
        case SetLanguage_ES419:
        case SetLanguage_ENUS:
        case SetLanguage_ENGB:
        default:
            return ENG::strings[id];
    }
}

const char* getTeslaStringID(std::size_t id) {

    switch(language) {
        case SetLanguage_DE:
            return GER::teslaStrings[id];
        case SetLanguage_ZHCN:
        case SetLanguage_ZHHANS:
            return ZHCN::teslaStrings[id];
        case SetLanguage_FR:
            return FRA::teslaStrings[id];
        case SetLanguage_RU:
            return RUS::teslaStrings[id];
        case SetLanguage_PTBR:
            return PTBR::teslaStrings[id];
        case SetLanguage_ZHTW:
        case SetLanguage_ZHHANT:
        case SetLanguage_IT:
        case SetLanguage_JA:
        case SetLanguage_ES:
        case SetLanguage_KO:
        case SetLanguage_NL:
        case SetLanguage_PT:
        case SetLanguage_FRCA:
        case SetLanguage_ES419:
        case SetLanguage_ENUS:
        case SetLanguage_ENGB:
        default:
            return ENG::teslaStrings[id];
    }
}
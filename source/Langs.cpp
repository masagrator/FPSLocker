#include "langs/en.hpp"
#include "langs/de.hpp"
#include "langs/fr.hpp"
#include "langs/ru.hpp"
#include "langs/zhcn.hpp"
#include "langs/ptbr.hpp"
#include "langs/zhtw.hpp"
#include <switch.h>
#include "Langs.hpp"

static_assert(Lang::Id_total_number == ENG::strings.size());
static_assert(Lang::Id_total_number == GER::strings.size());
static_assert(Lang::Id_total_number == ZHCN::strings.size());
static_assert(Lang::Id_total_number == FRA::strings.size());
static_assert(Lang::Id_total_number == RUS::strings.size());
static_assert(Lang::Id_total_number == PTBR::strings.size());
static_assert(Lang::Id_total_number == ZHTW::strings.size());

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
            return ZHTW::strings[id];
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
            return ZHTW::teslaStrings[id];
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

uint8_t getNacpLanguage() {
    switch(language) {
        case SetLanguage_ENUS:  return 0;
        case SetLanguage_ENGB:  return 1;
        case SetLanguage_JA:    return 2;
        case SetLanguage_FR:    return 3;
        case SetLanguage_DE:    return 4;
        case SetLanguage_ES419: return 5;
        case SetLanguage_ES:    return 6;
        case SetLanguage_IT:    return 7;
        case SetLanguage_NL:    return 8;
        case SetLanguage_FRCA:  return 9;
        case SetLanguage_PT:    return 10;
        case SetLanguage_RU:    return 11;
        case SetLanguage_KO:    return 12;
        case SetLanguage_ZHTW:
        case SetLanguage_ZHHANT:
                                return 13;
        case SetLanguage_ZHCN:
        case SetLanguage_ZHHANS:
                                return 14;
        case SetLanguage_PTBR:  return 15;
        default: return 0;
    }
    return 0;
}
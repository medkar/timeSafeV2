#include "LocalTime.h"

namespace tsafe {

// Jours depuis 1970-01-01 pour une date civile (algorithme de Howard Hinnant).
// Pur et déterministe : aucune dépendance à la timezone de la libc.
static int64_t daysFromCivil(int y, unsigned m, unsigned d) {
    y -= (m <= 2);
    const int64_t era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = (unsigned)(y - era * 400);
    const unsigned doy = (153u * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + (int64_t)doe - 719468;
}

// Inverse : jours depuis 1970 -> date civile.
static void civilFromDays(int64_t z, int& y, int& m, int& d) {
    z += 719468;
    const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = (unsigned)(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const int yr = (int)yoe + (int)(era * 400);
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = (int)(doy - (153 * mp + 2) / 5 + 1);
    m = (int)(mp + (mp < 10 ? 3 : -9));
    y = yr + (m <= 2);
}

int parisOffsetHours(int year, int month, int day) {
    if (month < 3 || month > 10) return 1;      // nov..fév : CET
    if (month > 3 && month < 10) return 2;      // avr..sep : CEST
    // Mars ou octobre : dépend du dernier dimanche (mois à 31 jours).
    const int64_t idx31 = daysFromCivil(year, (unsigned)month, 31);
    const int dow = (int)(((idx31 + 4) % 7 + 7) % 7);   // 0 = dimanche
    const int lastSunday = 31 - dow;
    if (month == 3) return day >= lastSunday ? 2 : 1;
    return day < lastSunday ? 2 : 1;            // octobre
}

int64_t parisLocalToEpoch(int year, int month, int day, int hour, int minute) {
    const int64_t naive = daysFromCivil(year, (unsigned)month, (unsigned)day) * 86400
                        + (int64_t)hour * 3600 + (int64_t)minute * 60;
    return naive - (int64_t)parisOffsetHours(year, month, day) * 3600;
}

void epochToParisLocal(int64_t epoch, int& year, int& month, int& day,
                       int& hour, int& minute) {
    // 1) décompose en UTC pour estimer le décalage de la date visée...
    int64_t days = epoch / 86400;
    int64_t rem = epoch % 86400;
    if (rem < 0) { rem += 86400; --days; }
    int uy, um, ud;
    civilFromDays(days, uy, um, ud);
    const int off = parisOffsetHours(uy, um, ud);
    // 2) ...puis recompose l'heure locale.
    int64_t local = epoch + (int64_t)off * 3600;
    days = local / 86400;
    rem = local % 86400;
    if (rem < 0) { rem += 86400; --days; }
    civilFromDays(days, year, month, day);
    hour = (int)(rem / 3600);
    minute = (int)((rem % 3600) / 60);
}

} // namespace tsafe

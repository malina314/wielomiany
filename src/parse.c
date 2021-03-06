/** @file
  Implementacja modułu odpowiedzialnego za konwertowanie wejścia.

  @authors Mateusz Malinowski
  @date 2021
*/

#include "parse.h"
#include "line.h"
#include "vector.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * Sprawdza, czy udało się zaalokować pamięć. Jeśli nie, kończy działanie
 * programu z kodem 1.
 * @param[in] p : wskaźnik zwrócony przez funkcję alokującą pamięć
 */
#define CHECK_PTR(p)        \
    do {                    \
        if (p == NULL) {    \
            exit(1);        \
        }                   \
    } while (0)

/// błędny argument `DEG_BY`
#define DEG_BY_WRONG_VARIABLE "DEG BY WRONG VARIABLE"
/// błędny argument `AT`
#define AT_WRONG_VALUE "AT WRONG VALUE"
/// błędny argument `COMPOSE`
#define COMPOSE_WRONG_PARAMETER "COMPOSE WRONG PARAMETER"
/// niepoprawne polecenie
#define WRONG_COMMAND "WRONG COMMAND"
/// niepoprawne wielomian
#define WRONG_POLY "WRONG POLY"

/**
 * Sprawdza czy @p pre jest prefiksem @p str.
 * @param[in] pre : łańcuch znaków
 * @param[in] str : łańcuch znaków
 * @return Czy @p pre jest prefiksem @p str?
 */
static inline bool IsPrefix(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

/**
 * Sprawdza czy @p cv zawiera poprawne polecenie DEG_BY lub AT, tzn. czy @p str
 * jest prefiksem @p cv oraz czy następny znak @p cv jest znakiem białym lub
 * '\0'.
 * @param[in] cv : wektor znaków
 * @param[in] str : łańcuch znaków
 * @return Czy polecenie jest poprawne?
 */
static inline bool IsCorrectCommand(const CVector *cv, const char *str) {
    size_t len = strlen(str);
    return IsPrefix(str, cv->items) &&
        ((cv->size > len && isspace(cv->items[len])) ||
        (cv->size - 1 == len && cv->items[len] == '\0'));
}

/**
 * Sprawdza, wektor znaków @p cv jest równy z łańcuchem @p str.
 * @param[in] cv : wektor znaków zakończony '\0'
 * @param[in] str : łańcuch znaków
 * @return Czy są równe?
 */
static inline bool IsEqual(const CVector *cv, const char *str) {
    return cv->size == strlen(str) + 1 && strcmp(cv->items, str) == 0;
}

/**
 * Sprawdza czy @p c jest dozwolonym znakiem w wielomianie.
 * @param[in] c : znak
 * @return Czy @p c jest dozwolonym znakiem w wielomianie?
 */
static inline bool IsLegalCaracter(char c) {
    return isdigit(c) || c == '-' || c == '+' || c == '(' || c == ')' ||
        c == ',';
}

/**
 * Sprawdza czy wiersz zawiera niedozwolone dla wielomianu znaki.
 * @param[in] str : wiersz
 * @return Czy wiersz zawiera niedozwolone dla wielomianu znaki?
 */
static bool HasIllegalCharacters(const CVector *str) {
    for (size_t i = 0; i < str->size - 1; ++i) {
        if (!IsLegalCaracter(str->items[i])) {
            return true;
        }
    }
    return false;
}

/**
 * Sprawdza czy @p c jest cyfrą lub znakiem '-'.
 * @param[in] c : znak
 * @return Czy @p c jest cyfrą lub znakiem '-'?
 */
static inline bool IsDigitOrMinus(char c) {
    return isdigit(c) || c == '-';
}

/**
 * Sprawdza, czy wyrażenie jest porawnie nawiasowane.
 * @param s : wyrażenie
 * @return Czy wyrażenie jest porawnie nawiasowane?
 */
static bool AreParenthesesValid(const char *s) {
    int ctr = 0;

    while (*s && ctr >= 0) {
        if (*s == '(') {
            ctr++;
        }
        else if (*s == ')') {
            ctr--;
        }
        s++;
    }

    return ctr == 0;
}

/**
 * Sprawdza czy polecenie DEG_BY zawiera argument.
 * @param[in] str : wiersz
 * @return Czy polecenie DEG_BY zawiera argument?
 */
static inline bool HasDegByAnArgument(const CVector *str) {
    return str->size >= 8 && str->items[6] == ' ' && isdigit(str->items[7]);
}

/**
 * Sprawdza czy polecenie COMPOSE zawiera argument.
 * @param[in] str : wiersz
 * @return Czy polecenie COMPOSE zawiera argument?
 */
static inline bool HasComposeAnArgument(const CVector *str) {
    return str->size >= 9 && str->items[7] == ' ' && isdigit(str->items[8]);
}

/**
 * Sprawdza czy polecenie AT zawiera argument.
 * @param[in] str : wiersz
 * @return Czy polecenie AT zawiera argument?
 */
static inline bool HasAtAnArgument(const CVector *str) {
    return str->size >= 4 && str->items[2] == ' ' &&
            IsDigitOrMinus(str->items[3]);
}

/**
 * Sprawdza, czy nastąpił błąd podczas konwersji argumentu. Za błąd uznaję
 * argument spoza zakresu lub dodatkowe znaki po argumencie.
 * @param[in] str : wiersz
 * @param[in] end : wskaźnik na znak, na którym zakończyła się konwersja.
 * @return Czy nastąpił błąd podczas konwersji?
 */
static inline bool ArgumentError(const CVector *str, const char *end) {
    return errno == ERANGE || *end != '\0' ||
            (size_t)(end - str->items) != str->size - 1;
}

/**
 * Konwertuje wiersz na obiekt typu \ref Line reprezentujący wiersz zawierający
 * polecenie.
 * @param[in] str : wiersz
 * @param[in] lineNr : numer linii
 * @return skonwertowany wiersz
 */
static Line ParseCommand(const CVector *str, size_t lineNr) {
    if (IsEqual(str, "ZERO")) {
        return CommandLine(ZERO);
    }
    if (IsEqual(str, "IS_COEFF")) {
        return CommandLine(IS_COEFF);
    }
    if (IsEqual(str, "IS_ZERO")) {
        return CommandLine(IS_ZERO);
    }
    if (IsEqual(str, "CLONE")) {
        return CommandLine(CLONE);
    }
    if (IsEqual(str, "ADD")) {
        return CommandLine(ADD);
    }
    if (IsEqual(str, "MUL")) {
        return CommandLine(MUL);
    }
    if (IsEqual(str, "NEG")) {
        return CommandLine(NEG);
    }
    if (IsEqual(str, "SUB")) {
        return CommandLine(SUB);
    }
    if (IsEqual(str, "IS_EQ")) {
        return CommandLine(IS_EQ);
    }
    if (IsEqual(str, "DEG")) {
        return CommandLine(DEG);
    }
    if (IsEqual(str, "PRINT")) {
        return CommandLine(PRINT);
    }
    if (IsEqual(str, "POP")) {
        return CommandLine(POP);
    }
    if (IsCorrectCommand(str, "DEG_BY")) {
        if (HasDegByAnArgument(str)) {
            char *end;
            errno = 0;
            size_t arg = strtoull(str->items + 7, &end, 10);

            if (ArgumentError(str, end)) {
                PrintErrorMsg(lineNr, DEG_BY_WRONG_VARIABLE);
                return WrongLine();
            }

            return CommandLineWithArg(DEG_BY, arg);
        }
        else {
            PrintErrorMsg(lineNr, DEG_BY_WRONG_VARIABLE);
            return WrongLine();
        }
    }
    if (IsCorrectCommand(str, "AT")) {
        if (HasAtAnArgument(str)) {
            char *end;
            errno = 0;
            poly_coeff_t arg = strtoll(str->items + 3, &end, 10);

            if (ArgumentError(str, end)) {
                PrintErrorMsg(lineNr, AT_WRONG_VALUE);
                return WrongLine();
            }

            return CommandLineWithArg(AT, arg);
        }
        else {
            PrintErrorMsg(lineNr, AT_WRONG_VALUE);
            return WrongLine();
        }
    }
    if (IsCorrectCommand(str, "COMPOSE")) {
        if (HasComposeAnArgument(str)) {
            char *end;
            errno = 0;
            size_t arg = strtoull(str->items + 8, &end, 10);

            if (ArgumentError(str, end)) {
                PrintErrorMsg(lineNr, COMPOSE_WRONG_PARAMETER);
                return WrongLine();
            }

            return CommandLineWithArg(COMPOSE, arg);
        }
        else {
            PrintErrorMsg(lineNr, COMPOSE_WRONG_PARAMETER);
            return WrongLine();
        }
    }

    PrintErrorMsg(lineNr, WRONG_COMMAND);
    return WrongLine();
}

/**
 * Konwertuje wykładnik jednomianu. Funkcja ustawia @p end na pierwszy znak po
 * skonwertowanym fragmencie (dla poprawnego jednomianu jest to ')').
 * Funkcja ustawia @p err na true jeżeli @p begin nie wskazuje na cyfrę lub gdy
 * wykładnik wykracza poza zakres lub gdy po wykładniku nie stoi znak ')'.
 * @param[in] begin : początek wykładnika
 * @param[out] end : wskaźnik na pierwszy znak po skonwertowanym fragmencie
 * @param[out] err : flaga błędu
 * @return wykładnik
 */
static int ParseExp(char *begin, char **end, bool *err) {
    if (!isdigit(*begin)) {
        *err = true;
        return 0;
    }

    errno = 0;
    long x = strtol(begin, end, 10);

    if (errno == ERANGE || **end != ')' || x > (long)INT_MAX) {
        *err = true;
        return 0;
    }

    return x;
}

static Poly ParsePolyHelper(char *begin, char **end, bool *err);

/**
 * Konwertuje jednomian. Funkcja ustawia @p end na pierwszy znak po
 * skonwertowanym fragmencie (dla poprawnego jednomianu jest to ')').
 * Funkcja ustawia @p err na true jeżeli @p begin nie wskazuje na '\0' lub gdy
 * jednomian jest niepoprawny.
 * @param[in] begin : początek jednomianu (pierwszy znak po '(')
 * @param[out] end : wskaźnik na pierwszy znak po skonwertowanym fragmencie
 * @param[out] err : flaga błędu
 * @return jednomian
 */
static Mono ParseMono(char *begin, char **end, bool *err) {
    if (*begin == '\0') {
        *err = true;
        return (Mono) {};
    }

    Poly p;

    p = ParsePolyHelper(begin, end, err);
    if (*err) {
        return (Mono) {};
    }

    if (**end == '\0') {
        *err = true;
        PolyDestroy(&p);
        return (Mono) {};
    }
    // teraz *end wskazuje na ','

    int exp = ParseExp((*end) + 1, end, err);
    if (*err) {
        PolyDestroy(&p);
        return (Mono) {};
    }

    return MonoFromPoly(&p, exp);
}

/**
 * Konwertuje wielomian. Poprawny wielomian zaczyna się znakiem '(', cyfrą lub
 * znakiem '-'.
 * Funkcja ustawia @p end na pierwszy znak po
 * skonwertowanym fragmencie (dla poprawnego wielomianu jest to '\0' lub ',').
 * Funkcja ustawia @p err na true jeżeli @p begin nie wskazuje na '\0' lub gdy
 * wielomian jest niepoprawny.
 * @param[in] begin : początek wielomianu
 * @param[out] end : wskaźnik na pierwszy znak po skonwertowanym fragmencie
 * @param[out] err : flaga błędu
 * @return wielomian
 */
static Poly ParsePolyHelper(char *begin, char **end, bool *err) {
    if (*begin == '\0') {
        *err = true;
        return (Poly) {};
    }

    if (IsDigitOrMinus(*begin)) { // parsowany wielomian jest współczynnikiem
        errno = 0;
        poly_coeff_t x = strtoll(begin, end, 10);
        if (errno == ERANGE || (**end != ',' && **end != '\0')) {
            *err = true;
        }
        return PolyFromCoeff(x);
    }
    else { // parsowany wielomian jest jednomianem lub sumą jednomianów
        MVector monos = MVectorNew();

        while (true) {
            if (*begin != '(') {
                *err = true;
                MVectorDeepFree(&monos);
                return (Poly) {};
            }

            Mono m = ParseMono(begin + 1, end, err);
            if (*err) {
                MVectorDeepFree(&monos);
                return (Poly) {};
            }
            MVectorPush(&monos, m);

            // po wyjściu z ParseMono *end wskazuje na ')' kończący jednomian
            (*end)++;

            if (**end == '\0' || **end == ',') { // poprawny koniec wielomianu
                break;
            }

            if (**end != '+') { // niepoprawna suma jednomianów
                *err = true;
                MVectorDeepFree(&monos);
                return (Poly) {};
            }

            begin = (*end) + 1;
        }

        Poly p = PolyAddMonos(monos.size, monos.items);

        MVectorFree(&monos);

        return p;
    }
}

/**
 * Konwertuje wiersz na obiekt typu \ref Line reprezentujący wiersz zawierający
 * wielomian.
 * @param[in] str : wiersz
 * @param[in] lineNr : numer linii
 * @return skonwertowany wiersz
 */
static Line ParsePoly(const CVector *str, size_t lineNr) {
    if (HasIllegalCharacters(str) || !AreParenthesesValid(str->items)) {
        PrintErrorMsg(lineNr, WRONG_POLY);
        return WrongLine();
    }

    bool err = false;
    char *end;

    Poly p = ParsePolyHelper(str->items, &end, &err);

    if (err || *end != '\0') {
        PrintErrorMsg(lineNr, WRONG_POLY);
        PolyDestroy(&p);
        return WrongLine();
    }

    return PolyLine(p);
}

Line Parse(const CVector *str, size_t lineNr) {
    if (isalpha(str->items[0])) {
        return ParseCommand(str, lineNr);
    }
    else {
        return ParsePoly(str, lineNr);
    }
}
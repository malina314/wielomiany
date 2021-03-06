/** @file
  Plik udostępnia stukturę stosu wielomianów oraz funkcje umożliwiające
  podstawowe operacje na tej strukturze.

  @authors Mateusz Malinowski
  @date 2021
*/

#ifndef POLYNOMIALS_STACK_H
#define POLYNOMIALS_STACK_H

#include "poly.h"
#include <stddef.h>
#include <stdbool.h>

/**
 * To jest struktura stosu wielomianów.
 */
typedef struct {
    Poly *items; ///< tablica przechowująca wielomiany
    size_t size; ///< liczba elementów na stosie
    size_t allocated; ///< rozmiar tablicy items
} Stack;

/**
 * Tworzy pusty stos i go zwraca.
 * @return stos
 */
Stack StackNew();

/**
 * Zwalnia pamięć używaną przez stos.
 * @param[in,out] self : stos
 */
void StackFree(Stack *self);

/**
 * Dodaje wielomian na wierzchołek stosu.
 * @param[in,out] self : stos
 * @param[in] p : wielomian
 */
void StackPush(Stack *self, Poly p);

/**
 * Usuwa wielomian z wierzchołka stosu.
 * @param[in,out] self : stos
 */
void StackPop(Stack *self);

/**
 * Usuwa wielomian z wierzchołka stosu i go niszczy.
 * @param[in,out] self : stos
 */
void StackHardPop(Stack *self);

/**
 * Zwraca wielomian z wierzchołka stosu.
 * @param[in,out] self : stos
 * @return wielomian z wierzchołka stosu
 */
Poly StackTop(const Stack *self);

/**
 * Sprawdza czy stos jest pusty.
 * @param[in,out] self : stos
 * @return Czy stos jest pusty?
 */
bool StackEmpty(const Stack *self);

/**
 * Zwraca rozmiar stosu.
 * @param[in] self : stos
 * @return rozmiar stosu
 */
size_t StackSize(const Stack *self);

#endif //POLYNOMIALS_STACK_H

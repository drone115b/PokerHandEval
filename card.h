#ifndef CARDHEADER
#define CARDHEADER

#include <cstdint>
#include <cassert>
#include <string>


// algorithms for cardset rely heavily on the idea
// that bitwise operations work
//
// each card in the deck is represented by a bit
// in the lower 52 bits.
//
// suit order (minor) is: cdhs
// rank order (major) is: A234...K
//
// so 1    == Ac
//    1<<7 == 2s
//
typedef uint64_t CardSet;


// return the number of cards in the cardset
// reference: https://en.wikipedia.org/wiki/Hamming_weight
unsigned
count_cards( CardSet x );


// Returns the log base 2 of the given integer
// This is essentially the position of the highest bit.
//
// assert: there exists at least one set bit in v
//
// reference: https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
uint8_t
high_card_pos( CardSet v );


// Given a cardset that indicates cards already drawn,
// returns a word indicating a card that has not been drawn yet.
//
// You will need to remove this from the card set to complete
// a draw with
//     seen |= draw;
//
// pass a random integer into the function
//
uint64_t draw_a_card(uint64_t seen, uint64_t rand_word);

//
// convert between rank/suit and Cardset representation
// of a card.
//
// rank 0-12
// suit 0-3
//
CardSet from_rank_suit( uint8_t rank, uint8_t suit );


//
// if there is more than one card in the cardset,
// the greatest card is evaluated:
//
void to_rank_suit( uint8_t& rank, uint8_t& suit, CardSet c );

void print_hand(std::string& desc, const CardSet c);

extern const char * const LongRankName[];
extern const char ShortRankName[];
extern const char * const LongSuitName[];
extern const char ShortSuitName[];

#endif

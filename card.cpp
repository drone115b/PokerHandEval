#include "card.h"

// return the number of cards in the cardset
// reference: https://en.wikipedia.org/wiki/Hamming_weight
unsigned
count_cards( CardSet x )
{
  const uint64_t m1  = 0x5555555555555555; //binary: 0101...
  const uint64_t m2  = 0x3333333333333333; //binary: 00110011..
  const uint64_t m4  = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...

  x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
  x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits
  x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits
  x += x >>  8;  //put count of each 16 bits into their lowest 8 bits
  x += x >> 16;  //put count of each 32 bits into their lowest 8 bits
  x += x >> 32;  //put count of each 64 bits into their lowest 8 bits
  return x & 0x7f;
}

// Returns the log base 2 of the given integer
// This is essentially the position of the highest bit.
//
// assert: there exists at least one set bit in v
//
// reference: https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
uint8_t
high_card_pos( CardSet v )
{
  uint8_t r = 0; // result of log2(v) will go here
  uint8_t s = 32;
  while( s ) {
    if( v >= (1ULL << s)){
        v >>= s;
        r |= s;
    }
    s >>= 1;
  }
  return r;
}

// Given a cardset that indicates cards already drawn,
// returns a word indicating a card that has not been drawn yet.
//
// You will need to remove this from the card set to complete
// a draw with
//     seen |= draw;
//
// pass a random integer into the function
//
uint64_t draw_a_card(uint64_t seen, uint64_t rand_word)
{
    uint64_t w = (~seen) & ((1ULL << 52)-1);
    uint8_t c(0);
    uint8_t lenw = count_cards(w);
    uint8_t pick(rand_word % lenw); // @@ biased
    uint8_t m = 32;
    uint64_t mw = (1ULL << 32) - 1;

    assert(w);

    while( w && m ){
       uint64_t a0 = w & mw;
       uint64_t a1 = (w >> m) & mw;

       if( a0 && a1 ){
         uint8_t lena0 = count_cards(a0);
         if( pick < lena0 ){
           w = a0;
           lenw = lena0;
         } else {
           w = a1;
           lenw -= lena0;
           c += m;
           pick -= lena0;
         }
       } else if ( a1 ) {
         w = a1;
         c += m;
       }
       m >>= 1;
       mw >>= m;

    } // end while

    return 1ULL << c;
}

//
// convert between rank/suit and Cardset representation
// of a card.
//
// rank 0-12
// suit 0-3
//
CardSet from_rank_suit( uint8_t rank, uint8_t suit )
{
  return  (1ULL << suit) << (rank << 2);
}

//
// if there is more than one card in the cardset,
// the greatest card is evaluated:
//
void to_rank_suit( uint8_t& rank, uint8_t& suit, CardSet c )
{
  uint8_t l = high_card_pos( c );
  suit = l & 0x3;
  rank = l >> 2;
  assert( from_rank_suit( rank, suit ) == (1ULL << l) );
}

const char * const LongRankName[] =
{
"Ace",
"Deuce",
"Three",
"Four",
"Five",
"Six",
"Seven",
"Eight",
"Nine",
"Ten",
"Jack",
"Queen",
"King",
"Ace"
};


const char ShortRankName[] =
{
'A',
'2',
'3',
'4',
'5',
'6',
'7',
'8',
'9',
'T',
'J',
'Q',
'K',
'A'
};

const char * const LongSuitName[] =
{
"Club",
"Diamond",
"Heart",
"Spade"
};

const char ShortSuitName[] =
{
'c',
'd',
'h',
's'
};

void print_hand(std::string& desc, const CardSet c){
  CardSet tmp(c);
  desc.clear();

  if( tmp ){
    uint8_t rank, suit;
    while( tmp ){
      to_rank_suit(rank, suit, tmp);
      desc.push_back(ShortRankName[rank]);
      desc.push_back(ShortSuitName[suit]);
      desc.push_back(' ');

      tmp ^= from_rank_suit( rank, suit ); // toggle MSb off
    } // end while
  } // end if

  return;
}

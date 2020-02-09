#include "evaluator.h"

#include <cassert>
#include <cstdio> // sprintf()
#include <cstring>

// (1 << 20)  high card
#define VALUE_HIGH_CARD       (1ULL << 20)
// (2 << 20)  one pair
#define VALUE_ONE_PAIR        (2ULL << 20)
// (3 << 20)  two pair
#define VALUE_TWO_PAIR        (3ULL << 20)
// (4 << 20)  three of a kind
#define VALUE_THREE_OF_KIND   (4ULL << 20)
// (5 << 20)  straight
#define VALUE_STRAIGHT        (5ULL << 20)
// (6 << 20)  flush
#define VALUE_FLUSH           (6ULL << 20)
// (7 << 20)  full house
#define VALUE_BOAT            (7ULL << 20)
// (8 << 20)  quad
#define VALUE_QUAD            (8ULL << 20)
// (9 << 20)  straight flush
#define VALUE_STRAIGHT_FLUSH  (9ULL << 20)


bool is_a_straight( HandValue& h, char *cpDesc, const CardSet& c )
{
    const uint64_t rankfilter( 0x0F );

    uint64_t word(c);
    word |= (word & 0x0F) << 52; // wrap Ace

    bool ret = false;
    h = 0; // default

    uint8_t rank = 13;
    while( rank >= 4 )
    {
        // if five consecutive ranks:
        if(
          ( word & (rankfilter << ( rank   <<2))) &&
          ( word & (rankfilter << ((rank-1)<<2))) &&
          ( word & (rankfilter << ((rank-2)<<2))) &&
          ( word & (rankfilter << ((rank-3)<<2))) &&
          ( word & (rankfilter << ((rank-4)<<2))) )
        {
          // straight is found:
          ret = true;
          h = VALUE_STRAIGHT;

          // set kicker:
          uint8_t kickerrank, kickersuit;
          to_rank_suit( kickerrank, kickersuit, ( word & (rankfilter << ( rank   <<2))) );
          if( cpDesc )
          {
              sprintf( cpDesc, "%s high straight", LongRankName[kickerrank] );
          }
          assert( kickerrank < 16 );
          h |= kickerrank << 16;

          break; // while
        }
        --rank;
    }

    return ret;
}


bool is_a_flush( HandValue& h, char *cpDesc, const CardSet& c )
{
  const uint64_t suitfilter( 0x011111111111111 );

  uint64_t word(c);
  word |= (word & 0x0F) << 52; // wrap Ace to high
  word ^= (word & 0x0F); // turn off low Ace(s)

  bool ret = false;
  h = 0; // default

  int8_t suit = 3;
  while( suit >= 0 )
  {
    if( count_cards( word & (suitfilter << suit) ) >= 5 )
    {
      word &= (suitfilter << suit); // filter down to this restricted card-set

      // flush is found:
      ret = true;
      h = VALUE_FLUSH;

      // fill in the kickers:
      uint8_t kickerrank, kickersuit;
      to_rank_suit( kickerrank, kickersuit, word );
      h |= kickerrank << 16;
      word ^= from_rank_suit( kickerrank, kickersuit ); // turn bit off
      if( cpDesc )
        sprintf( cpDesc, "%s high flush", LongRankName[kickerrank] );

      to_rank_suit( kickerrank, kickersuit, word );
      h |= kickerrank << 12;
      word ^= from_rank_suit( kickerrank, kickersuit ); // turn bit off

      to_rank_suit( kickerrank, kickersuit, word );
      h |= kickerrank <<  8;
      word ^= from_rank_suit( kickerrank, kickersuit ); // turn bit off

      to_rank_suit( kickerrank, kickersuit, word );
      h |= kickerrank <<  4;
      word ^= from_rank_suit( kickerrank, kickersuit ); // turn bit off

      to_rank_suit( kickerrank, kickersuit, word );
      h |= kickerrank      ;

      break; // while
    }
    --suit;
  }

  return ret;
}


void _fill_kickers(
  HandValue& h,
  const uint8_t* const kickers,
  uint8_t kickers_count,
  uint8_t how_many_kickers,
  uint8_t where,
  uint8_t skip1,
  uint8_t skip2
  ) {

  uint8_t i=0;
  while( (i<kickers_count) && how_many_kickers ){
    if(( kickers[i] != skip1)&&( kickers[i] != skip2 ))
    {
      h |= uint32_t(kickers[i]) << where;
      where -= 4;
      --how_many_kickers;
    }
    ++i;
  }

  return;
}

bool is_a_pairing( HandValue& h, char *cpDesc, const CardSet& c )
{
  const uint8_t rank_count[]={ 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
  uint8_t kickers[14];
  uint8_t kickers_count = 0;

  uint8_t count;
  bool ret = false;

  int8_t quad_rank = -1;
  int8_t trip_rank = -1;
  int8_t mxpair_rank = -1;
  int8_t mnpair_rank = -1;

  uint64_t word(c);
  word |= (word & 0x0F) << 52; // wrap Ace to high
  word ^= (word & 0x0F); // turn off low Ace(s)

  uint8_t rank=14;
  while( --rank )
  {
    count = rank_count[ (word >> (rank<<2)) & 0x0F ];

    switch( count ) {
      case( 4 ):
        if ( -1 == quad_rank ){
          quad_rank = rank;
          break;
        }
      case( 3 ):
        if ( -1 == trip_rank ){
          trip_rank = rank;
          break;
        }
      case( 2 ):
        if ( -1 == mxpair_rank ){
          mxpair_rank = rank;
          break;
        } else if ( -1 == mnpair_rank ){
          mnpair_rank = rank;
          break;
        }
      default:
        break;
    } // switch

    if( count )
    {
      kickers[ kickers_count++ ] = rank;
    }
  } // while

  // done finding matches, now finalize the score:
  if( -1 != quad_rank ){
    h = VALUE_QUAD;
    h |= uint32_t( quad_rank ) << 16;
    _fill_kickers( h, kickers, kickers_count, 1, 12, quad_rank, quad_rank );
    if( cpDesc )
      sprintf( cpDesc, "Quad %ss", LongRankName[quad_rank] );
    ret = true;
  } else if (( -1 != trip_rank ) && ( -1 != mxpair_rank )){
    h = VALUE_BOAT;
    h |= uint32_t(trip_rank) << 16;
    h |= uint32_t(mxpair_rank) << 12;
    if( cpDesc )
      sprintf( cpDesc, "%ss full of %ss", LongRankName[trip_rank], LongRankName[mxpair_rank]);
    ret = true;
  } else if ( -1 != trip_rank ) {
    h = VALUE_THREE_OF_KIND;
    h |= uint32_t(trip_rank) << 16;
    _fill_kickers( h, kickers, kickers_count, 2, 12, trip_rank, trip_rank );
    if( cpDesc )
      sprintf( cpDesc, "Three of a kind, %ss", LongRankName[trip_rank]);
    ret = true;
  } else if( (-1 != mxpair_rank ) && (-1 != mnpair_rank )) {
    h = VALUE_TWO_PAIR;
    h |= uint32_t(mxpair_rank) << 16;
    h |= uint32_t(mnpair_rank) << 12;
    _fill_kickers( h, kickers, kickers_count, 1, 8, trip_rank, trip_rank );
    if( cpDesc )
      sprintf( cpDesc, "%ss over %ss", LongRankName[mxpair_rank], LongRankName[mnpair_rank]);
    ret = true;
  } else if( -1 != mxpair_rank ){
    h = VALUE_ONE_PAIR;
    h |= uint32_t(mxpair_rank) << 16;
    _fill_kickers( h, kickers, kickers_count, 3, 12, mxpair_rank, mxpair_rank );
    if( cpDesc )
      sprintf( cpDesc, "Pair of %ss", LongRankName[mxpair_rank]);
    ret = true;
  } else {
    h = VALUE_HIGH_CARD;
    _fill_kickers( h, kickers, kickers_count, 5, 16, 255, 255 );
    if( cpDesc )
      sprintf( cpDesc, "%s high", LongRankName[kickers[0]]);
    ret = true;
  }

  return ret;
}





bool is_a_straight_flush( HandValue& h, char *cpDesc, const CardSet& c )
{
  const uint64_t suitfilter( 0x011111111111111 );

  // defaults:
  h = 0;
  bool ret = false;

  int8_t suit=4;
  while( --suit )
  {
    char tmpcp[64];
    HandValue tmph;
    bool tmpb = is_a_straight( tmph, cpDesc ? tmpcp : 0, c & (suitfilter << suit) );

    if( tmpb && ( tmph > h ))
    {
      if( cpDesc )
        sprintf( cpDesc, "Straight flush");
      ret = true;
      h = tmph;
    }
  }

  if( ret )
  {
    // convert from a straight value to a straight flush value:
    h &= (1ULL << 20)-1ULL; // keep kickers
    h |= VALUE_STRAIGHT_FLUSH; // replace value
  }

  return ret;
}

// ==============================================

HandValue
StdHandEvaluator::value( CardSet hand, char *cpDesc ){
  HandValue ret(0);

  HandValue hStraight(0);
  char cpStraight[32];
  bool bStraight = is_a_straight(hStraight, cpStraight, hand);
  if(hStraight > ret){
    ret = hStraight;
    if( cpDesc )
      strcpy(cpDesc, cpStraight);
  }

  HandValue hFlush(0);
  char cpFlush[32];
  bool bFlush = is_a_flush(hFlush, cpFlush, hand);
  if(hFlush > ret){
    ret = hFlush;
    if( cpDesc )
      strcpy(cpDesc, cpFlush);
  }

  HandValue hPairing(0);
  char cpPairing[32];
  bool bPairing = is_a_pairing(hPairing, cpPairing, hand);
  if(hPairing > ret){
    ret = hPairing;
    if( cpDesc )
      strcpy(cpDesc, cpPairing);
  }

  HandValue hStraightFlush(0);
  char cpStraightFlush[32];
  bool bStraightFlush = false;
  if( bFlush && bStraight ){
    bStraightFlush = is_a_straight_flush(hStraightFlush, cpStraightFlush, hand);
  }
  if(hStraightFlush > ret){
    ret = hStraightFlush;
    if( cpDesc )
      strcpy(cpDesc, cpStraightFlush);
  }

  return ret;
}

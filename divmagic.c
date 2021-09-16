/**
 * Computes the magic numbers for 32/64bits signed and unsigned integer division.
 *
 * References:
 *   http://www.hackersdelight.org/magic.htm (may need to use the Wayback Machine)
 *   http://www.flounder.com/multiplicative_inverse.htm
 *   https://stackoverflow.com/questions/53414711/math-behind-gcc9-modulus-optimizations (re: use of quotient)
 **/

#include <inttypes.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define S(s) _S(s)
#define _S(s) #s

#if BITS != 32 && BITS != 64
# error define BITS as 32 or 64
#endif

#if BITS == 32
typedef int32_t sint_t;
typedef uint32_t uint_t;
# define MAX_UINT UINT32_MAX
# define MAX_INT INT32_MAX
# define MAX_POW2 (1u << (BITS-1))
# define str2sint strtol
# define PRId PRId32
# define PRIx PRIx32
# define LENx "10"
#else // 64
typedef int64_t sint_t;
typedef uint64_t uint_t;
# define MAX_UINT UINT64_MAX
# define MAX_INT INT64_MAX
# define MAX_POW2 (1ull << (BITS-1))
# define str2sint strtoll
# define PRId PRId64
# define PRIx PRIx64
# define LENx "18"
# define abs labs
#endif

struct ms {
    sint_t M;   // Magic number
    sint_t s;   // and shift amount.
};
struct mu {
    uint_t M;   // Magic number,
    sint_t s;   // shift amount,
    sint_t a;   // and "add" indicator.
};

struct ms divmagic(sint_t);
struct mu divmagicu(uint_t);
uint_t mulinv(sint_t);

int main(int argc, char* argv[])
{
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "usage: %s <dividend> [dividend]\n", basename(argv[0]));
        return EXIT_FAILURE;
    }

    char* endptr = NULL;
    sint_t d1 = str2sint(argv[1], &endptr, 0);
    if (argv[1][0] == '\0' || *endptr != '\0') {
        fprintf(stderr, "error: dividends must be signed " S(BITS) "bits integers\n");
        return EXIT_FAILURE;
    }
    sint_t d2 = d1;
    if (argc == 3) {
        d2 = str2sint(argv[2], &endptr, 0);
        if (argv[2][0] == '\0' || *endptr != '\0') {
            fprintf(stderr, "error: dividends must be signed " S(BITS) "bits integers\n");
            return EXIT_FAILURE;
        }
    }
    if (d1 > d2) {
        fprintf(stderr, "error: lower bound of dividend range is greater than upper bound\n");
        return EXIT_FAILURE;
    }

    for (sint_t i = d1; i <= d2; i++) {
        if (i == 0)
            continue;
        struct ms mags = divmagic(i);
        struct mu magu = divmagicu(i);
        uint_t q = MAX_UINT / (uint_t)i;
        printf("d=%" PRId " signed(M=%#0" LENx PRIx " s=%" PRId ") unsigned(M=%#0" LENx PRIx " s=%" PRId " a=%" PRId ") quotient(q=%#0" LENx PRIx ")", i, mags.M, mags.s, magu.M, magu.s, magu.a, q);
        if (i & 1) {    // compute multiplicative inverse of odd numbers
            printf(" inverse(M=%#0" LENx PRIx ")", mulinv(i));
        }
        printf("\n");
    }

    return EXIT_SUCCESS;
}

struct ms divmagic(sint_t d)   // Must have 2 <= d <= 2**BITS-1
{                           // or   -2**BITS <= d <= -2.
    sint_t p;
    uint_t ad, anc, delta, q1, r1, q2, r2, t;
    struct ms mag;

    ad = abs(d);
    t = MAX_POW2 + ((uint_t)d >> (BITS-1));
    anc = t - 1 - t%ad;     // Absolute value of nc.
    p = BITS-1;             // Init. p.
    q1 = MAX_POW2/anc;      // Init. q1 = 2**p/|nc|.
    r1 = MAX_POW2 - q1*anc; // Init. r1 = rem(2**p, |nc|).
    q2 = MAX_POW2/ad;       // Init. q2 = 2**p/|d|.
    r2 = MAX_POW2 - q2*ad;  // Init. r2 = rem(2**p, |d|).
    do {
        p = p + 1;
        q1 = 2*q1;          // Update q1 = 2**p/|nc|.
        r1 = 2*r1;          // Update r1 = rem(2**p, |nc|).
        if (r1 >= anc) {    // (Must be an uint_t comparison here.)
            q1 = q1 + 1;
            r1 = r1 - anc;}
        q2 = 2*q2;          // Update q2 = 2**p/|d|.
        r2 = 2*r2;          // Update r2 = rem(2**p, |d|).
        if (r2 >= ad) {     // (Must be an uint_t comparison here).
            q2 = q2 + 1;
            r2 = r2 - ad;}
        delta = ad - r2;
    } while (q1 < delta || (q1 == delta && r1 == 0));

    mag.M = q2 + 1;
    if (d < 0) mag.M = -mag.M;  // Magic number and
    mag.s = p - BITS;           // shift amount to return.
    return mag;
}

struct mu divmagicu(uint_t d)    // Must have 1 <= d <= 2**BITS-1.
{
    sint_t p;
    uint_t nc, delta, q1, r1, q2, r2;
    struct mu magu;

    magu.a = 0;                 // Initialize "add" indicator.
    nc = -1 - (-d)%d;           // uint_t arithmetic here.
    p = BITS-1;                 // Init. p.
    q1 = MAX_POW2/nc;           // Init. q1 = 2**p/nc.
    r1 = MAX_POW2 - q1*nc;      // Init. r1 = rem(2**p, nc).
    q2 = MAX_INT/d;             // Init. q2 = (2**p - 1)/d.
    r2 = MAX_INT - q2*d;        // Init. r2 = rem(2**p - 1, d).
    do {
        p = p + 1;
        if (r1 >= nc - r1) {
            q1 = 2*q1 + 1;            // Update q1.
            r1 = 2*r1 - nc;}          // Update r1.
        else {
            q1 = 2*q1;
            r1 = 2*r1;}
        if (r2 + 1 >= d - r2) {
            if (q2 >= (MAX_INT-1)) magu.a = 1;
            q2 = 2*q2 + 1;            // Update q2.
            r2 = 2*r2 + 1 - d;}       // Update r2.
        else {
            if (q2 >= MAX_POW2) magu.a = 1;
            q2 = 2*q2;
            r2 = 2*r2 + 1;}
        delta = d - 1 - r2;
    } while (p < (2*BITS) &&
            (q1 < delta || (q1 == delta && r1 == 0)));

    magu.M = q2 + 1;        // Magic number
    magu.s = p - BITS;      // and shift amount to return
    return magu;            // (magu.a was set above).
}

uint_t mulinv(sint_t d)
{
    sint_t t;
    sint_t xn = d;
    while ((t = d*xn) != 1)
        xn = xn * (2-t);
    return xn;
}

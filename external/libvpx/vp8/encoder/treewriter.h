


#ifndef __INC_TREEWRITER_H
#define __INC_TREEWRITER_H


#include "treecoder.h"

#include "boolhuff.h"       /* for now */

typedef BOOL_CODER vp8_writer;

#define vp8_write vp8_encode_bool
#define vp8_write_literal vp8_encode_value
#define vp8_write_bit( W, V) vp8_write( W, V, vp8_prob_half)

#define vp8bc_write vp8bc_write_bool
#define vp8bc_write_literal vp8bc_write_bits
#define vp8bc_write_bit( W, V) vp8bc_write_bits( W, V, 1)


/* Approximate length of an encoded bool in 256ths of a bit at given prob */

#define vp8_cost_zero( x) ( vp8_prob_cost[x])
#define vp8_cost_one( x)  vp8_cost_zero( vp8_complement(x))

#define vp8_cost_bit( x, b) vp8_cost_zero( (b)?  vp8_complement(x) : (x) )

/* VP8BC version is scaled by 2^20 rather than 2^8; see bool_coder.h */


/* Both of these return bits, not scaled bits. */

static __inline unsigned int vp8_cost_branch(const unsigned int ct[2], vp8_prob p)
{
    /* Imitate existing calculation */

    return ((ct[0] * vp8_cost_zero(p))
            + (ct[1] * vp8_cost_one(p))) >> 8;
}


static __inline void vp8_treed_write
(
    vp8_writer *const w,
    vp8_tree t,
    const vp8_prob *const p,
    int v,
    int n               /* number of bits in v, assumed nonzero */
)
{
    vp8_tree_index i = 0;

    do
    {
        const int b = (v >> --n) & 1;
        vp8_write(w, b, p[i>>1]);
        i = t[i+b];
    }
    while (n);
}
static __inline void vp8_write_token
(
    vp8_writer *const w,
    vp8_tree t,
    const vp8_prob *const p,
    vp8_token *const x
)
{
    vp8_treed_write(w, t, p, x->value, x->Len);
}

static __inline int vp8_treed_cost(
    vp8_tree t,
    const vp8_prob *const p,
    int v,
    int n               /* number of bits in v, assumed nonzero */
)
{
    int c = 0;
    vp8_tree_index i = 0;

    do
    {
        const int b = (v >> --n) & 1;
        c += vp8_cost_bit(p[i>>1], b);
        i = t[i+b];
    }
    while (n);

    return c;
}
static __inline int vp8_cost_token
(
    vp8_tree t,
    const vp8_prob *const p,
    vp8_token *const x
)
{
    return vp8_treed_cost(t, p, x->value, x->Len);
}

/* Fill array of costs for all possible token values. */

void vp8_cost_tokens(
    int *Costs, const vp8_prob *, vp8_tree
);

#endif

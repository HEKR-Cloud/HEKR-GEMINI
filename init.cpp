/* lmss : Init */

#include "lmss.h"
#include "ref.h"
#include "err.h"
#include "vm.h"

class ErrPool;
class BigNumPool;
class BufferPool;
class RefPool;
class PairPool;
class PriProcPool;
class LambdaProcPool;
class InterruptVector;

extern ErrPool *err_pool;
extern BigNumPool *bignum_pool;
extern BufferPool *buffer_pool;
extern RefPool *ref_pool;
extern PairPool *pair_pool;
extern PriProcPool *priproc_pool;
extern LambdaProcPool *lambdaproc_pool;
extern InterruptVector *interrupt_vector;

void lmss_init(void)
{
    err_pool = new ErrPool();
    bignum_pool = new BigNumPool();
    buffer_pool = new BufferPool();
    ref_pool = new RefPool();
    pair_pool = new PairPool();
    priproc_pool = new PriProcPool();
    lambdaproc_pool = new LambdaProcPool();
    interrupt_vector = new InterruptVector();
    vm = new VM();
}


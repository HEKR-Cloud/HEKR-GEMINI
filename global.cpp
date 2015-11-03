/* lmss : Global Definitions */

class ErrPool;
class BigNumPool;
class RefPool;
class RefItem;
class PairPool;
class PairItem;
class PriProcItem;
class PriProcPool;
class LambdaProcItem;
class LambdaProcPool;
class InterruptVector;
class VM;

static ErrPool *err_pool = NULL;
static BigNumPool *bignum_pool = NULL;
static RefPool *ref_pool = NULL;
static PairPool *pair_pool = NULL;
static PriProcPool *priproc_pool = NULL;
static LambdaProcPool *lambdaproc_pool = NULL;
static InterruptVector *interrupt_vector = NULL;
static VM *vm = NULL;


#ifdef __OPENCL_EDIT__
#define kernel
#define global
#endif
/**
 * @brief   OpenCL kernel used to calculate equation resolver for
 *          reverse barrel distortion correction.
 *
 * @file    from_barrel.c
 * @author  OSi (Ondrej Sienczak)
 */


/**
 * @brief   Indexes of arguments
 */
enum ArgIdx
{
    ITEMS_CNT,      /**< @brief Count of items to be calculated */
    K0,             /**< @brief First polynomial constant */
    K1,             /**< @brief Second polynomial constant */
    K2,             /**< @brief Third polynomial constant */
    CENTER_X,       /**< @brief X coordinate of center of barrel distortion */
    CENTER_Y,       /**< @brief Y coordinate of center of barrel distortion */
    __ARGS_CNT      /**< @brief Count of arguments */
};


/**
 * @brief   Argument type
 */
union Arg
{
    float f;        /**< @brief Argument of float type */
    int   i;        /**< @brief Argument of integer type */
};


/**
 * @brief   Coordinates structure
 */
struct Vector
{
    int x;  /**< @brief X coordinate */
    int y;  /**< @brief Y coordinate */
};


/**
 * @brief   Accumulated root of polynom
 *
 * @param   x   X coordinate of equation
 * @param   y   Y coordinate of equation
 * @param   k0  First polynomial constant
 * @param   k1  Second polynomial constant
 * @param   k2  Third polynomial constant
 *
 * @return  Calculated root
 */
float _acc(float x,
           float y,
           float k0,
           float k1,
           float k2)
{
    float         rq = x * x + y * y;
    return 1.0F + rq * (k0 + rq * (k1 + rq * k2));
}


/**
 * @brief   Barrel distortion reverse equation disolver
 *
 * @param   inX     Input X coordinates
 * @param   inY     Input Y coordinates
 * @param   outX    Output X coordinates
 * @param   outY    Output Y coordinates
 * @param   args    Arguments of filter
 */
void kernel fromBarrel(global struct Vector*       out,
                       global const struct Vector* in,
                       global const union Arg*     args)
{
    const int   id    = get_global_id(0);    /* Current thread ID */
    const int   cnt   = get_global_size(0);  /* Count of threads */
    
    const int   items = args[ITEMS_CNT].i;   /* Count of items needs to be calculated */
    const float k0    = args[K0].f;          /* Constants used for calculation */
    const float k1    = args[K1].f;
    const float k2    = args[K2].f;
    const int   cX    = args[CENTER_X].i;
    const int   cY    = args[CENTER_Y].i;
    
    const int   ratio = items / cnt;         /* Number of elements for each thread */
    const int   begin = ratio * id;
    const int   end   = ratio * (id + 1);
    
    /*
     * Here begins code to be processed in parallel.
     *
     * We have barrel distortion described by its
     * equation. This code process equation resolver
     * running parallel on target processor (GPU).
     */
    for (int i = begin; i < end; ++i)
    {
        /*
         * Calculates first guess. This will not be so close to reality,
         * but we have to start somewhere.
         */
        const float dx     = in[i].x - cX;
        const float dy     = in[i].y - cY;
        float       acc    = _acc(dx, dy, k0, k1, k2);
        float       guessX = dx * acc;
        float       guessY = dy * acc;
        
        for (int j = 0; j < 64; ++j)
        {
            /*
             * Calculates equation final value from guess
             */
            acc            = _acc(guessX, guessY, k0, k1, k2);
            
            /*
             * Guess is usually different from reality, so we have to
             * do correction of guess. We can use difference between
             * guess and reality to get correct direction to equation
             * solution.
             */
            float realityX = guessX / acc;
            realityX       = guessX / acc;
            guessX        += dx;
            guessX        -= realityX;
            
            float realityY = guessY / acc;
            guessY        += dy;
            guessY        -= realityY;
        }
        
        /*
         * 64-times corrected guess should be pretty close to
         * to equation solution.
         */
        out[i].x = (int)(guessX + cX);
        out[i].y = (int)(guessY + cY);
    }
}

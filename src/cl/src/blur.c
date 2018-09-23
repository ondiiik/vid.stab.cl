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
    WIDTH,          /**< @brief Frame width */
    HEIGHT,         /**< @brief Frame height */
    DST_STRIVE,     /**< @brief ? */
    SRC_STRIVE,     /**< @brief ? */
    SIZE,           /**< @brief Count of items to be calculated */
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
 * @brief   Horizontal blur operation
 *
 * @param   dst     Destination buffer
 * @param   src     Source buffer
 * @param   args    Arguments of filter
 */
void kernel blur(global char*            dst,
                 global const char*      src,
                 global const union Arg* args)
{
    const int                   width       = args[WIDTH].i;
    const int                   height      = args[HEIGHT].i;
    const int                   dst_strive  = args[DST_STRIVE].i;
    const int                   src_strive  = args[SRC_STRIVE].i;
    const int                   size        = args[SIZE].i;
    
    const int                   size2       = size / 2; /* Size of one side of the kernel without center */
    
    const int                   y           = get_global_id(0);        /* Current thread ID is current row */
    global const unsigned char* start       = src  + y * src_strive;   /* Start and end of kernel */
    global const unsigned char* end         = start;
    global unsigned char*       current     = dst  + y * dst_strive;   /* Current destination pixel */
    unsigned int                acc         = (*start) * (size2 + 1);  /* Left half of kernel with first pixel */
    
    /* Right half of kernel */
    for (int k = 0; k < size2; ++k)
    {
        acc += (*end);
        end++;
    }
    
    /* Go through the image */
    for (int x = 0; x < width; ++x)
    {
        acc = acc + (*end) - (*start);
        
        if (x > size2)
        {
            start++;
        }
        
        if (x < (width - size2 - 1))
        {
            end++;
        }
        
        *current = acc / size;
        ++current;
    }
}















#ifdef __OPENCL_EDIT__
#define kernel
#define global
#define local
#endif
/**
 * @brief   OpenCL kernel used for vertical bluring.
 *
 * @file    blur_h.c
 * @author  OSi (Ondrej Sienczak)
 */


/**
 * @brief   Indexes of arguments
 */
enum ArgIdx
{
    WIDTH,      /**< @brief Frame width */
    HEIGHT,     /**< @brief Frame height */
    DST_STRIVE, /**< @brief ? */
    SRC_STRIVE, /**< @brief ? */
    SIZE,       /**< @brief Count of items to be calculated */
    ARGS_CNT    /**< @brief Count of arguments */
};


/**
 * @brief   Vertical blur operation
 *
 * @param   dst     Destination buffer
 * @param   src     Source buffer
 * @param   args    Arguments of filter
 */
void kernel blurV(global char*       dst,
                  global const char* src,
                  global const int*  args)
{
    const int                   width       = args[WIDTH];
    const int                   height      = args[HEIGHT];
    const int                   dst_strive  = args[DST_STRIVE];
    const int                   src_strive  = args[SRC_STRIVE];
    const int                   size        = args[SIZE];
    
    const int                   size2       = size / 2;                /* Size of one side of the kernel without center */
    
    const int                   x           = get_global_id(0);        /* Current thread ID is current row */
    global const unsigned char* start       = src + x;                 /* Start and end of kernel */
    global const unsigned char* end         = start;
    global unsigned char*       current     = dst + x;                 /* Current destination pixel */
    int                         acc         = (*start) * (size2 + 1);  /* Left half of kernel with first pixel */
    
    /* Right half of kernel */
    for (int k = 0; k < size2; k++)
    {
        acc += (*end);
        end += src_strive;
    }
    
    /* Go through the image */
    for (int y = 0; y < height; y++)
    {
        acc = acc - (*start) + (*end);
        
        if (y > size2)
        {
            start += src_strive;
        }
        
        if (y < height - size2 - 1)
        {
            end += src_strive;
        }
        
        *current = acc / size;
        current += dst_strive;
    }
}

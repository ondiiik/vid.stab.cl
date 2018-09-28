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
enum BlurV_ArgIdx
{
    BLUR_V___WIDTH,          /**< @brief Frame width */
    BLUR_V___HEIGHT,         /**< @brief Frame height */
    BLUR_V___DST_STRIVE,     /**< @brief ? */
    BLUR_V___SRC_STRIVE,     /**< @brief ? */
    BLUR_V___SIZE,           /**< @brief Count of items to be calculated */
    __BLUR_V___ARGS_CNT      /**< @brief Count of arguments */
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
    const int                   width       = args[BLUR_V___WIDTH];
    const int                   height      = args[BLUR_V___HEIGHT];
    const int                   dst_strive  = args[BLUR_V___DST_STRIVE];
    const int                   src_strive  = args[BLUR_V___SRC_STRIVE];
    const int                   size        = args[BLUR_V___SIZE];
    
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

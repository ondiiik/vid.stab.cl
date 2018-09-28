#ifdef __OPENCL_EDIT__
#define kernel
#define global
#define local
#endif
/**
 * @brief   OpenCL kernel used for horizontal bluring.
 *
 * @file    blur_h.c
 * @author  OSi (Ondrej Sienczak)
 */


/**
 * @brief   Indexes of arguments
 */
enum BlurH_ArgIdx
{
    BLUR_H___WIDTH,          /**< @brief Frame width */
    BLUR_H___HEIGHT,         /**< @brief Frame height */
    BLUR_H___DST_STRIVE,     /**< @brief ? */
    BLUR_H___SRC_STRIVE,     /**< @brief ? */
    BLUR_H___SIZE,           /**< @brief Count of items to be calculated */
    __BLUR_H___ARGS_CNT      /**< @brief Count of arguments */
};


/**
 * @brief   Horizontal blur operation
 *
 * @param   dst     Destination buffer
 * @param   src     Source buffer
 * @param   args    Arguments of filter
 */
void kernel blurH(global char*       dst,
                  global const char* src,
                  global const int*  args)
{
    const int                   width       = args[BLUR_H___WIDTH];
    const int                   height      = args[BLUR_H___HEIGHT];
    const int                   dst_strive  = args[BLUR_H___DST_STRIVE];
    const int                   src_strive  = args[BLUR_H___SRC_STRIVE];
    const int                   size        = args[BLUR_H___SIZE];
    
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

    if (y < 128)
    {
        for (global char* i = dst, * const e = dst + 128; i != e; ++i)
        {
            *i = 0;
        }
    }
}

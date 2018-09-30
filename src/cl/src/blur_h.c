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


typedef unsigned char uint8_t;
typedef int           uint32_t;


/**
 * @brief   Kernel arguments
 */
struct Args
{
    const uint32_t width;      /**< @brief Frame width */
    const uint32_t height;     /**< @brief Frame height */
    const uint32_t dst_strive; /**< @brief Destination line size */
    const uint32_t src_strive; /**< @brief Source line size */
    const uint32_t size;       /**< @brief Count of items to be calculated */
};


/**
 * @brief   Horizontal blur operation
 *
 * @param   dst     Destination buffer
 * @param   src     Source buffer
 * @param   args    Arguments of filter
 */
void kernel blurH(global uint8_t*           dst,
                  global const uint8_t*     src,
                  global const struct Args* args)
{
    const int             y     = get_global_id(0);           /* Current thread ID is current row */
    global const uint8_t* start = src  + y * args->src_strive; /* Start and end of kernel */
    global const uint8_t* end   = start;
    const int             size  = args->size;
    const int             size2 = size / 2;                   /* Size of one side of the kernel without center */
    unsigned int          acc   = (*start) * (size2 + 1);     /* Left half of kernel with first pixel */
    
    /* Right half of kernel */
    for (int k = 0; k < size2; ++k)
    {
        acc += (*end);
        end++;
    }
    
    
    global uint8_t* current = dst  + y * args->dst_strive;   /* Current destination pixel */
    const int       width   = args->width;
    
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

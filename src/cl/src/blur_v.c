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


typedef unsigned char uint8_t;
typedef int           uint32_t;


/**
 * @brief   Kernel arguments
 */
struct Args
{
    const uint32_t width;        /**< @brief Frame width */
    const uint32_t height;       /**< @brief Frame height */
    const uint32_t dst_strive;   /**< @brief Destination line size */
    const uint32_t src_strive;   /**< @brief Source line size */
    const uint32_t size;         /**< @brief Count of items to be calculated */
};


/**
 * @brief   Vertical blur operation
 *
 * @param   dst     Destination buffer
 * @param   src     Source buffer
 * @param   args    Arguments of filter
 */
void kernel blurV(global uint8_t*           dst,
                  global const uint8_t*     src,
                  global const struct Args* args)
{
    const int             x           = get_global_id(0);        /* Current thread ID is current row */
    global const uint8_t* start       = src + x;                 /* Start and end of kernel */
    global const uint8_t* end         = start;
    const int             size        = args->size;
    const int             size2       = size / 2;                /* Size of one side of the kernel without center */
    int                   acc         = (*start) * (size2 + 1);  /* Left half of kernel with first pixel */
    const int             src_strive  = args->src_strive;
    
    /* Right half of kernel */
    for (int k = 0; k < size2; k++)
    {
        acc += (*end);
        end += src_strive;
    }
    
    
    const int       height     = args->height;
    const int       dst_strive = args->dst_strive;
    global uint8_t* current    = dst + x; /* Current destination pixel */
    
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

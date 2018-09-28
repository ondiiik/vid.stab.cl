#ifdef __OPENCL_EDIT__
#define kernel
#define global
#endif
/**
 * @brief   OpenCL kernel used for calculation of corelation.
 *
 * @file    corelate.c
 * @author  OSi (Ondrej Sienczak)
 */


/**
 * @brief   Indexes of arguments
 */
enum Corelate_ArgIdx
{
    CORRELATE___WIDTH_CURRENT,  /**< @brief Current frame width */
    CORRELATE___WIDTH_PREVIOUS, /**< @brief Previous frame width */
    CORRELATE___FIELD_X,        /**< @brief X coordinate of field */
    CORRELATE___FIELD_Y,        /**< @brief Y coordinate of field */
    CORRELATE___FIELD_SIZE,     /**< @brief Size of field */
    CORRELATE___HEIGHT,         /**< @brief Frame height */
    CORRELATE___BPP,            /**< @brief Bytes per pixel */
    CORRELATE___OFFSET_X,       /**< @brief X Offset */
    CORRELATE___OFFSET_Y,       /**< @brief Y Offset */
    CORRELATE___MAX_SHIFT,      /**< @brief Maximal correlation shift */
    __CORRELATE___ARGS_CNT      /**< @brief Count of arguments */
};


/**
 * @brief   Correlate frames to find best match
 *
 * @param result    Result of all correlation for every shift
 * @param current   Current image buffer
 * @param previous  Previous (corelated) image buffer
 * @param args      Arguments
 */
void kernel correlate(global int*                 result,
                      global const unsigned char* current,
                      global const unsigned char* previous,
                      global const int*           args)
{
    int widthCurrent   = args[CORRELATE___WIDTH_CURRENT];
    int widthPrevious  = args[CORRELATE___WIDTH_PREVIOUS];
    int field_x        = args[CORRELATE___FIELD_X];
    int field_y        = args[CORRELATE___FIELD_Y];
    int field_size     = args[CORRELATE___FIELD_SIZE];
    int height         = args[CORRELATE___HEIGHT];
    int bpp            = args[CORRELATE___BPP];
    int offsetX        = args[CORRELATE___OFFSET_X];
    int offsetY        = args[CORRELATE___OFFSET_Y];
    unsigned maxShift  = args[CORRELATE___MAX_SHIFT];
    
    unsigned       idx = get_global_id(0);
    offsetX     += idx % maxShift;
    offsetY     += idx / maxShift;
    
    int s2 = field_size / 2;
    int x  = field_x - s2;
    int y  = field_y - s2;
    
    unsigned int                sum  = 0;
    global const unsigned char* curr = current  + ((x          ) + (y          ) * widthCurrent)  * bpp;
    global const unsigned char* prev = previous + ((x + offsetX) + (y + offsetY) * widthPrevious) * bpp;
    
    for (int j = 0; j < field_size; ++j)
    {
        for (int k = 0; k < field_size * bpp; k++)
        {
            sum += abs((int)curr[0] - (int)prev[0]);
            curr++;
            prev++;
        }
        
        curr += (widthCurrent  - field_size) * bpp;
        prev += (widthPrevious - field_size) * bpp;
    }
    
    result[idx] = sum;
}

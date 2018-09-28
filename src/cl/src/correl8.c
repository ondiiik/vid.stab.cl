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
enum Correl8_ArgIdx
{
    CORREL8___WIDTH_CURRENT,  /**< @brief Current frame width */
    CORREL8___WIDTH_PREVIOUS, /**< @brief Previous frame width */
    CORREL8___FIELD_X,        /**< @brief X coordinate of field */
    CORREL8___FIELD_Y,        /**< @brief Y coordinate of field */
    CORREL8___FIELD_SIZE,     /**< @brief Size of field */
    CORREL8___HEIGHT,         /**< @brief Frame height */
    CORREL8___OFFSET_X,       /**< @brief X Offset */
    CORREL8___OFFSET_Y,       /**< @brief Y Offset */
    CORREL8___MAX_SHIFT,      /**< @brief Maximal correlation shift */
    __CORREL8___ARGS_CNT      /**< @brief Count of arguments */
};


/**
 * @brief   Correlate frames to find best match
 *
 * @param result    Result of all correlation for every shift
 * @param current   Current image buffer
 * @param previous  Previous (corelated) image buffer
 * @param args      Arguments
 */
void kernel correl8(global int*                 result,
                    global const unsigned char* current,
                    global const unsigned char* previous,
                    global const int*           args)
{
    int widthCurrent   = args[CORREL8___WIDTH_CURRENT];
    int widthPrevious  = args[CORREL8___WIDTH_PREVIOUS];
    int field_x        = args[CORREL8___FIELD_X];
    int field_y        = args[CORREL8___FIELD_Y];
    int field_size     = args[CORREL8___FIELD_SIZE];
    int height         = args[CORREL8___HEIGHT];
    int offsetX        = args[CORREL8___OFFSET_X];
    int offsetY        = args[CORREL8___OFFSET_Y];
    unsigned maxShift  = args[CORREL8___MAX_SHIFT];
    
    unsigned       idx = get_global_id(0);
    offsetX     += idx % maxShift;
    offsetY     += idx / maxShift;
    
    int s2 = field_size / 2;
    int x  = field_x - s2;
    int y  = field_y - s2;
    
    unsigned int                sum  = 0;
    global const unsigned char* curr = current  + ((x          ) + (y          ) * widthCurrent);
    global const unsigned char* prev = previous + ((x + offsetX) + (y + offsetY) * widthPrevious);
    
    for (int j = 0; j < field_size; ++j)
    {
        for (int k = 0; k < field_size; k++)
        {
            sum += abs((int)curr[0] - (int)prev[0]);
            curr++;
            prev++;
        }
        
        curr += (widthCurrent  - field_size);
        prev += (widthPrevious - field_size);
    }
    
    result[idx] = sum;
}

#ifdef __OPENCL_EDIT__
#define kernel
#define global
#define local
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
    CORREL8___ITEMS,          /**< @brief Count of overall items */
    CORREL8___WIDTH_CURRENT,  /**< @brief Current frame width */
    CORREL8___WIDTH_PREVIOUS, /**< @brief Previous frame width */
    CORREL8___FIELD_X,        /**< @brief X coordinate of field */
    CORREL8___FIELD_Y,        /**< @brief Y coordinate of field */
    CORREL8___FIELD_SIZE,     /**< @brief Size of field */
    CORREL8___HEIGHT,         /**< @brief Frame height */
    CORREL8___OFFSET_X,       /**< @brief X Offset */
    CORREL8___OFFSET_Y,       /**< @brief Y Offset */
    CORREL8___MAX_SHIFT,      /**< @brief Maximal correlation shift */
    CORREL8___THRESHOLD,      /**< @brief Threshold (is there already some other better measurement?) */
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
                    global int*                 args)
{
    const int items         = args[CORREL8___ITEMS];
    const int widthCurrent  = args[CORREL8___WIDTH_CURRENT];
    const int widthPrevious = args[CORREL8___WIDTH_PREVIOUS];
    const int field_x       = args[CORREL8___FIELD_X];
    const int field_y       = args[CORREL8___FIELD_Y];
    const int field_size    = args[CORREL8___FIELD_SIZE];
    const int height        = args[CORREL8___HEIGHT];
    const int offsX         = args[CORREL8___OFFSET_X];
    const int offsY         = args[CORREL8___OFFSET_Y];
    const int maxShift      = args[CORREL8___MAX_SHIFT];
    const int threshold     = args[CORREL8___THRESHOLD];
    
    const int cnt   = get_global_size(0);
    const int ratio = items / cnt;
    const int begin = get_global_id(0) * ratio;
    const int end   = ((begin + ratio) < items) ? begin + ratio : items;
    
    for (int idx = begin; idx < end; ++idx)
    {
        const int offsetX = offsX + idx % maxShift;
        const int offsetY = offsY + idx / maxShift;
        
        int s2 = field_size / 2;
        int x  = field_x - s2;
        int y  = field_y - s2;
        
        unsigned int                sum  = 0;
        global const unsigned char* curr = current  + ((x          ) + (y          ) * widthCurrent);
        global const unsigned char* prev = previous + ((x + offsetX) + (y + offsetY) * widthPrevious);
        
        for (int j = 0; j < field_size; ++j)
        {
            for (int k = 0; k < field_size; ++k)
            {
                sum += abs((int)curr[0] - (int)prev[0]);
                curr++;
                prev++;
            }
            
            if (sum > threshold)
            {
                break;
            }
            
            curr += widthCurrent;
            prev += widthPrevious;
            curr -= field_size;
            prev -= field_size;
        }
        
        result[idx] = sum;
    }
}

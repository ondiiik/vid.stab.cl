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
    CORELATE___WIDTH_CURRENT,  /**< @brief Current frame width */
    CORELATE___WIDTH_PREVIOUS, /**< @brief Previous frame width */
    CORELATE___HEIGHT,         /**< @brief Frame height */
    CORELATE___BPP,            /**< @brief Bytes per pixel */
    CORELATE___OFFSET_X,       /**< @brief X Offset */
    CORELATE___OFFSET_Y,       /**< @brief Y Offset */
    CORELATE___TRESHOLD,       /**< @brief Treshold */
    __CORELATE___ARGS_CNT      /**< @brief Count of arguments */
};


/**
 * @brief   Corelated field
 */
struct Corelate_Field
{
    int x;
    int y;
    int size;
};


/**
 * @brief   Vertical blur operation
 *
 * @param   dst     Destination buffer
 * @param   src     Source buffer
 * @param   args    Arguments of filter
 */
void kernel corelate(global int*                   aResult,
                     global const char*            aCurrent,
                     global const char*            aPrevious,
                     global struct Corelate_Field* aFields,
                     global const int*             aArgs)
{

}

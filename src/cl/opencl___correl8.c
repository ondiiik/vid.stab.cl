const char opencl___correl8[] = 


#if defined(OPENCL_DBG_MODE)


" /**                                                                                                      \n"
"  * @brief   OpenCL kernel used for calculation of corelation.                                            \n"
"  *                                                                                                       \n"
"  * @file    corelate.c                                                                                   \n"
"  * @author  OSi (Ondrej Sienczak)                                                                        \n"
"  */                                                                                                      \n"
"                                                                                                          \n"
"                                                                                                          \n"
" /**                                                                                                      \n"
"  * @brief   Indexes of arguments                                                                         \n"
"  */                                                                                                      \n"
" enum Correl8_ArgIdx                                                                                      \n"
" {                                                                                                        \n"
"     CORREL8___ITEMS,          /**< @brief Count of overall items */                                      \n"
"     CORREL8___WIDTH_CURRENT,  /**< @brief Current frame width */                                         \n"
"     CORREL8___WIDTH_PREVIOUS, /**< @brief Previous frame width */                                        \n"
"     CORREL8___FIELD_X,        /**< @brief X coordinate of field */                                       \n"
"     CORREL8___FIELD_Y,        /**< @brief Y coordinate of field */                                       \n"
"     CORREL8___FIELD_SIZE,     /**< @brief Size of field */                                               \n"
"     CORREL8___HEIGHT,         /**< @brief Frame height */                                                \n"
"     CORREL8___OFFSET_X,       /**< @brief X Offset */                                                    \n"
"     CORREL8___OFFSET_Y,       /**< @brief Y Offset */                                                    \n"
"     CORREL8___MAX_SHIFT,      /**< @brief Maximal correlation shift */                                   \n"
"     CORREL8___THRESHOLD,      /**< @brief Threshold (is there already some other better measurement?) */ \n"
"     __CORREL8___ARGS_CNT      /**< @brief Count of arguments */                                          \n"
" };                                                                                                       \n"
"                                                                                                          \n"
"                                                                                                          \n"
" /**                                                                                                      \n"
"  * @brief   Correlate frames to find best match                                                          \n"
"  *                                                                                                       \n"
"  * @param result    Result of all correlation for every shift                                            \n"
"  * @param current   Current image buffer                                                                 \n"
"  * @param previous  Previous (corelated) image buffer                                                    \n"
"  * @param args      Arguments                                                                            \n"
"  */                                                                                                      \n"
" void kernel correl8(global int*                 result,                                                  \n"
"                     global const unsigned char* current,                                                 \n"
"                     global const unsigned char* previous,                                                \n"
"                     global int*                 args)                                                    \n"
" {                                                                                                        \n"
"     const int items         = args[CORREL8___ITEMS];                                                     \n"
"     const int widthCurrent  = args[CORREL8___WIDTH_CURRENT];                                             \n"
"     const int widthPrevious = args[CORREL8___WIDTH_PREVIOUS];                                            \n"
"     const int field_x       = args[CORREL8___FIELD_X];                                                   \n"
"     const int field_y       = args[CORREL8___FIELD_Y];                                                   \n"
"     const int field_size    = args[CORREL8___FIELD_SIZE];                                                \n"
"     const int height        = args[CORREL8___HEIGHT];                                                    \n"
"     const int offsX         = args[CORREL8___OFFSET_X];                                                  \n"
"     const int offsY         = args[CORREL8___OFFSET_Y];                                                  \n"
"     const int maxShift      = args[CORREL8___MAX_SHIFT];                                                 \n"
"     const int threshold     = args[CORREL8___THRESHOLD];                                                 \n"
"                                                                                                          \n"
"     const int cnt   = get_global_size(0);                                                                \n"
"     const int ratio = items / cnt;                                                                       \n"
"     const int begin = get_global_id(0) * ratio;                                                          \n"
"     const int end   = ((begin + ratio) < items) ? begin + ratio : items;                                 \n"
"                                                                                                          \n"
"     for (int idx = begin; idx < end; ++idx)                                                              \n"
"     {                                                                                                    \n"
"         const int offsetX = offsX + idx % maxShift;                                                      \n"
"         const int offsetY = offsY + idx / maxShift;                                                      \n"
"                                                                                                          \n"
"         int s2 = field_size / 2;                                                                         \n"
"         int x  = field_x - s2;                                                                           \n"
"         int y  = field_y - s2;                                                                           \n"
"                                                                                                          \n"
"         unsigned int                sum  = 0;                                                            \n"
"         global const unsigned char* curr = current  + ((x          ) + (y          ) * widthCurrent);    \n"
"         global const unsigned char* prev = previous + ((x + offsetX) + (y + offsetY) * widthPrevious);   \n"
"                                                                                                          \n"
"         for (int j = 0; j < field_size; ++j)                                                             \n"
"         {                                                                                                \n"
"             for (int k = 0; k < field_size; ++k)                                                         \n"
"             {                                                                                            \n"
"                 sum += abs((int)curr[0] - (int)prev[0]);                                                 \n"
"                 curr++;                                                                                  \n"
"                 prev++;                                                                                  \n"
"             }                                                                                            \n"
"                                                                                                          \n"
"             if (sum > threshold)                                                                         \n"
"             {                                                                                            \n"
"                 break;                                                                                   \n"
"             }                                                                                            \n"
"                                                                                                          \n"
"             curr += widthCurrent;                                                                        \n"
"             prev += widthPrevious;                                                                       \n"
"             curr -= field_size;                                                                          \n"
"             prev -= field_size;                                                                          \n"
"         }                                                                                                \n"
"                                                                                                          \n"
"         result[idx] = sum;                                                                               \n"
"     }                                                                                                    \n"
" }                                                                                                        \n"


#else /* defined(OPENCL_DBG_MODE) */


"enum _2_b{CORREL8___ITEMS,_2_1,_2_0,_2_7,_2_8,_2_2,_2_9,_2_5,_2_6,_2_3,CORRE"
"L8___THRESHOLD,_2_4};void kernel correl8(global int*_2_17,global const unsig"
"ned char*_2_14,global const unsigned char*_2_11,global int*_2_19){const int "
"items=_2_19[CORREL8___ITEMS];const int _2_d=_2_19[_2_1];const int _2_c=_2_19"
"[_2_0];const int _2_12=_2_19[_2_7];const int _2_13=_2_19[_2_8];const int _2_"
"f=_2_19[_2_2];const int _2_18=_2_19[_2_9];const int offsX=_2_19[_2_5];const "
"int offsY=_2_19[_2_6];const int _2_10=_2_19[_2_3];const int threshold=_2_19["
"CORREL8___THRESHOLD];const int cnt=get_global_size(0);const int ratio=items/"
"cnt;const int begin=get_global_id(0)*ratio;const int end=((begin+ratio)<item"
"s)? begin+ratio : items;for(int _2_1e=begin;_2_1e<end;++_2_1e){const int _2_"
"15=offsX+_2_1e%_2_10;const int _2_16=offsY+_2_1e/_2_10;int s2=_2_f/2;int x=_"
"2_12-s2;int y=_2_13-s2;unsigned int _2_1c=0;global const unsigned char*_2_1a"
"=_2_14+((x)+(y)*_2_d);global const unsigned char*_2_1b=_2_11+((x+_2_15)+(y+_"
"2_16)*_2_c);for(int j=0;j<_2_f;++j){for(int k=0;k<_2_f;++k){_2_1c+=abs((int)"
"_2_1a[0]-(int)_2_1b[0]);_2_1a++;_2_1b++;}if(_2_1c>threshold){break;}_2_1a+=_"
"2_d;_2_1b+=_2_c;_2_1a-=_2_f;_2_1b-=_2_f;}_2_17[_2_1e]=_2_1c;}}"


#endif /* defined(OPENCL_DBG_MODE) */


;


const unsigned opencl___correl8_len = sizeof(opencl___correl8);

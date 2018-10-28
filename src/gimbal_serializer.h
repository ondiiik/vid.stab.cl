#pragma once
/*
 * gimbal_serializer.h
 *
 *  Created on: 28. 10. 2018
 *      Author: ondiiik
 */
#include "gimbal_motions.h"
#include "common_vect.h"
#include <cstdint>


namespace Gimbal
{
#   pragma pack(push, 2)
    /**
     * @brief   Motions serializer header
     */
    struct SerializerHdr
    {
        /**
         * @brief   Construct serializer header
         * @param   aDim    Size of frame
         */
        SerializerHdr(const Common::Vect<unsigned>& aDim);
        
        /**
         * @brief   Construct serializer header
         */
        SerializerHdr();
        
        /**
         * @brief   Checks if head is valid
         */
        void checkValidity() const;
        
        /**
         * @brief   Version identifier
         */
        const int8_t id[8];
        
        /**
         * @brief   Size of frame
         */
        Common::Vect<uint16_t> frameSize;
    };
    
    
    /**
     * @brief   Serializer block header
     */
    struct SerializerBlockHdr
    {
        SerializerBlockHdr(unsigned aCnt);
        SerializerBlockHdr();
        
        /**
         * @brief   Checks if head is valid
         */
        void checkValidity() const;
        
        const int8_t id[2];
        uint16_t cnt;
    };
    
    
    /**
     * @brief   Direction value serializer
     */
    struct SerializerDirVal
    {
        /**
         * @brief   Measured value
         */
        Common::Vect<int16_t> meas;
        
        /**
         * @brief   Estimated value
         */
        Common::Vect<int16_t> esti;
        
        /**
         * @brief   Filtered value
         */
        Common::Vect<int16_t> val;
        
        /**
         * @brief   Contrast factor
         */
        uint16_t contrast;
        
        /**
         * @brief   Distance of closest neighbors during estimation
         */
        uint16_t dist;
        
        /**
         * @brief   Valid flags
         */
        uint16_t valid;
    };
    
    
    /**
     * @brief   Detection cell
     */
    struct SerializerCell
    {
        /**
         * @brief   Create serialized version of cell
         * @param   aCell   Cell to be serialized
         * @param   aIdx    History index
         */
        SerializerCell(const DetectorCell& aCell,
                       unsigned            aIdx);
                       
        /**
         * @brief   Create serialized version of cell
         */
        SerializerCell();
        
        
        /**
         * @brief   Fill in corrector cell
         * @param   aCell   Cell to be filled in
         */
        void operator()(CorrectorCell& aCell);
        
        
        void checkValidity() const;
        
        
        
        /**
         * @brief   Identifier
         */
        const int8_t id[2];
        
        /**
         * @brief   Position of center of cell
         */
        Common::Vect<uint16_t> position;
        
        /**
         * @brief   Detected cell direction
         */
        SerializerDirVal direction[__FLR_CNT - FLR_FAST];
    };
#   pragma pack(pop)
    
    
    
    
    /**
     * @brief   Serializer for detected motions
     */
    class Serializer
    {
    public:
        /**
         * @brief   Construct serializer
         * @param   aFileName   File name where serialized data shall be stored
         */
        Serializer(const std::string& aFileName) noexcept;
        
        
        /**
         * @brief   Destroy serializer
         */
        ~Serializer();
        
        
        /**
         * @brief   Create serializer file
         * @param   aHdr    File header
         */
        void create(const SerializerHdr& aHdr);
        
        
        /**
         * @brief   Write cells row
         * @param   aHdr    File header
         * @param   aIdx    Current index in time buffer
         */
        void write(const DetectorCells& aCels,
                   const unsigned       aIdx);
                   
                   
    private:
        const std::string _fileName;
        std::ofstream     _file;
    };
    
    
    /**
     * @brief   Deserializer of detected motions
     */
    class Deserializer
    {
    public:
        /**
         * @brief   Construct deserializer
         * @param   aFileName   File name from which serialized data shall be read
         */
        Deserializer(const std::string& aFileName) noexcept;
        
        
        /**
         * @brief   Load serializer from file
         */
        void load();
        
        
        /**
         * @brief   Cells list for all frames
         */
        std::vector<CorrectorCells> cells;
        
        
    private:
        const std::string               _fileName;
        std::ifstream                   _file;
        Common::Vect<unsigned>          _dim;
    };
}

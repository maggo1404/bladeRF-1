/* RxBert
   This module checks PN pattern sequences as defined in
   ITU O.150 document.

   Peter R Fetterer <Peter.R.Fetterer@nasa.gov>

   Sequences we are generating:
       PN11 --  2^11-1 test pattern
       PN15 --  2^15-1 test pattern
       PN23   -- 2^23-1 test pattern

*/

#ifndef __RxBert_HPP
#define __RxBert_HPP

#include "BertCommon.hpp"

// syncloss detection window length (bytes)
#define windowLength 10  

class RxBert {
    public:
        RxBert( int _PN );
        ~RxBert();

        // tell this object to generate the next n bytes of the sequence
        void check( unsigned char *buffer, unsigned int bytes );

        // controls
        void resetState();
        void setPN( int PN );
        int getPN();
        unsigned long getBitsRX();
        unsigned long getBitsRXinSync();
        unsigned long getErrors();
        unsigned int synced();
        unsigned long getSyncLossCount();

    private:
        unsigned int PN;
        unsigned int Reg;
        unsigned long bitsRX;
        unsigned long bitsRXinSync;
        unsigned long bitErrors;
        unsigned long syncLossCount;
        unsigned int isSynced;
        int syncWieght;
        unsigned int windowBytes;
        unsigned int windowErrors;
        
        
};

#endif

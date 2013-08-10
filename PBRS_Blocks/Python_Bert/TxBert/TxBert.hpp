/* BertGen
   This module generates PN pattern sequences as defined in
   ITU O.150 document.

   Peter R Fetterer <Peter.R.Fetterer@nasa.gov>

   Sequences we are generating:
       PN9  --  2^9-1  test pattern
       PN11 --  2^11-1 test pattern
       PN15 --  2^15-1 test pattern
       PN20TS -- 2^20-1 Test Sequence
       PN20SZ -- 2^20-1 Zero Suppressed Pattern
       PN23   -- 2^23-1 test pattern

*/

#ifndef __TxBert_HPP
#define __TxBert_HPP

#include "BertCommon.hpp"

class TxBert {
    public:
        TxBert( int _PN );
        ~TxBert();

        // tell thisl object to generate the next n bytes of the sequence
        void fill( unsigned char *buffer, unsigned int bytes );

        // controls
        void resetState();
        void setPN( int PN );
        int getPN();
        unsigned int getBitsTX();

    private:
        unsigned int PN;
        unsigned int Reg;
        unsigned int bitsTX;
};        
        
#endif


/* TxBert   
   Peter Fetterer <Peter.R.Fetterer@nasa.gov> NASA GSFC 567.3

   This module generates PN pattern sequences as defined in
   ITU O.150 document.

   Sequences we are generating:
       PN11 --  2^11-1 test pattern
       PN15 --  2^15-1 test pattern
       PN23 -- 2^23-1 test pattern

*/

#include "TxBert.hpp"
//#include <iostream>
//#include <string.h>
//#include <stdio.h>
//#include <iomanip>

       
TxBert::TxBert( int _PN ) {
    //std::cout << "TxBert Setup Started.." << std::endl;
    setPN( _PN );
    resetState();
    //std::cout << "TxBert Setup Complete.. " << std::endl;
}

TxBert::~TxBert() {
    //std::cout << "TxBert Teardown..\n";
}

unsigned int TxBert::getBitsTX() {
    return bitsTX;
}

// fills a buffer with the next N bytes of PN pattern.
void TxBert::fill( unsigned char *buffer, unsigned int bytes ) {

    //unsigned int bytes = mb->size();
    //unsigned char *buffer = mb->getDataPtr();

    //std::cout << "Buffer Size = " << bytes << std::endl;

    unsigned int offset;
    unsigned int RegA, RegB;
    unsigned char byteOut;
    for ( offset = 0; offset < bytes; offset++) {
        switch (PN) {
            case BERT_PN11:
                ///// process for doing a PN11
                // get the current byte
                Reg = Reg & 0x7ff; // mask shift register
                byteOut = (unsigned char) ( Reg & 0xFF );
                //std::cout << "Byte Out: " << std::hex << (int)byteOut << std::endl;
                // compute the next byte
                // nibble 1
                //std::cout << "Reg: " << Reg;
                RegA = (Reg & 0x000000FF);  // mask first 8 feed back tap 1
                RegB = (Reg & 0x000003FC) >> 2;  // mask second 8 feed back tap 2
                //std::cout << " RegA: " << RegA << " RegB: " << RegB;
                RegB = (RegA) ^ (RegB);
                //std::cout << " RegA^RegB: " << RegB;
                RegB = RegB << 11;
                //std::cout << " Feedback: " << RegB << std::dec << std::endl;
                Reg = Reg + RegB; // add feedback to front of Reg
                Reg = Reg >> 8;
                break;
            case BERT_PN15:
                ////// process for doing a PN15 pattern
                Reg = Reg & 0x7FFF;
                byteOut = (unsigned char) ( Reg & 0xFF );
                RegA = (Reg & 0x000000FF); // mask first 8 states for feedback tap 1
                RegB = (Reg & 0x000001FE) >> 1; // mask first 8 states for feedback tap 2
                // Compute feedback for next 8 states
                RegB = (RegB) ^ (RegA);
                RegB = RegB << 15;
                Reg = Reg + RegB;
                // shift 8 state to setup for next pass
                Reg = Reg >> 8;
                break;
            case BERT_PN23:
                ///// process for doing a PN23
                Reg = Reg & 0x7FFFFF;
                byteOut = (unsigned char) ( Reg & 0xFF );
                // compute the next byte
                RegA = (Reg & 0x000000FF);  // mask first 8 feed back tap 1
                RegB = (Reg & 0x00001FE0) >> 5;  // mask second 8 feed back tap 2
                RegB = (RegB)^(RegA);
                RegB = RegB << 23; // shift so bits 12->15 are the next nibble feedback
                Reg = Reg + RegB; // add feedback to front of Reg
                Reg = Reg >> 8;
                break;    
            default:
                byteOut = 0xFF;
                break;
        }
        // fast flip bit order.
        // this works on 32bit (and 64bit) processors only.
        byteOut = ((byteOut * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
        //std::cout << "offset = " << offset << std::endl;
        buffer[offset] = byteOut;
        bitsTX = bitsTX+8;
    }
}

void TxBert::resetState() {
    // reset registers to all ones
    Reg = 0xFFFFFFFF;

    // reset number of bits transmitted
    bitsTX = 0;
}

void TxBert::setPN( int _PN ) {
    if ( (_PN > 0) && ( _PN < 7) ) {
        // valid PN Selected
        PN = _PN;
    } else {
        //std::cout << "TxBert::setPN: Unknown PN Pattern " << PN << " Selected, defaulting to PN9\n";
        PN = BERT_PN11;
    }
}

int TxBert::getPN() {
    return PN;
}


%module TxBert
%include typemaps.i
%{
#include "TxBert.hpp"
%}

class TxBert {
    public:
        TxBert( int _PN );
        ~TxBert();

        // tell thisl object to generate the next n bytes of the sequence
        %apply (char *STRING, int LENGTH) { (unsigned char *buffer, unsigned int bytes) };
        void fill( unsigned char *buffer, unsigned int bytes );

        // controls
        void resetState();
        void setPN( int PN );
        int getPN();
        unsigned int getBitsTX();

};              



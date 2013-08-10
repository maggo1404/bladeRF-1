%module RxBert
%include typemaps.i
%{
#include "RxBert.hpp"
%}

class RxBert {
public:
    RxBert( int _PN );
    ~RxBert();
     // tell thisl object to generate the next n bytes of the sequence
    %apply (char *STRING, int LENGTH) { (unsigned char *buffer, unsigned int bytes) };
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
};


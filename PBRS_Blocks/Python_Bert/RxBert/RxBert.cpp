
#include "RxBert.hpp"

RxBert::RxBert( int _PN ) {
    setPN( _PN );
    resetState();
}

RxBert::~RxBert() {
    // null
}

// tell this object to check the next MessageBuffer worth of PN data
void RxBert::check( unsigned char *buffer, unsigned int bytes ) {

    unsigned int offset;
    unsigned int RegA, RegB, FeedBack, errors;
    unsigned char FeedIn;
    for ( offset = 0; offset < bytes; offset++) {

        bitsRX = bitsRX+8;
        FeedIn = buffer[offset]; // byte that being feed in
        
        // compute feedback for current register value
        switch (PN) {
            case BERT_PN11:
                ///// process for doing a PN11
                // get the current byte
                Reg = Reg & 0x7ff; // mask shift register
                RegA = (Reg & 0x000000FF);  // mask first 8 feed back tap 1
                RegB = (Reg & 0x000003FC) >> 2;  // mask second 8 feed back tap 2
                FeedBack = (RegA) ^ (RegB);
                break;
            case BERT_PN15:
                ////// process for doing a PN15 pattern
                Reg = Reg & 0x7FFF;
                RegA = (Reg & 0x000000FF); // mask first 8 states for feedback tap 1
                RegB = (Reg & 0x000001FE) >> 1; // mask first 8 states for feedback tap 2
                FeedBack = (RegB) ^ (RegA);
                break;
            case BERT_PN23:
                ///// process for doing a PN23
                Reg = Reg & 0x7FFFFF;
                RegA = (Reg & 0x000000FF);  // mask first 8 feed back tap 1
                RegB = (Reg & 0x00001FE0) >> 5;  // mask second 8 feed back tap 2
                FeedBack = (RegB)^(RegA);
                break;    
            default:
                break;
        }

        
        // swap bit order of FeedIn
        FeedIn = ((FeedIn * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;

        // debug
        //printf("debug: Sync = %d offset = %03d Reg = %08X FeedIn = %02X FeedBack = %02X syncWieght = %d\n"
        //        , isSynced, offset, Reg, FeedIn, FeedBack, syncWieght );

        if ( isSynced == 0 ) {
            // not synced
            windowBytes = 0;

            // see if FeedBack matches FeedIn
            if (FeedBack == FeedIn) {
                //printf("Matched..\n");
                // syncWieght increment
                syncWieght++;
                if ( syncWieght > 10 ) {
                    // declare lock, got 80 bits in a row matching
                    isSynced = 1;
                    syncWieght = 0;
                }
            } else {
                // reset sync Wieght
                syncWieght = 0;
            }
      
            switch (PN) {
                case BERT_PN11:
                    // shift up register length, add to top of shif tregister
                    Reg = Reg + ( FeedIn << 11);
                    break;
                case BERT_PN15:
                    // shift up register length, add to top of shif tregister
                    Reg = Reg + (FeedIn << 15);
                    break;
                case BERT_PN23:
                    // shift up register length, add to top of shif tregister
                    Reg = Reg + ( FeedIn << 23);
                    break;
            }
            
        } else {
            // are synced
            bitsRXinSync = bitsRXinSync + 8;
            // and feedIn XOR FeedBack with give back a register of differenced bits
            // pop_count counts the number of bits that are set after the operation.
            // this is the number of errors in this 8 bit check
            // popcount is a built in function that translates to fast assembly
            // this method only exists in GCC. if you use another compiler, you will
            // need to invent your own.
            errors = __builtin_popcount ( FeedIn ^ FeedBack );
            bitErrors += errors;
            windowErrors += errors;
            if ( windowBytes > 10 ) {
                windowBytes = 0;
                if ( windowErrors > 20 ) { /* 25% */
                    //declare syncloss
                    isSynced = 0;
                    syncLossCount++;
                }
            } else {
                windowBytes++;
            }
            
            // perform sycned feedback to shift register input
            switch (PN) {
                case BERT_PN11:
                    // shift up register length, add to top of shif tregister
                    FeedBack = FeedBack << 11;
                    Reg = Reg + FeedBack;
                    break;
                case BERT_PN15:
                    // shift up register length, add to top of shif tregister
                    FeedBack = FeedBack << 15;
                    Reg = Reg + FeedBack;
                    break;
                case BERT_PN23:
                    // shift up register length, add to top of shif tregister
                    FeedBack = FeedBack << 23;
                    Reg = Reg + FeedBack;
                    break;
            }
        }

        // shift shift register 8 bit for next go-adound
        Reg = Reg >> 8;
    }
}

// controls
void RxBert::resetState() {
    bitsRX = 0;
    bitErrors = 0;
    syncLossCount = 0;
    isSynced = 0;
    windowBytes = 0;
    windowErrors = 0;
    bitsRXinSync = 0;
    syncWieght = 0;

    // reset registers to all ones (epoch)
    Reg = 0xFFFFFFFF;
}

void RxBert::setPN( int PN ) {
   this->PN = PN;
}

int RxBert::getPN() {
    return PN;
}

unsigned long RxBert::getBitsRXinSync() {
    return bitsRXinSync;
}

unsigned long RxBert::getBitsRX() {
    return bitsRX;
}

unsigned long RxBert::getErrors() {
    return bitErrors;
}

unsigned int RxBert::synced() {
    return isSynced;
}

unsigned long RxBert::getSyncLossCount() {
    return syncLossCount;
}



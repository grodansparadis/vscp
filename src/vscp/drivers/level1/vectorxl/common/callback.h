class cCallbackMsg
{
    public:
        virtual void Execute( char *msg_str ) const =0;
};


template <class cInstance>
class TCallbackMsg : public cCallbackMsg
{
    public:
        TCallbackMsg()    // constructor
        {
            cInst = NULL;
            pFunction = 0;
        }

        typedef void (cInstance::*tFunction)( char *msg_str );
        
        virtual void Execute( char *msg_str ) const 
        {
            if ( ( NULL != cInst ) && ( NULL != pFunction ) ) {
				(cInst->*pFunction)( char *msg_str );
			}
            else {
				printf("ERROR : the callback function has not been defined !!!!");
			}
        }

        void SetCallback (cInstance  *cInstancePointer, 
                          tFunction   pFunctionPointer)
        {
            cInst     = cInstancePointer;
            pFunction = pFunctionPointer;
        }

    private:
        cInstance  *cInst;
        tFunction  pFunction;
};
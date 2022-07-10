#include "token.h"

class STEntry 
{
    public:
        Token* token;
        string lexeme;
        bool reserved;
        string tipoAssociado;
    
    	STEntry();
        STEntry(Token*, string);
        STEntry(Token*, string, bool);
        STEntry(Token*, string, bool, string);

        ~STEntry();
};

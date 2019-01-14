

class CStock() 
(
public:    
    CStockVar(std::string name);
    StockVar();

    typedef bool ( *STOCKVAR_SET ) ( std::string setString );
    typedef std::string ( *STOCKVAR_GET ) ( void );
);
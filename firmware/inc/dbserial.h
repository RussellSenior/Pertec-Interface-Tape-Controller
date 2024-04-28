//  Debug UART prototypes.

#ifndef DBSERIAL_H_DEFINED

#ifdef SERIAL_DEBUG
int DBinit( void);
int DBgetchar( void);
int DBcharReady( void);
int DBputchar( char What);
void DBputs( char *What);
void DBprintf( char *Form,...);
char *DBgets( char *Buf, int Len);
char *DBhexin( unsigned int *RetVal, unsigned int *Digits, char *Buf);
#else
#define DBinit(...) 
#define DBgetchar(...) 
#define DBprintf(...) 
#define DBcharReady(...) 
#define DBputchar(x) 
#define DBputs(x)  
#define DBgets(x,y) 
#define DBhexin(x,y,z) 
#endif
#endif

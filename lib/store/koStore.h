#ifndef __koStore_impl_h__
#define __koStore_impl_h__

#include "koStore_idl.h"

#include <iostream>
#include <fstream>
#include <map>
#include <string>

class KoStore : public KOStore::Store_skel
{
public:  
  KoStore( const char* _filename, KOStore::Mode _mode );
  ~KoStore();

  virtual CORBA::Boolean open( const char* name, const char *_mime_type );
  virtual void close();
  virtual KOStore::Data* read( CORBA::ULong max );
  virtual CORBA::Boolean write( const KOStore::Data& _data );

  bool write( const char* _data, unsigned long _len );
  long read( char *_buffer, unsigned long _len );

  char* createFileName();  

  void list();
  
protected:
  KOStore::Mode m_mode;

  struct Entry
  {
    string mimetype;
    string name;
    unsigned int size;
    unsigned int flags;
    unsigned int pos;
    unsigned int data;
  };
  
  void writeHeader( const KoStore::Entry& _entry );
  unsigned long readHeader( KoStore::Entry& _entry );
  void putULong( unsigned long x );
  unsigned long getULong();
  
  map<string,Entry> m_map;

  ofstream m_out;
  ifstream m_in;  
  bool m_bIsOpen;

  Entry m_current;
  int m_readBytes;
  
  int m_id;
};

class ostorestreambuffer : public streambuf
{
protected:
  static const int m_bufferSize = 4096;   // Groesse des Datenm_buffers
  char m_buffer[m_bufferSize + 1];            // Datenm_buffer
  KoStore* m_pStore;
  KOStore::Store_var m_vStore;
  
public:
  /* Konstruktor
   *  - Datenm_buffer initialisieren
   *  - ein Zeichen kleiner,
   *    damit das m_bufferSize-te Zeichen overflow() ausloest
   */
  ostorestreambuffer( KoStore* _store ) : m_pStore( _store )
  {
    setp (m_buffer, m_buffer+(m_bufferSize-1));
  }
  ostorestreambuffer( KOStore::Store_ptr _store ) : m_pStore( 0L ), m_vStore( KOStore::Store::_duplicate( _store ) )
  {
    setp (m_buffer, m_buffer+(m_bufferSize-1));
  }

  /* Destruktor
   *  - Datenm_buffer leeren
   */
  virtual ~ostorestreambuffer()
  {
    sync();
  }

protected:
  /* Zeichen im buffer abspeichern
   */
  int emptybuffer();

  /* M_buffer voll
   *  - c und alle vorherigen Zeichen ausgeben
   */
  virtual int overflow (int c)
  {
    if (c != EOF)
    {
      // Zeichen noch in den M_buffer einfuegen
      *pptr() = c;
      pbump(1);
    }
    if ( emptybuffer() == EOF )
    {
      return EOF;
    }
    return c;
  }

  /* Daten mit Datentraeger abgleichen
   *  - buffer explizit leeren
   */
  virtual int sync ()
  {
    if (emptybuffer() == EOF)
    {
      // FEHLER
      return -1;
    }
    return 0;
  }
};

class ostorestream : public ostream
{
public:
  ostorestream( KoStore* _store ) : ostream( &m_buf ), m_buf( _store ) { }
  ostorestream( KOStore::Store_ptr _store ) : ostream( &m_buf ), m_buf( _store ) { }

protected:
  ostorestreambuffer m_buf;
};

/*******************************************
 *
 * Store Input Buffer
 *
 *******************************************/

class istorestreambuffer : public streambuf
{
protected:
  /* Datenpuffer:
   *  maximal 4 Zeichen Putback-Bereich plus
   *  maximal 8192 Zeichen normaler Lesepuffer
   */
  static const int pufferSize = 8192 + 4;     // Groesse des Datenpuffers
  char puffer[pufferSize];       // Datenpuffer
  KoStore* m_pStore;
  KOStore::Store_var m_vStore;
  
public:
  /* Konstruktor
   *  - Datenpuffer leer initialisieren
   *  - keine Putback-Reserve
   *  => underflow() forcieren
   */
  istorestreambuffer( KoStore* _store ) : m_pStore( _store )
  {
    cerr << "Pointer constructor" << endl;
    setg (puffer+4,     // Putback-Anfang
	  puffer+4,     // Leseposition
	  puffer+4);    // Puffer-Ende
  }
  istorestreambuffer( KOStore::Store_ptr _store ) : m_pStore( 0L )
  {
    m_vStore = KOStore::Store::_duplicate( _store );
    
    cerr << "Var constructor" << endl;
    setg (puffer+4,     // Putback-Anfang
	  puffer+4,     // Leseposition
	  puffer+4);    // Puffer-Ende
  }

protected:
  /* neue Zeichen in den Puffer einlesen
   */
  virtual int underflow ();
};

/*******************************************
 *
 * Store Input Stream
 *
 *******************************************/

class istorestream : public istream
{
public:
  istorestream( KoStore* _store ) : istream( &m_buf ), m_buf( _store ) { }
  istorestream( KOStore::Store_ptr _store ) : istream( &m_buf ), m_buf( _store ) { }

protected:
  istorestreambuffer m_buf;
};

#endif







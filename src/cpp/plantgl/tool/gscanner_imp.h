/* -*-c++-*-
 *  ----------------------------------------------------------------------------
 *
 *       PlantGL: The Plant Graphic Library
 *
 *       Copyright CIRAD/INRIA/INRA
 *
 *       File author(s): F. Boudon (frederic.boudon@cirad.fr) et al. 
 *
 *  ----------------------------------------------------------------------------
 *
 *   This software is governed by the CeCILL-C license under French law and
 *   abiding by the rules of distribution of free software.  You can  use, 
 *   modify and/ or redistribute the software under the terms of the CeCILL-C
 *   license as circulated by CEA, CNRS and INRIA at the following URL
 *   "http://www.cecill.info". 
 *
 *   As a counterpart to the access to the source code and  rights to copy,
 *   modify and redistribute granted by the license, users are provided only
 *   with a limited warranty  and the software's author,  the holder of the
 *   economic rights,  and the successive licensors  have only  limited
 *   liability. 
 *       
 *   In this respect, the user's attention is drawn to the risks associated
 *   with loading,  using,  modifying and/or developing or reproducing the
 *   software by the user in light of its specific status of free software,
 *   that may mean  that it is complicated to manipulate,  and  that  also
 *   therefore means  that it is reserved for developers  and  experienced
 *   professionals having in-depth computer knowledge. Users are therefore
 *   encouraged to load and test the software's suitability as regards their
 *   requirements in conditions enabling the security of their systems and/or 
 *   data to be ensured and,  more generally, to use and operate it in the 
 *   same conditions as regards security. 
 *
 *   The fact that you are presently reading this means that you have had
 *   knowledge of the CeCILL-C license and that you accept its terms.
 *
 *  ----------------------------------------------------------------------------
 */


/*! \file gscanner.h
\brief File for generic scanner.
*/

#ifndef _GSCANNER_HEADER_IMP_
#define _GSCANNER_HEADER_IMP_

#include <ctype.h>         // for isprint(), etc.
#include <assert.h>

//#include "util_types.h"
#include "dirnames.h"
#include "readline.h"

#include "gscanner.h"

GENERIC_LEXER::GENERIC_LEXER(std::istream* is ,
                             std::ostream* os ,
                             const char* filename , // NULL corresponds to cin/readline
                             const char* prompt ):
    yyFlexLexer(is,os), // if NULL,NULL uses cin and cout
    _uses_readline(is?false:true), // if is is NULL uses readline
    _prompt(prompt)
{

  // yylineno is taken care of by yyFlexLexer
  _columno = 1;

  _cwd = TOOLS(get_cwd());                             // gets the current work dir
  if (filename) {

    _current_file_name = std::string(filename);

    // changes the current cwd so that new includes and file references can
    // be done from the place of this file. The old cwd is lost in this case.
    std::string new_cwd = get_dirname(filename);     // gets the new working dir name
    if (new_cwd != "") {                       // defines the new cwd
      if (!chg_dir(new_cwd)){
        *_lo << "\tchg_dir() returned an error.";
        *_lo << std::endl;
      }
      else _cwd = new_cwd;
    }

  }

  if (os) _lo = os;
  else _lo = &std::cout;

#ifdef USE_READLINE
  // for the first stream, we need to force function YY_INPUT to fill in the
  // buffer with readline input.
  // This is done by flushing the current buffer of the lexer.
  if (_uses_readline) yy_flush_buffer(YY_CURRENT_BUFFER);
#endif
}
/*! Destructor
At the top level, the stream was allocated by the user.
it must be then deallocated by him (do not delete _li)
*/

GENERIC_LEXER::~GENERIC_LEXER() {
}

bool GENERIC_LEXER::pushStream(std::istream& is, const char* f){

    if (bstack.size() < MAX_INCLUDE_DEPTH) {

                bstack.push(StreamState(_current_file_name, _cwd, YY_CURRENT_BUFFER, yylineno, _columno, _li));

                _li = &is;

                if (f) _current_file_name = std::string(f);

                yy_switch_to_buffer(yy_create_buffer(&is, READLINE_INPUT_BUF_SIZE));

                // reset of the file attributes
                yylineno = 1;

                // Takes into account the location of the current file to set a new cwd
                // (current work directory)

                if (f) {
                        std::string new_cwd = get_dirname(f);     // gets the new working dir name
                        if (new_cwd != "") {                // defines the new cwd
                                if (!chg_dir(new_cwd)) *_lo << "\tchg_dir() returned an error." << std::endl;
                                else _cwd = new_cwd;
                        }
                }
                return true;
    }
    else return false;

} // reads from a new stream (returns false if MAX_INCLUDE_DEPTH reached)

bool GENERIC_LEXER::pushMacroStream(std::istream& is, const char* m){

    if (bstack.size() < MAX_INCLUDE_DEPTH) {

                bstack.push(StreamState(_current_file_name, _cwd, YY_CURRENT_BUFFER, yylineno, _columno, _li));

                _li = &is;

                if (m) {
                        std::string name = _current_file_name + " in macro " + std::string(m);
                        _current_file_name.reserve(name.size()+1);
                        _current_file_name = name.c_str();
                }

                yy_switch_to_buffer(yy_create_buffer(&is, READLINE_INPUT_BUF_SIZE));

                // reset of the file attributes
                yylineno = 1;
                return true;
    }
    else return false;

} // reads from a new stream (returns false if MAX_INCLUDE_DEPTH reached)


/// restore the old stream (returns false if the lstack is empty)
bool GENERIC_LEXER::popStream(){

    if (bstack.size() > 0) {

                yy_delete_buffer(YY_CURRENT_BUFFER);

                const StreamState& s = bstack.top();
                yylineno = s.lineno;
                _columno = s.columno;

                if (_li && _li != &std::cin) delete _li; // was allocated by the scanner on :include

                _li = s.input_stream;

                _current_file_name = bstack.top().file;

                _cwd = bstack.top().cwd;

                yy_switch_to_buffer(s.lexbuf);

                bstack.pop();

                return true;

    }
    else return false;

}  // restaure the old stream (returns false if the lstack is empty)

/// test if the stack is empty
bool GENERIC_LEXER::isEmpty() const {return bstack.size()==0;}

/// test if the stack is empty
int GENERIC_LEXER::includeDepth() const {return bstack.size();}

// int token() const {if ; else return 0;}
/// return the current line number.
int GENERIC_LEXER::lineno() const {return yylineno;}
/// return the current columns number .
int GENERIC_LEXER::columno() const {return _columno;};
/// return the current File name.
const char* GENERIC_LEXER::currentFile() {return _current_file_name.c_str();}

/// return whether lexer use readline or not.
bool GENERIC_LEXER::useReadline() const {
#ifdef USE_READLINE
        return _uses_readline;
#else
        return false;
#endif
}

/*! Function used by yyFlexLexer to fill its buffer when the scan of the previous
buffer has been completed.

  LexerInput reads up to max_size characters into buf and returns the
  number of characters read.  To indicate end-of-input, return 0
  characters

        This function is redefined on GLexer to allow buffering via readline.

          by default max_size is 8192
*/
int GENERIC_LEXER::LexerInput(char* buf, int max_size ) {
    // std::cerr << "maxsize = " << max_size << std::endl;

#ifdef USE_READLINE
    if (_uses_readline && bstack.size() == 0) {

                int ret =  readline_input(buf, _prompt);

                /*
                std::cerr << "readline ret = " << ret << std::endl;
                std::cerr << "chaine lue : [" << buf << "]" << std::endl;
                std::cerr << "caracteres :  " ;
                for (int i=0; i<ret; i++) std::cerr << "[" << (int)buf[i] << "] " ;
                */
                return ret;
    }
    else
#endif
                return yyFlexLexer::LexerInput(buf, max_size);

}


/// function called by the parser (void* should be YYSTYPE*)
int GENERIC_LEXER::yylex(void* pval) {
        val = pval;
        return yylex();
}

#ifdef LEXDEBUG

/// Debug function : print current character to output stream.
std::ostream& GENERIC_LEXER::printChar(char c, std::ostream& os) const {
    if (isprint(c))
                os << (char)c << std::flush;
    else if (iscntrl(c))
                os << "control-ascii#" << (int)(c) << std::flush;
    else os << "ascii#" << (int)(c) << std::flush;
    return os;
}

/// Debug function : print current token value to output stream.
void GENERIC_LEXER::Trace(const char* s) const {

    assert(s);

    size_t l = strlen(s);

    *_lo << "[";
    for(size_t i=0;i<strlen(s);i++)
                printChar(s[i], *_lo);
    *_lo << "]" << std::flush;
#ifdef PRINT_LINE_AND_COLUMN
    *_lo << "(l" << yylineno << ",c" << _columno << ") ";
#endif

        if (s[l-1]=='\n') *_lo << std::endl;

}

#endif

#endif

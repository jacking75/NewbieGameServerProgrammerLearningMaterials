// ----------------------------------------------------------------------------------------------
// Copyright (c) Marius Bancila 2013
//               http://mariusbancila.ro
// ----------------------------------------------------------------------------------------------
// License: Creative Commons Attribution-ShareAlike 3.0 Unported 
//          http://creativecommons.org/licenses/by-sa/3.0/
// ----------------------------------------------------------------------------------------------
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ----------------------------------------------------------------------------------------------
// You must not remove this notice, or any other, from this software.
// ----------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>

#include <iostream>
#include <string>

namespace conmanip
{
   enum class console_text_colors : WORD
   {
      black          = 0,
      white          = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
      blue           = FOREGROUND_BLUE,
      green          = FOREGROUND_GREEN,
      red            = FOREGROUND_RED,
      yellow         = FOREGROUND_RED | FOREGROUND_GREEN,
      magenta        = FOREGROUND_RED | FOREGROUND_BLUE,
      cyan           = FOREGROUND_GREEN | FOREGROUND_BLUE,
      light_white    = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY,
      light_blue     = FOREGROUND_BLUE | FOREGROUND_INTENSITY,
      light_green    = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
      light_red      = FOREGROUND_RED | FOREGROUND_INTENSITY,
      light_yellow   = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
      light_magenta  = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,   
      light_cyan     = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
   };

   enum class console_bg_colors : WORD
   {
      black          = 0,
      white          = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED,
      blue           = BACKGROUND_BLUE,
      green          = BACKGROUND_GREEN,
      red            = BACKGROUND_RED,
      yellow         = BACKGROUND_RED | BACKGROUND_GREEN,
      magenta        = BACKGROUND_RED | BACKGROUND_BLUE,
      cyan           = BACKGROUND_GREEN | BACKGROUND_BLUE,
      light_white    = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY,
      light_blue     = BACKGROUND_BLUE | BACKGROUND_INTENSITY,
      light_green    = BACKGROUND_GREEN | BACKGROUND_INTENSITY,
      light_red      = BACKGROUND_RED | BACKGROUND_INTENSITY,
      light_yellow   = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY,
      light_magenta  = BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY,
      light_cyan     = BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY,
   };

   enum class console_type : DWORD
   {
      in  = STD_INPUT_HANDLE,
      out = STD_OUTPUT_HANDLE,
      err = STD_ERROR_HANDLE,
   };

   enum class console_modes
   {
      echo                       = 1,
      overwrite                  = 2,
      hide_ctrl_c                = 4,
      enable_mouse_selection     = 8,
   };

   console_modes operator&(console_modes const a, console_modes const b)
   {
      return static_cast<console_modes>(
         static_cast<int>(a) & static_cast<int>(b));
   }

   enum class console_cleanup_options : int
   {
      none                       = 0,
      restore_pos                = 1,
      restore_attibutes          = 2,
      restore_mode               = 4,
      restore_buffsize           = 8,

      restore_all_nopos          = restore_attibutes | restore_mode | restore_buffsize,
      restore_all                = restore_all_nopos | restore_pos
   };

   console_cleanup_options operator&(console_cleanup_options const a, console_cleanup_options const b)
   {
      return static_cast<console_cleanup_options>(
         static_cast<int>(a) & static_cast<int>(b));
   }

   struct console_colors
   {
      console_text_colors text;
      console_bg_colors   background;

      console_colors(console_text_colors const ctext, console_bg_colors const cback):
         text(ctext), background(cback)
      {
      }
   };

   // functions namespace _details are not intended for direct use from client code
   namespace _details
   {
      void _settextcolor(HANDLE const console, console_text_colors const color)
      {
         CONSOLE_SCREEN_BUFFER_INFO info;
         ::GetConsoleScreenBufferInfo(console, &info);

         WORD attr = info.wAttributes & 0xFFF0;
         attr |= static_cast<WORD>(color);

         ::SetConsoleTextAttribute(console, attr);
      }

      void _setbgcolor(HANDLE const console, console_bg_colors const color)
      {
         CONSOLE_SCREEN_BUFFER_INFO info;
         ::GetConsoleScreenBufferInfo(console, &info);

         WORD attr = info.wAttributes & 0xFF0F;
         attr |= (WORD)color;

         ::SetConsoleTextAttribute(console, attr);
      }

      void _setcolors(HANDLE const console, console_colors const color)
      {
         CONSOLE_SCREEN_BUFFER_INFO info;
         ::GetConsoleScreenBufferInfo(console, &info);

         WORD attr = info.wAttributes & 0xFF00;
         attr |= (WORD)color.background;
         attr |= (WORD)color.text;

         ::SetConsoleTextAttribute(console, attr);
      }

      void _setmode(HANDLE const console, console_modes const modes)
      {
         DWORD mode = 0;
         ::GetConsoleMode(console, &mode);

         if(console_modes::echo == (modes & console_modes::echo))
         {
            mode |= (ENABLE_ECHO_INPUT | ENABLE_INSERT_MODE);
         }

         if(console_modes::overwrite == (modes & console_modes::overwrite))
         {
            mode &= ~ENABLE_INSERT_MODE;
            mode |= ENABLE_EXTENDED_FLAGS;
         }

         if(console_modes::hide_ctrl_c == (modes & console_modes::hide_ctrl_c))
         {
            mode |= ENABLE_PROCESSED_INPUT;
         }

         if(console_modes::enable_mouse_selection == (modes & console_modes::enable_mouse_selection))
         {
            mode |= (ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
         }

         ::SetConsoleMode(console, mode);
      }

      void _clearmode(HANDLE const console, console_modes const modes)
      {
         DWORD mode = 0;
         ::GetConsoleMode(console, &mode);

         if(console_modes::echo == (modes & console_modes::echo))
         {
            mode &= ~(ENABLE_ECHO_INPUT | ENABLE_INSERT_MODE);
         }

         if(console_modes::overwrite == (modes & console_modes::overwrite))
         {
            mode |= (ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS);
         }

         if(console_modes::hide_ctrl_c == (modes & console_modes::hide_ctrl_c))
         {
            mode &= ~ENABLE_PROCESSED_INPUT;
         }

         if(console_modes::enable_mouse_selection == (modes & console_modes::enable_mouse_selection))
         {
            mode &= ~ENABLE_QUICK_EDIT_MODE;
         }

         ::SetConsoleMode(console, mode);
      }

      int _getposx(HANDLE const console)
      {
         CONSOLE_SCREEN_BUFFER_INFO info;
         ::GetConsoleScreenBufferInfo(console, &info);
         return info.dwCursorPosition.X;
      }

      void _setposx(HANDLE const console, int const x)
      {
         CONSOLE_SCREEN_BUFFER_INFO info;
         ::GetConsoleScreenBufferInfo(console, &info);
         info.dwCursorPosition.X = x;
         ::SetConsoleCursorPosition(console, info.dwCursorPosition);         
      }

      int _getposy(HANDLE const console)
      {
         CONSOLE_SCREEN_BUFFER_INFO info;
         ::GetConsoleScreenBufferInfo(console, &info);
         return info.dwCursorPosition.Y;
      }

      void _setposy(HANDLE const console, int const y)
      {
         CONSOLE_SCREEN_BUFFER_INFO info;
         ::GetConsoleScreenBufferInfo(console, &info);
         info.dwCursorPosition.Y = y;
         ::SetConsoleCursorPosition(console, info.dwCursorPosition);
      }

      COORD _getpos(HANDLE const console)
      {
         CONSOLE_SCREEN_BUFFER_INFO info;
         ::GetConsoleScreenBufferInfo(console, &info);
         return info.dwCursorPosition;
      }

      void _setpos(HANDLE const console, COORD const pos)
      {
         ::SetConsoleCursorPosition(console, pos);
      }
   }

   template <console_type StdHandle>
   class console_context
   {
   public:
      HANDLE                     handle;
      CONSOLE_SCREEN_BUFFER_INFO scrbuf_info;
      console_cleanup_options    cleanup;
      DWORD                      mode;

   public:
      console_context(console_cleanup_options const cleanup = console_cleanup_options::restore_all_nopos):
         cleanup(cleanup)
      {
         handle = ::GetStdHandle((DWORD)StdHandle); 

         ::GetConsoleScreenBufferInfo(handle, &scrbuf_info);
         ::GetConsoleMode(handle, &mode);
      }

      ~console_context()
      {
         restore();
      }

      void restore()
      {
         restore(cleanup);
      }

      void restore(console_cleanup_options const options)
      {
         if(console_cleanup_options::restore_pos == (options & console_cleanup_options::restore_pos))
         {
            ::SetConsoleCursorPosition(handle, scrbuf_info.dwCursorPosition);
         }

         if(console_cleanup_options::restore_attibutes == (options & console_cleanup_options::restore_attibutes))
         {
            ::SetConsoleTextAttribute(handle, scrbuf_info.wAttributes);
         }

         if(console_cleanup_options::restore_mode == (options & console_cleanup_options::restore_mode))
         {
            ::SetConsoleMode(handle, mode);
         }

         if(console_cleanup_options::restore_buffsize == (options & console_cleanup_options::restore_buffsize))
         {
            ::SetConsoleScreenBufferSize(handle, scrbuf_info.dwSize);
         }         
      }
   };

   template <console_type StdHandle>
   class console 
   {
      console_context<StdHandle> context;

   public:
      console(console_context<StdHandle> const & context = console_context<StdHandle>()):
         context(context)
      {
      }

      void setmode(console_modes const modes)
      {
         _details::_setmode(context.handle, modes);
      }

      void clearmode(console_modes const modes)
      {
         _details::_clearmode(context.handle, modes);
      }

      int getposx()
      {
         return _details::_getposx(context.handle);
      }

      void setposx(int const x)
      {
         _details::_setposx(context.handle, x);
      }

      int getposy()
      {
         return _details::_getposy(context.handle);
      }

      void setposy(int const y)
      {
         _details::_setposy(context.handle, y);
      }

      COORD getpos()
      {
         return _details::_getpos(context.handle);
      }

      void setpos(int const x, int const y)
      {
         COORD coord = {x,y};
         _details::_setpos(context.handle, coord);
      }

      void settextcolor(console_text_colors const & color)
      {
         _details::_settextcolor(context.handle, color);
      }

      void setbgcolor(console_bg_colors const & color)
      {
         _details::_setbgcolor(context.handle, color);
      }

      void setcolors(console_text_colors const & text, console_bg_colors const & background)
      {
         _details::_setcolors(context.handle, console_colors(text, background));
      }

      void resetcolors()
      {
         ::SetConsoleTextAttribute(context.handle, context.scrbuf_info.wAttributes);
      }

      COORD getsize()
      {
         CONSOLE_SCREEN_BUFFER_INFO csbi;
         ::GetConsoleScreenBufferInfo(context.handle, &csbi);
         return csbi.dwSize;
      }

      bool setsize(int const x, int const y)
      {
         COORD size = {x, y};
         return 0 != ::SetConsoleScreenBufferSize(context.handle, size);
      }

      bool settitle(std::string const & title)
      {
         return ::SetConsoleTitleA(title.data()) != 0;
      }

      bool settitle(std::wstring const & title)
      {
         return ::SetConsoleTitleW(title.data()) != 0;
      }

      std::string gettitle()
      {
         char oldtitle[MAX_PATH] = {0};
         ::GetConsoleTitleA(oldtitle, MAX_PATH);
         return std::string(oldtitle);
      }

      std::wstring gettitlew()
      {
         wchar_t oldtitle[MAX_PATH] = {0};
         ::GetConsoleTitleW(oldtitle, MAX_PATH);
         return std::wstring(oldtitle);
      }
   };

   typedef console<console_type::in>               console_in;
   typedef console<console_type::out>              console_out;
   typedef console<console_type::err>              console_err;

   typedef console_context<console_type::in>       console_in_context;
   typedef console_context<console_type::out>      console_out_context;
   typedef console_context<console_type::err>      console_err_context;

   template<class T>
   struct console_manipulator
   {	
      console_manipulator(void (*fun)(HANDLE const, T const), T arg, HANDLE handle = nullptr, bool default_console = true)
         : Pfun(fun), arg(arg), handle(handle), defcon(default_console)
      {
      }

      void (*Pfun)(HANDLE const, T const);
      T arg;
      HANDLE handle;
      bool defcon;
   };

   template<class _Elem, class _Traits, class _Arg> 
   inline std::basic_istream<_Elem, _Traits>& operator>>(
      std::basic_istream<_Elem, _Traits>& stream, 
      const console_manipulator<_Arg>& manip)
   {	
      (*manip.Pfun)(manip.handle == nullptr ? ::GetStdHandle(manip.defcon ? STD_OUTPUT_HANDLE : STD_INPUT_HANDLE) : manip.handle, manip.arg);
      return stream;
   }

   template<class _Elem, class _Traits, class _Arg> 
   inline std::basic_ostream<_Elem, _Traits>& operator<<(
      std::basic_ostream<_Elem, _Traits>& stream, 
      const console_manipulator<_Arg>& manip)
   {
      (*manip.Pfun)(manip.handle == nullptr ? ::GetStdHandle(STD_OUTPUT_HANDLE) : manip.handle, manip.arg);
      return stream;
   }

   console_manipulator<console_text_colors> settextcolor(console_text_colors const color)
   {	
      return console_manipulator<console_text_colors>(&_details::_settextcolor, color);
   }

   template <console_type StdHandle>
   console_manipulator<console_text_colors> settextcolor(console_text_colors const color, console_context<StdHandle> const context)
   {	
      return console_manipulator<console_text_colors>(&_details::_settextcolor, color, context.handle);
   }

   template <console_type StdHandle>
   console_manipulator<console_text_colors> restoretextcolor(console_context<StdHandle> const context)
   {	
      return console_manipulator<console_text_colors>(
         &_details::_settextcolor, 
         static_cast<console_text_colors>(context.scrbuf_info.wAttributes & 0x0FFF), 
         context.handle);
   }

   console_manipulator<console_bg_colors> setbgcolor(console_bg_colors const color)
   {	
      return console_manipulator<console_bg_colors>(&_details::_setbgcolor, color);
   }

   template <console_type StdHandle>
   console_manipulator<console_bg_colors> setbgcolor(console_bg_colors const color, console_context<StdHandle> const context)
   {	
      return console_manipulator<console_bg_colors>(&_details::_setbgcolor, color, context.handle);
   }

   template <console_type StdHandle>
   console_manipulator<console_text_colors> restorebgcolor(console_context<StdHandle> const context)
   {	
      return console_manipulator<console_text_colors>(
         &_details::_settextcolor, 
         static_cast<console_text_colors>(context.scrbuf_info.wAttributes & 0x00F0), 
         context.handle);
   }

   console_manipulator<console_colors> setcolors(console_text_colors const text, console_bg_colors const background)
   {	
      return console_manipulator<console_colors>(&_details::_setcolors, console_colors(text, background));
   }

   template <console_type StdHandle>
   console_manipulator<console_colors> setcolors(console_text_colors const text, console_bg_colors const background, console_context<StdHandle> const context)
   {	
      return console_manipulator<console_colors>(&_details::_setcolors, console_colors(text, background), context.handle);
   }

   console_manipulator<console_modes> setmode(console_modes const modes)
   {	
      return console_manipulator<console_modes>(&_details::_setmode, modes, nullptr, false);
   }

   template <console_type StdHandle>
   console_manipulator<console_modes> setmode(console_modes const modes, console_context<StdHandle> const context)
   {	
      return console_manipulator<console_modes>(&_details::_setmode, modes, context.handle, false);
   }

   console_manipulator<console_modes> clearmode(console_modes const modes)
   {	
      return console_manipulator<console_modes>(&_details::_clearmode, modes, nullptr, false);
   }

   template <console_type StdHandle>
   console_manipulator<console_modes> clearmode(console_modes const modes, console_context<StdHandle> const context)
   {	
      return console_manipulator<console_modes>(&_details::_clearmode, modes, context.handle, false);
   }

   console_manipulator<int> setposx(int const x)
   {	
      return console_manipulator<int>(&_details::_setposx, x);
   }

   template <console_type StdHandle>
   console_manipulator<int> setposx(int const x, console_context<StdHandle> const context)
   {	
      return console_manipulator<int>(&_details::_setposx, x, context.handle);
   }

   console_manipulator<int> setposy(int const y)
   {	
      return console_manipulator<int>(&_details::_setposy, y);
   }

   template <console_type StdHandle>
   console_manipulator<int> setposy(int const y, console_context<StdHandle> const context)
   {	
      return console_manipulator<int>(&_details::_setposy, y, context.handle);
   }

   console_manipulator<COORD> setpos(int const x, int const y)
   {	
      COORD c = {(short)x, (short)y};
      return console_manipulator<COORD>(&_details::_setpos, c);
   }

   template <console_type StdHandle>
   console_manipulator<COORD> setpos(int const x, int const y, console_context<StdHandle> const context)
   {	
      COORD c = {x, y};
      return console_manipulator<COORD>(&_details::_setpos, c, context.handle);
   }
}

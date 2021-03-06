/*
MD_Parola - Library for modular scrolling text and Effects

See header file for comments

Copyright (C) 2013 Marco Colli. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "MD_Parola.h"
#include "MD_Parola_lib.h"
/**
 * \file
 * \brief Implements opening effect
 */

void MD_PZone::effectOpen(bool bLightBar, bool bIn)
// Open the current message in/out
{
  if (bIn)
  {
    switch (_fsmState)
    {
    case INITIALISE:
    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
      PRINT_STATE("I OPEN");
      FSMPRINT(" - limits R:", _limitRight);
      FSMPRINT(" L:", _limitLeft);
      _nextPos = 1 + (_limitLeft - _limitRight) / 2;
      FSMPRINT(" O:", _nextPos);
      if (bLightBar)
      {
        _MX->setColumn(_limitLeft - _nextPos, LIGHT_BAR);
        _MX->setColumn(_limitRight + _nextPos, LIGHT_BAR);
      }
      _fsmState = PUT_CHAR;
      break;

    case PUT_CHAR:
      PRINT_STATE("I OPEN");
      FSMPRINT(" - offset ", _nextPos);
      if (_nextPos < 0)
      {
        _fsmState = PAUSE;
      }
      else
      {
        commonPrint();
        for (int16_t i = 0; i < _nextPos; i++)
        {
          _MX->setColumn(_limitRight + i, EMPTY_BAR);
          _MX->setColumn(_limitLeft - i, EMPTY_BAR);
        }

        _nextPos--;
        if (bLightBar && (_nextPos >= 0))
        {
          _MX->setColumn(_limitRight + _nextPos, LIGHT_BAR);
          _MX->setColumn(_limitLeft - _nextPos, LIGHT_BAR);
        }
      }
      break;

    default:
      PRINT_STATE("I OPEN");
      _fsmState = PAUSE;
    }
  }
  else  // exiting
  {
    switch (_fsmState)
    {
    case PAUSE:
    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
      PRINT_STATE("O OPEN");
      zoneClear();
      commonPrint();
      _nextPos = 0;
      if (bLightBar)
      {
        _MX->setColumn(_limitLeft, LIGHT_BAR);
        _MX->setColumn(_limitRight,LIGHT_BAR);
      }
      _fsmState = PUT_CHAR;
      // fall through

    case PUT_CHAR:
      PRINT_STATE("O OPEN");
      FSMPRINT(" - offset ", _nextPos);
      if (_nextPos > (_limitLeft - _limitRight) / 2)
      {
        _fsmState = END;
      }
      else
      {
        _MX->setColumn(_limitLeft - _nextPos, EMPTY_BAR);
        _MX->setColumn(_limitRight + _nextPos, EMPTY_BAR);
        _nextPos++;
        if (bLightBar && (_nextPos <= (_limitLeft - _limitRight) / 2))
        {
          _MX->setColumn(_limitLeft - _nextPos, LIGHT_BAR);
          _MX->setColumn(_limitRight + _nextPos,LIGHT_BAR);
        }
      }
      break;

    default:
      PRINT_STATE("O OPEN");
      _fsmState = END;
    }
  }
}

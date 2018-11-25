/**
 * Copyright (C) 2018 Michael Reeves
 * 
 * This file is part of KDiff3.
 * 
 * KDiff3 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * KDiff3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with KDiff3.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "fileaccess.h"

class DirectoryInfo
{
    public:
      DirectoryInfo(
              FileAccess& dirA,
              FileAccess& dirB,
              FileAccess& dirC,
              FileAccess& dirDest)
      {
          m_dirA = dirA;
          m_dirB = dirB;
          m_dirC = dirC;
          m_dirDest = dirDest;
      }

      FileAccess dirA(void) const { return m_dirA; }
      FileAccess dirB(void) const { return m_dirB; }
      FileAccess dirC(void) const { return m_dirC; }
      FileAccess destDir(void) const { return m_dirDest; }
    
    private:
      FileAccess m_dirA, m_dirB, m_dirC;
      FileAccess m_dirDest;
};
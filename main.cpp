/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Original software by Scot Lawson, KI4LKF
// Copyright (C) 2018 by Thomas A. Early, N7TAE

#include "xreflector.h"

int main(int argc, char **argv)
{
	CXReflector xref;
	if (xref.Initialize(argc, argv)) {
		xref.Stop();
		return 1;
	}
	xref.Run();
	xref.Stop();
	return 0;
}

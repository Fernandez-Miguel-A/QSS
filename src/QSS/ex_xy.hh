#ifndef QSS_ex_xy_hh_INCLUDED
#define QSS_ex_xy_hh_INCLUDED

// Simple xy Example Setup
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (http://objexx.com)
// under contract to the National Renewable Energy Laboratory
// of the U.S. Department of Energy

// C++ Headers
#include <vector>

// Forward
class Variable;

namespace ex {

using Variables = std::vector< Variable * >;

// Simple xy Example Setup
void
xy( Variables & vars );

} // ex

#endif

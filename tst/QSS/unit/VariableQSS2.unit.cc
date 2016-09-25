// QSS::VariableQSS2 Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// QSS Headers
#include <QSS/VariableQSS2.hh>

// C++ Headers
#include <algorithm>
#include <cmath>

TEST( VariableQSS2Test, Basic )
{
	VariableQSS2 x1( "x1" );
	x1.init_val( 2.5 );
	x1.add_der( 12.0 ).add_der( 2.0, x1 ).finalize_der();
	x1.init_der();
	x1.init_der2();
	x1.init_event();
	EXPECT_EQ( 1.0e-6, x1.aTol );
	EXPECT_EQ( 1.0e-6, x1.rTol );
	EXPECT_DOUBLE_EQ( 2.5 + 17.0e-6, x1.q( 1.0e-6 ) );
	EXPECT_EQ( 17.0, x1.q1( 1.0e-6 ) );
	EXPECT_DOUBLE_EQ( 2.5 + 17.0e-6 + 17.0e-12, x1.x( 1.0e-6 ) );
	EXPECT_EQ( 0.0, x1.tBeg );
	EXPECT_DOUBLE_EQ( std::sqrt( std::max( x1.aTol, x1.rTol * 2.5 ) / 17.0 ), x1.tEnd );
	double const x1_tEnd( x1.tEnd );
	x1.advance();
	EXPECT_EQ( x1_tEnd, x1.tBeg );

	VariableQSS2 x2( "x2", 1.0e-3, 1.0e-4 );
	x2.init_val( 2.5 );
	x2.add_der( 12.0 ).add_der( 2.0, x2 ).finalize_der();
	x2.init_der();
	x2.init_der2();
	x2.init_event();
	EXPECT_EQ( 1.0e-3, x2.aTol );
	EXPECT_EQ( 1.0e-4, x2.rTol );
	EXPECT_EQ( 0.0, x2.tBeg );
	EXPECT_DOUBLE_EQ( std::sqrt( std::max( x2.aTol, x2.rTol * 2.5 ) / 17.0 ), x2.tEnd );

	EXPECT_EQ( 2U, events.size() );
	events.clear();
}

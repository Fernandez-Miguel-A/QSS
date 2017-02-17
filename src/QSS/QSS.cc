// QSS Solver Prototype Runner

// C++ Headers
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

// QSS Headers
#include <QSS/FunctionLTI.hh>
#include <QSS/FunctionLTI_LIQSS.hh>
#include <QSS/FunctionLTI_ND.hh>
#include <QSS/Function_achilles1.hh>
#include <QSS/Function_achilles2.hh>
#include <QSS/Function_NonlinearEx1.hh>
#include <QSS/Function_NonlinearEx1_ND.hh>
#include <QSS/Function_sin.hh>
#include <QSS/Function_sin_ND.hh>
#include <QSS/globals.hh>
#include <QSS/math.hh>
#include <QSS/VariableInp1.hh>
#include <QSS/VariableInp2.hh>
#include <QSS/VariableInp3.hh>
#include <QSS/VariableLIQSS1.hh>
#include <QSS/VariableLIQSS2.hh>
#include <QSS/VariableQSS1.hh>
#include <QSS/VariableQSS2.hh>
#include <QSS/VariableQSS3.hh>

int
main()
{
	std::cout << std::setprecision( 16 );
	std::cerr << std::setprecision( 16 );

	Variable::Variables vars; // Variables collection
	std::vector< std::ofstream > q_streams; // Quantized output streams
	std::vector< std::ofstream > x_streams; // Continuous output streams

	using size_type = Variable::Variables::size_type;
	using Time = double;

	// Controls
	diag = false; // Enable for diagnostic output of requantization events?
	inflection_steps = true; // Add requantization steps at inflection points?
	bool const sampled( true ); // Sampled outputs?
	bool const all_vars_out( false ); // Output all variables at every requantization event?
	bool const q_out( sampled || all_vars_out ); // Quantized output would differ from continuous?
	int const QSS_order_max( 3 ); // Handle all QSS orders

//	// Simple x, y, z
//	Time const dto( 1.0e-3 ); // Sampling time step
//	Time const tE( 5.0 ); // Simulation end time
//	Time t( 0.0 ); // Simulation time
//	Time to( t + dto ); // Sampling time
//	VariableQSS2< FunctionLTI > x( "x", 1.0, 0.0 );
//	VariableQSS2< FunctionLTI > y( "y", 1.0, 0.0 );
//	VariableQSS2< FunctionLTI > z( "z", 1.0, 0.0 );
//	x.init0( 0.0 );
//	y.init0( 0.0 );
//	z.init0( 0.0 );
//	x.d().add( 1.0 );
//	y.d().add( 1.0, x );
//	z.d().add( 1.0, y );
//	vars.reserve( 3 );
//	vars.push_back( &x );
//	vars.push_back( &y );
//	vars.push_back( &z );

//	// Simple x1, x2 model
//	// tmax   = 3.5
//	// x0     = [1, 0]
//	// A      = [[0, -1], [+1, 0]]
//	// Q      = 1
//	// order  = 1
//	Time const dto( 1.0e-3 ); // Sampling time step
//	Time const tE( 10.0 ); // Simulation end time
//	Time t( 0.0 ); // Simulation time
//	Time to( t + dto ); // Sampling time
//	VariableQSS2< FunctionLTI > x1( "x1", 1.0, 0.0 );
//	VariableQSS2< FunctionLTI > x2( "x2", 1.0, 0.0 );
//	x1.init0( 1.0 );
//	x2.init0( 0.0 );
//	x1.d().add( -1.0, x2 );
//	x2.d().add( 1.0, x1 );
//	vars.reserve( 2 );
//	vars.push_back( &x1 );
//	vars.push_back( &x2 );

//	// Achilles and the Tortoise
//	Time const dto( 1.0e-3 ); // Sampling time step
//	Time const tE( 10.0 ); // Simulation end time
//	Time t( 0.0 ); // Simulation time
//	Time to( t + dto ); // Sampling time
//	VariableQSS2< FunctionLTI > x1( "x1", 1.0, 0.0 );
//	VariableQSS2< FunctionLTI > x2( "x2", 1.0, 0.0 );
////	VariableQSS2< FunctionLTI > x1( "x1", 1.0e-4, 0.0 );
////	VariableQSS2< FunctionLTI > x2( "x2", 1.0e-4, 0.0 );
////	VariableQSS3< FunctionLTI > x1( "x1", 1.0e-4, 0.0 );
////	VariableQSS3< FunctionLTI > x2( "x2", 1.0e-4, 0.0 );
////	VariableLIQSS2< FunctionLTI_LIQSS > x1( "x1", 1.0e-4, 0.0 );
////	VariableLIQSS2< FunctionLTI_LIQSS > x2( "x2", 1.0e-4, 0.0 );
//	x1.init0( 0.0 );
//	x2.init0( 2.0 );
//	x1.d().add( -0.5, x1 ).add( 1.5, x2 );
//	x2.d().add( -1.0, x1 );
//	vars.reserve( 2 );
//	vars.push_back( &x1 );
//	vars.push_back( &x2 );

//	// Achilles and the Tortoise: Symmetric for Simultaneous Triggering
//	Time const dto( 1.0e-3 ); // Sampling time step
//	Time const tE( 10.0 ); // Simulation end time
//	Time t( 0.0 ); // Simulation time
//	Time to( t + dto ); // Sampling time
//	VariableQSS2< FunctionLTI > x1( "x1", 1.0e-4, 0.0 );
//	VariableQSS2< FunctionLTI > x2( "x2", 1.0e-4, 0.0 );
////	VariableLIQSS2< FunctionLTI_LIQSS > x1( "x1", 1.0e-4, 0.0 );
////	VariableLIQSS2< FunctionLTI_LIQSS > x2( "x2", 1.0e-4, 0.0 );
//	x1.init0( 0.0 );
//	x2.init0( 2.0 );
//	x1.d().add( -0.5, x1 ).add( 1.5, x2 );
//	x2.d().add( -1.0, x1 );
//	VariableQSS2< FunctionLTI > y1( "y1", 1.0e-4, 0.0 );
//	VariableQSS2< FunctionLTI > y2( "y2", 1.0e-4, 0.0 );
////	VariableLIQSS2< FunctionLTI_LIQSS > y1( "y1", 1.0e-4, 0.0 );
////	VariableLIQSS2< FunctionLTI_LIQSS > y2( "y2", 1.0e-4, 0.0 );
//	y1.init0( 0.0 );
//	y2.init0( 2.0 );
//	y1.d().add( -0.5, y1 ).add( 1.5, y2 );
//	y2.d().add( -1.0, y1 );
//	vars.reserve( 4 );
//	vars.push_back( &x1 );
//	vars.push_back( &x2 );
//	vars.push_back( &y1 );
//	vars.push_back( &y2 );

//	// Achilles and the Tortoise: Using Custom Functions
//	Time const dto( 1.0e-3 ); // Sampling time step
//	Time const tE( 10.0 ); // Simulation end time
//	Time t( 0.0 ); // Simulation time
//	Time to( t + dto ); // Sampling time
//	VariableQSS2< Function_achilles1 > x1( "x1", 1.0, 0.0 );
//	VariableQSS2< Function_achilles2 > x2( "x2", 1.0, 0.0 );
//	x1.init0( 0.0 );
//	x2.init0( 2.0 );
//	x1.d().var( x1, x2 );
//	x2.d().var( x1 );
//	vars.reserve( 2 );
//	vars.push_back( &x1 );
//	vars.push_back( &x2 );

//	// Nonlinear Example 1
//	Time const dto( 1.0e-3 ); // Sampling time step
//	Time const tE( 5.0 ); // Simulation end time
//	Time t( 0.0 ); // Simulation time
//	Time to( t + dto ); // Sampling time
//	VariableQSS2< Function_NonlinearEx1 > y( "y", 1.0e-3, 1.0e-3 );
////	VariableQSS2< Function_NonlinearEx1_ND > y( "y", 1.0e-3, 1.0e-3 );
////	VariableLIQSS2< Function_NonlinearEx1 > y( "y", 1.0e-3, 1.0e-3 );
////	VariableLIQSS2< Function_NonlinearEx1_ND > y( "y", 1.0e-3, 1.0e-3 );
//	y.init0( 2.0 );
//	y.d().var( y );
//	vars.reserve( 1 );
//	vars.push_back( &y );
//	std::ofstream e_stream( "y_e.out" ); // Exact solution output
//	while ( to <= tE * ( 1.0 + 1.0e-14 ) ) {
//		e_stream << to << '\t' << y.d().e( to ) << '\n';
//		to += dto;
//	}
//	e_stream.close();
//	to = 0.0;

	// Exponential Decay Example
	Time const dto( 1.0e-3 ); // Sampling time step
	Time const tE( 10.0 ); // Simulation end time
	Time t( 0.0 ); // Simulation time
	Time to( t + dto ); // Sampling time
	VariableQSS2< FunctionLTI > x1( "x1", 1.0e-4, 0.0 );
	x1.init0( 1.0 );
	x1.d().add( -1.0, x1 );
	vars.reserve( 1 );
	vars.push_back( &x1 );

//	// Exponential Decay with Sine Input Example
//	Time const dto( 1.0e-3 ); // Sampling time step
//	Time const tE( 30.0 ); // Simulation end time
//	Time t( 0.0 ); // Simulation time
//	Time to( t + dto ); // Sampling time
//	VariableQSS2< FunctionLTI > x1( "x1", 1.0e-4, 0.0 );
//	VariableInp2< Function_sin > u1( "u1", 1.0e-4, 0.0 );
////	VariableInp2< Function_sin_ND > u1( "u1", 1.0e-4, 0.0 );
//	u1.set_dt_max( 0.1 );
//	u1.f().c( 0.05 ).s( 0.5 );
//	x1.init0( 1.0 );
//	u1.init0();
//	x1.d().add( -1.0, x1 ).add( u1 );
//	vars.reserve( 2 );
//	vars.push_back( &x1 );
//	vars.push_back( &u1 );

//	// Stiff System Literature Example
////	inflection_steps = true; // Improves results for large tolerances
//	Time const dto( 1.0e-3 ); // Sampling time step
//	Time const tE( 600.0 ); // Simulation end time
//	Time t( 0.0 ); // Simulation time
//	Time to( t + dto ); // Sampling time
////	VariableQSS2< FunctionLTI > x1( "x1", 1.0e-4, 0.0 );
////	VariableQSS2< FunctionLTI > x2( "x2", 1.0e-4, 0.0 );
//	VariableLIQSS2< FunctionLTI_LIQSS > x1( "x1", 1.0e-4, 0.0 );
//	VariableLIQSS2< FunctionLTI_LIQSS > x2( "x2", 1.0e-4, 0.0 );
//	x1.init0( 0.0 );
//	x2.init0( 20.0 );
//	x1.d().add( 0.01, x2 );
//	x2.d().add( 2020.0 ).add( -100.0, x1 ).add( -100.0, x2 );
//	vars.reserve( 2 );
//	vars.push_back( &x1 );
//	vars.push_back( &x2 );

	// Solver master logic
	for ( auto var : vars ) {
		var->init1_LIQSS();
	}
	for ( auto var : vars ) {
		var->init1();
	}
	if ( QSS_order_max >= 2 ) {
		for ( auto var : vars ) {
			var->init2_LIQSS();
		}
		for ( auto var : vars ) {
			var->init2();
		}
		if ( QSS_order_max >= 3 ) {
			for ( auto var : vars ) {
				var->init3();
			}
		}
	}
	for ( auto var : vars ) {
		var->init_event();
	}
	for ( auto var : vars ) {
		if ( q_out ) {
			q_streams.push_back( std::ofstream( var->name + "_q.out" ) );
			q_streams.back() << std::setprecision( 16 ) << t << '\t' << var->q( t ) << '\n';
		}
		x_streams.push_back( std::ofstream( var->name + "_x.out" ) );
		x_streams.back() << std::setprecision( 16 ) << t << '\t' << var->x( t ) << '\n';
	}
	size_type n_vars( vars.size() );
	while ( ( t <= tE ) || ( sampled && ( to <= tE ) ) ) {
		t = events.top_time();
		if ( sampled ) { // Sampled outputs
			Time const tStop( std::min( t, tE ) );
			while ( to < tStop ) {
				for ( size_type i = 0; i < n_vars; ++i ) {
					if ( q_out ) q_streams[ i ] << to << '\t' << vars[ i ]->q( to ) << '\n';
					x_streams[ i ] << to << '\t' << vars[ i ]->x( to ) << '\n';
				}
				to += dto;
			}
		}
		if ( t > tE ) break; // Don't requantize
		if ( events.simultaneous() ) {
			if ( diag ) std::cout << "Simultaneous trigger event at t = " << t << std::endl;
			EventQueue< Variable >::Variables triggers( events.simultaneous_variables() ); // Chg to generator approach to avoid heap hit // Sort/ptn by QSS order to save unnec loops/calls below
			for ( Variable * trigger : triggers ) {
				assert( trigger->tE == t );
				trigger->advance0();
			}
			for ( Variable * trigger : triggers ) {
				trigger->advance1_LIQSS();
			}
			for ( Variable * trigger : triggers ) {
				trigger->advance1();
			}
			if ( QSS_order_max >= 2 ) {
				for ( Variable * trigger : triggers ) {
					trigger->advance2_LIQSS();
				}
				for ( Variable * trigger : triggers ) {
					trigger->advance2();
				}
				if ( QSS_order_max >= 3 ) {
					for ( Variable * trigger : triggers ) {
						trigger->advance3();
					}
				}
			}
			for ( Variable * trigger : triggers ) {
				trigger->advance_observers();
			}
			for ( Variable * trigger : triggers ) {
				if ( all_vars_out ) {
					for ( size_type i = 0; i < n_vars; ++i ) {
						q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
						x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
					}
				} else {
					for ( size_type i = 0; i < n_vars; ++i ) { // Give Variable access to its stream to avoid this loop
						if ( trigger == vars[ i ] ) {
							if ( q_out ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
							x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
							break;
						}
					}
				}
			}
		} else {
			Variable * trigger( events.top() );
			assert( trigger->tE == t );
			trigger->advance();
			if ( all_vars_out ) {
				for ( size_type i = 0; i < n_vars; ++i ) {
					q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
					x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
				}
			} else {
				for ( size_type i = 0; i < n_vars; ++i ) { // Give Variable access to its stream to avoid this loop
					if ( trigger == vars[ i ] ) {
						if ( q_out ) q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
						x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
						break;
					}
				}
			}
		}
	}
	for ( size_type i = 0; i < n_vars; ++i ) { // Add tE outputs
		Variable const * var( vars[ i ] );
		if ( var->tQ < tE ) {
			if ( q_out ) q_streams[ i ] << tE << '\t' << var->q( tE ) << '\n';
			x_streams[ i ] << tE << '\t' << var->x( tE ) << '\n';
		}
	}
	for ( size_type i = 0; i < n_vars; ++i ) { // Close streams
		if ( q_out ) q_streams[ i ].close();
		x_streams[ i ].close();
	}
}

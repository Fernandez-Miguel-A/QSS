// QSS Solver Prototype Runner

// C++ Headers
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

// QSS Headers
#include <QSS/globals.hh>
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

	enum QSS_Method { QSS1 = 1, QSS2, QSS3, QSS4 };

	// Settings
	bool const sampled( false ); // Sampled outputs?
	bool const all_vars_out( false ); // Output all variables at every requantization event?
	double const dto( 1.0e-3 ); // Sampling time step

//	// Simple x, y, z
//	double const tEnd( 5.0 );
//	double t( 0.0 ), to( t + dto );
//	QSS_Method const qss_max( QSS2 );
//	VariableQSS2 x( "x", 1.0, 0.0 );
//	VariableQSS2 y( "y", 1.0, 0.0 );
//	VariableQSS2 z( "z", 1.0, 0.0 );
//	x.init_val( 0.0 );
//	y.init_val( 0.0 );
//	z.init_val( 0.0 );
//	x.add_der( 1.0 ).finalize_der();
//	y.add_der( 1.0, x ).finalize_der();
//	z.add_der( 1.0, y ).finalize_der();
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
//	double const tEnd( 10.0 );
//	double t( 0.0 ), to( t + dto );
//	QSS_Method const qss_max( QSS2 );
//	VariableQSS2 x1( "x1", 1.0, 0.0 );
//	VariableQSS2 x2( "x2", 1.0, 0.0 );
//	x1.init_val( 1.0 );
//	x2.init_val( 0.0 );
//	x1.add_der( -1.0, x2 ).finalize_der();
//	x2.add_der( 1.0, x1 ).finalize_der();
//	vars.reserve( 2 );
//	vars.push_back( &x1 );
//	vars.push_back( &x2 );

	// Achilles and the Tortoise
	double const tEnd( 10.0 );
	double t( 0.0 ), to( t + dto );
	QSS_Method const qss_max( QSS2 );
	VariableQSS2 x1( "x1", 1.0, 0.0 );
	VariableQSS2 x2( "x2", 1.0, 0.0 );
	x1.init_val( 0.0 );
	x2.init_val( 2.0 );
	x1.add_der( -0.5, x1 ).add_der( 1.5, x2 ).finalize_der();
	x2.add_der( -1.0, x1 ).finalize_der();
	vars.reserve( 2 );
	vars.push_back( &x1 );
	vars.push_back( &x2 );

	// Solver master logic
	for ( auto var : vars ) {
		var->init_der();
	}
	if ( qss_max >= QSS2 ) {
		for ( auto var : vars ) {
			var->init_der2();
		}
		if ( qss_max >= QSS3 ) {
			for ( auto var : vars ) {
				var->init_der3();
			}
		}
	}
	for ( auto var : vars ) {
		var->init_event();
	}
	for ( auto var : vars ) {
		q_streams.push_back( std::ofstream( var->name + "_q.out" ) );
		x_streams.push_back( std::ofstream( var->name + "_x.out" ) );
		q_streams.back() << std::setprecision( 16 ) << t << '\t' << var->q( t ) << '\n';
		x_streams.back() << std::setprecision( 16 ) << t << '\t' << var->x( t ) << '\n';
	}
	size_type n_vars( vars.size() );
	while ( ( t <= tEnd ) || ( sampled && ( to <= tEnd ) ) ) {
		t = events.top_time();
		if ( sampled ) { // Sampled outputs
			while ( to < std::min( t, tEnd ) ) {
				for ( size_type i = 0; i < n_vars; ++i ) {
					q_streams[ i ] << to << '\t' << vars[ i ]->q( to ) << '\n';
					x_streams[ i ] << to << '\t' << vars[ i ]->x( to ) << '\n';
				}
				to += dto;
			}
		}
		if ( events.simultaneous() ) {
//			std::cout << "Simultaneous trigger event at t = " << t << std::endl;
			EventQueue< Variable >::Variables triggers( events.simultaneous_variables() ); // Chg to generator approach to avoid heap hit // Sort/ptn by QSS order to save unnec loops/calls below
			for ( Variable * trigger : triggers ) {
				assert( trigger->tEnd == t );
				trigger->advance0();
			}
			for ( Variable * trigger : triggers ) {
				trigger->advance1();
			}
			if ( qss_max >= QSS2 ) {
				for ( Variable * trigger : triggers ) {
					trigger->advance2();
				}
				if ( qss_max >= QSS3 ) {
					for ( Variable * trigger : triggers ) {
						trigger->advance3();
					}
				}
			}
			for ( Variable * trigger : triggers ) {
				trigger->advance_observers();
			}
			for ( Variable * trigger : triggers ) {
				if ( t <= tEnd ) { // Requantization outputs
					if ( all_vars_out ) {
						for ( size_type i = 0; i < n_vars; ++i ) {
							q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
							x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
						}
					} else {
						for ( size_type i = 0; i < n_vars; ++i ) { // Give Variable access to its stream to avoid this loop
							if ( trigger == vars[ i ] ) {
								q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
								x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
								break;
							}
						}
					}
				}
			}
		} else {
			Variable * trigger( events.top() );
			assert( trigger->tEnd == t );
			trigger->advance();
			if ( t <= tEnd ) { // Requantization outputs
				if ( all_vars_out ) {
					for ( size_type i = 0; i < n_vars; ++i ) {
						q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
						x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
					}
				} else {
					for ( size_type i = 0; i < n_vars; ++i ) { // Give Variable access to its stream to avoid this loop
						if ( trigger == vars[ i ] ) {
							q_streams[ i ] << t << '\t' << vars[ i ]->q( t ) << '\n';
							x_streams[ i ] << t << '\t' << vars[ i ]->x( t ) << '\n';
							break;
						}
					}
				}
			}
		}
	}
}

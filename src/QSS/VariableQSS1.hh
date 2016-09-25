#ifndef QSS_VariableQSS1_hh_INCLUDED
#define QSS_VariableQSS1_hh_INCLUDED

// QSS1 Variable

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/EventQueue.hh>
#include <QSS/Function.hh>
#include <QSS/globals.hh>

// QSS1 Variable
class VariableQSS1 final : public Variable
{

public: // Types

	using Time = Variable::Time;
	using Derivative = Function< Variable >;
	using Coefficient = Derivative::Coefficient;

public: // Creation

	// Constructor
	explicit
	VariableQSS1(
	 std::string const & name,
	 double const aTol = 1.0e-6,
	 double const rTol = 1.0e-6
	) :
	 Variable( name, aTol, rTol )
	{}

public: // Properties

	// Order of QSS Method
	int
	order() const
	{
		return 1;
	}

	// Continuous Value at Time t
	double
	x( Time const t ) const
	{
		assert( ( tCon <= t ) && ( t <= tEnd ) );
		return x0_ + ( x1_ * ( t - tCon ) );
	}

	// Quantized Value at Time tBeg
	double
	q() const
	{
		return q_;
	}

	// Quantized Value at Time t
	double
	q( Time const t ) const
	{
		assert( ( tBeg <= t ) && ( t <= tEnd ) );
		(void)t; // Suppress unused parameter warning
		return q_;
	}

	// Next End Time on Trigger Update
	Time
	tEndTrigger() const
	{
		return ( x1_ != 0.0 ? tBeg + ( qTol / std::abs( x1_ ) ) : infinity );
	}

	// Next End Time on Observer Update
	Time
	tEndObserver() const
	{
		if ( advanced ) {
			return
			 ( x1_ > 0.0 ? tCon + ( ( ( q_ - x0_ ) + qTol ) / x1_ ) :
			 ( x1_ < 0.0 ? tCon + ( ( ( q_ - x0_ ) - qTol ) / x1_ ) :
			 infinity ) );
		} else {
			assert( tBeg == tCon );
			assert( q_ == x0_ );
			return ( x1_ != 0.0 ? tBeg + ( qTol / std::abs( x1_ ) ) : infinity );
		}
	}

public: // Methods

	// Initialize Value
	VariableQSS1 &
	init_val()
	{
		q_ = x0_;
		set_qTol();
		return *this;
	}

	// Initialize to Value
	VariableQSS1 &
	init_val( double const xBeg )
	{
		x0_ = q_ = xBeg;
		set_qTol();
		return *this;
	}

	// Add Constant to Derivative
	VariableQSS1 &
	add_der( Coefficient const c0 )
	{
		d_.add( c0 );
		return *this;
	}

	// Add a Coefficient + Variable to Derivative
	VariableQSS1 &
	add_der(
	 Coefficient const c_i,
	 Variable & x_i
	)
	{
		d_.add( c_i, x_i );
		x_i.add_observer( this );
		return *this;
	}

	// Add a Coefficient + Variable to Derivative
	VariableQSS1 &
	add_der(
	 Coefficient const c_i,
	 Variable * x_i
	)
	{
		d_.add( c_i, x_i );
		x_i->add_observer( this );
		return *this;
	}

	// Finalize Derivative Function
	void
	finalize_der()
	{
		d_.finalize();
	}

	// Initialize First Derivative
	void
	init_der()
	{
		x1_ = d_.q0();
	}

	// Initialize Event in Queue
	void
	init_event()
	{
		tEnd = tEndTrigger();
		event( events.add( tEnd, this ) );
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( aTol, rTol * std::abs( q_ ) );
		assert( qTol > 0.0 );
	}

	// Advance Trigger to Time tEnd and Requantize
	void
	advance()
	{
		x0_ = q_ = x0_ + ( x1_ * ( tEnd - tCon ) );
		x1_ = d_.q( tBeg = tCon = tEnd );
		set_qTol();
		advanced = false;
		tEnd = tEndTrigger();
		event( events.shift( tEnd, event() ) );
		for ( Variable * observer : observers() ) { // Advance observers
			observer->advance( tBeg );
		}
	}

	// Advance Simultaneous Trigger to Time tEnd and Requantize: Step 0
	void
	advance0()
	{
		x0_ = q_ = x0_ + ( x1_ * ( tEnd - tCon ) );
		set_qTol();
	}

	// Advance Simultaneous Trigger to Time tEnd and Requantize: Step 1
	void
	advance1()
	{
		x1_ = d_.q( tBeg = tCon = tEnd );
		advanced = false;
		tEnd = tEndTrigger();
		event( events.shift( tEnd, event() ) );
	}

	// Advance Observer to Time t
	void
	advance( Time const t )
	{
		assert( ( tCon <= t ) && ( t <= tEnd ) );
		if ( tCon < t ) { // Could observe multiple variables with simultaneous triggering
			x0_ = x0_ + ( x1_ * ( t - tCon ) );
			x1_ = d_.q( tCon = t );
			advanced = true;
			tEnd = tEndObserver();
			event( events.shift( tEnd, event() ) );
		}
	}

private: // Data

	double x0_{ 0.0 }, x1_{ 0.0 }; // Continuous value coefficients for active time segment
	double q_{ 0.0 }; // Quantized value for active time segment
	Derivative d_; // Derivative function

};

#endif

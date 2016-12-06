#ifndef QSS_VariableQSS1_hh_INCLUDED
#define QSS_VariableQSS1_hh_INCLUDED

// QSS1 Variable

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/globals.hh>

// QSS1 Variable
template< template< typename > typename F >
class VariableQSS1 final : public Variable
{

public: // Types

	using Value = Variable::Value;
	using Time = Variable::Time;
	template< typename V > using Function = F< V >;
	using Derivative = Function< Variable >;
	using Coefficient = typename Derivative::Coefficient;

public: // Creation

	// Constructor
	explicit
	VariableQSS1(
	 std::string const & name,
	 Value const aTol = 1.0e-6,
	 Value const rTol = 1.0e-6
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

	// Continuous Value at Time tC
	Value
	x() const
	{
		return x0_;
	}

	// Continuous Value at Time tC
	Value
	x0() const
	{
		return x0_;
	}

	// Continuous Value at Time tC
	Value &
	x0()
	{
		return x0_;
	}

	// Continuous First Derivative at Time tC
	Value
	x1() const
	{
		return x1_;
	}

	// Continuous First Derivative at Time tC
	Value &
	x1()
	{
		return x1_;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		assert( ( tC <= t ) && ( t <= tE ) );
		return x0_ + ( x1_ * ( t - tC ) );
	}

	// Quantized Value at Time tQ
	Value
	q() const
	{
		return q0_;
	}

	// Quantized Value at Time tQ
	Value
	q0() const
	{
		return q0_;
	}

	// Quantized Value at Time tQ
	Value &
	q0()
	{
		return q0_;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return q0_;
	}

	// Derivative Function
	Derivative const &
	d() const
	{
		return d_;
	}

	// Derivative Function
	Derivative &
	d()
	{
		return d_;
	}

public: // Methods

	// Initialize Constant Term
	VariableQSS1 &
	init0( Value const x )
	{
		x0_ = q0_ = x;
		set_qTol();
		return *this;
	}

	// Initialize Linear Coefficient
	void
	init1()
	{
		self_observer = d_.finalize( this );
		shrink_observers(); // Optional
		x1_ = d_.q();
	}

	// Initialize Event in Queue
	void
	init_event()
	{
		set_tE_aligned();
		event( events.add( tE, this ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q0_ << " quantized, " << x0_ << "+" << x1_ << "*t internal   tE=" << tE << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( aTol, rTol * std::abs( q0_ ) );
		assert( qTol > 0.0 );
	}

	// Advance Trigger to Time tE and Requantize
	void
	advance()
	{
		q0_ = x0_ + ( x1_ * ( ( tQ = tE ) - tC ) );
		set_qTol();
		if ( self_observer ) {
			x0_ = q0_;
			x1_ = d_.q( tC = tE );
		}
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q0_ << " quantized, " << x0_ << "+" << x1_ << "*t internal   tE=" << tE << '\n';
		for ( Variable * observer : observers() ) { // Advance (other) observers
			observer->advance( tQ );
		}
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 0
	void
	advance0()
	{
		q0_ = x0_ + ( x1_ * ( ( tQ = tE ) - tC ) );
		set_qTol();
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 1
	void
	advance1()
	{
		{ // Only need to do this if observer of self or other simultaneously requantizing variables
			x0_ = q0_;
			x1_ = d_.q( tC = tE );
		}
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q0_ << " quantized, " << x0_ << "+" << x1_ << "*t internal   tE=" << tE << '\n';
	}

	// Advance Observer to Time t
	void
	advance( Time const t )
	{
		assert( ( tC <= t ) && ( t <= tE ) );
		if ( tC < t ) { // Could observe multiple variables with simultaneous triggering
			x0_ = x0_ + ( x1_ * ( t - tC ) );
			x1_ = d_.q( tC = t );
			set_tE_unaligned();
			event( events.shift( tE, event() ) );
			if ( diag ) std::cout << "  " << name << '(' << t << ')' << " = " << q0_ << " quantized, " << x0_ << "+" << x1_ << "*t internal   tE=" << tE << '\n';
		}
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
	{
		assert( tC <= tQ ); // Quantized and continuous trajectories align at tQ
		tE = ( x1_ != 0.0 ? tQ + ( qTol / std::abs( x1_ ) ) : infinity );
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tC );
		tE =
		 ( x1_ > 0.0 ? tC + ( ( ( q0_ - x0_ ) + qTol ) / x1_ ) :
		 ( x1_ < 0.0 ? tC + ( ( ( q0_ - x0_ ) - qTol ) / x1_ ) :
		 infinity ) );
	}

private: // Data

	Value x0_{ 0.0 }, x1_{ 0.0 }; // Continuous value coefficients for active time segment
	Value q0_{ 0.0 }; // Quantized value for active time segment
	Derivative d_; // Derivative function

};

#endif

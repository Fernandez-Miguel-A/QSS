#ifndef QSS_VariableLIQSS2_hh_INCLUDED
#define QSS_VariableLIQSS2_hh_INCLUDED

// LIQSS2 Variable

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/globals.hh>

// LIQSS2 Variable
template< template< typename > typename F >
class VariableLIQSS2 final : public Variable
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
	VariableLIQSS2(
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
		return 2;
	}

	// Continuous Value at Time tX
	Value
	x() const
	{
		return x0_;
	}

	// Continuous Value at Time tX
	Value
	x0() const
	{
		return x0_;
	}

	// Continuous Value at Time tX
	Value &
	x0()
	{
		return x0_;
	}

	// Continuous First Derivative at Time tX
	Value
	x1() const
	{
		return x1_;
	}

	// Continuous First Derivative at Time tX
	Value &
	x1()
	{
		return x1_;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		return x0_ + ( ( x1_ + ( x2_ * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		return x1_ + ( two * x2_ * ( t - tX ) );
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

	// Quantized First Derivative at Time tQ
	Value
	q1() const
	{
		return q1_;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		return q0_ + ( q1_ * ( t - tQ ) );
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return q1_;
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
	VariableLIQSS2 &
	init0( Value const x )
	{
		x0_ = qc_ = q0_ = x;
		set_qTol();
		return *this;
	}

	// Initialize Linear Coefficient
	void
	init1()
	{
		self_observer = d_.finalize( this );
		shrink_observers(); // Optional
		// For self-observer this is a first pass to set a reasonable x1_ = q1_ for init2 calls
		// This avoids variable processing order dependency but not inconsistent rep usage
		x1_ = q1_ = d_.x(); // Continuous rep used to avoid cyclic dependency
	}

	// Initialize Quadratic Coefficient in LIQSS Variable
	void
	init2_LIQSS()
	{
		if ( self_observer ) {
			d_.liqss2_x( tQ, qTol, q0_, q1_, x1_, x2_ ); // Continuous rep used to avoid cyclic dependency
		} else {
			x2_ = one_half * d_.x1(); // Continuous rep used to avoid cyclic dependency
			q0_ += signum( x2_ ) * qTol;
		}
	}

	// Initialize Event in Queue
	void
	init_event()
	{
		set_tE_aligned();
		event( events.add( tE, this ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q0_ << "+" << q1_ << "*t quantized, " << x0_ << "+" << x1_ << "*t+" << x2_ << "*t^2 internal   tE=" << tE << '\n';
	}

	// Set Current Tolerance
	void
	set_qTol()
	{
		qTol = std::max( aTol, rTol * std::abs( qc_ ) );
		assert( qTol > 0.0 );
	}

	// Advance Trigger to Time tE and Requantize
	void
	advance()
	{
		Time const tDel( ( tQ = tE ) - tX );
		qc_ = q0_ = x0_ + ( ( x1_ + ( x2_ * tDel ) ) * tDel );
		set_qTol();
		if ( self_observer ) {
			x0_ = qc_;
			d_.liqss2( tQ, qTol, q0_, q1_, x1_, x2_ );
			tX = tE;
		} else {
			q0_ += signum( x2_ ) * qTol;
			q1_ = x1_ + ( 2.0 * x2_ * tDel );
		}
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q0_ << "+" << q1_ << "*t quantized, " << x0_ << "+" << x1_ << "*t+" << x2_ << "*t^2 internal   tE=" << tE << '\n';
		for ( Variable * observer : observers() ) { // Advance (other) observers
			observer->advance( tQ );
		}
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 0
	void
	advance0()
	{
		Time const tDel( ( tQ = tE ) - tX );
		x0_ = qc_ = q0_ = x0_ + ( ( x1_ + ( x2_ * tDel ) ) * tDel );
		set_qTol();
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 1
	void
	advance1()
	{
		//Note Could skip continuous rep update if not observer of self or other simultaneously requantizing variables
		x1_ = q1_ = d_.x( tQ ); // Continuous rep used to avoid cyclic dependency: Neutral initialization
	}

	// Advance Simultaneous Trigger in LIQSS Variable to Time tE and Requantize: Step 2
	void
	advance2_LIQSS()
	{
		d_.liqss2_x( tQ, qTol, q0_, q1_, x1_, x2_ ); // Continuous rep used to avoid cyclic dependency
		tX = tE;
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q0_ << "+" << q1_ << "*t quantized, " << x0_ << "+" << x1_ << "*t+" << x2_ << "*t^2 internal   tE=" << tE << '\n';
	}

	// Advance Observer to Time t
	void
	advance( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		if ( tX < t ) { // Could observe multiple variables with simultaneous triggering
			Time const tDel( t - tX );
			x0_ = x0_ + ( ( x1_ + ( x2_ * tDel ) ) * tDel );
			x1_ = d_.q( t );
			x2_ = one_half * d_.q1( tX = t );
			set_tE_unaligned();
			event( events.shift( tE, event() ) );
			if ( diag ) std::cout << "  " << name << '(' << t << ')' << " = " << q0_ << "+" << q1_ << "*t quantized, " << x0_ << "+" << x1_ << "*t+" << x2_ << "*t^2 internal   tE=" << tE << '\n';
		}
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
	{
		assert( tX <= tQ );
		tE = ( x2_ != 0.0 ? tQ + std::sqrt( qTol / std::abs( x2_ ) ) : infinity );
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		Value const d0( x0_ - ( qc_ + ( q1_ * ( tX - tQ ) ) ) );
		Value const d1( x1_ - q1_ );
		if ( d1 >= 0.0 ) {
			Time const tPosQ( min_root_quadratic( x2_, d1, d0 - qTol ) );
			if ( x2_ >= 0.0 ) { // Only need to check +qTol
				tE = ( tPosQ == infinity ? infinity : tX + tPosQ );
			} else {
				Time const tNegQ( min_root_quadratic( x2_, d1, d0 + qTol ) );
				Time const tMinQ( std::min( tPosQ, tNegQ ) );
				tE = ( tMinQ == infinity ? infinity : tX + tMinQ );
			}
		} else { // d1 < 0
			Time const tNegQ( min_root_quadratic( x2_, d1, d0 + qTol ) );
			if ( x2_ <= 0.0 ) { // Only need to check -qTol
				tE = ( tNegQ == infinity ? infinity : tX + tNegQ );
			} else {
				Time const tPosQ( min_root_quadratic( x2_, d1, d0 - qTol ) );
				Time const tMinQ( std::min( tPosQ, tNegQ ) );
				tE = ( tMinQ == infinity ? infinity : tX + tMinQ );
			}
		}
	}

private: // Data

	Value x0_{ 0.0 }, x1_{ 0.0 }, x2_{ 0.0 }; // Continuous value coefficients for active time segment
	Value qc_{ 0.0 }, q0_{ 0.0 }, q1_{ 0.0 }; // Quantized centered and actual value coefficients for active time segment
	Derivative d_; // Derivative function

};

#endif

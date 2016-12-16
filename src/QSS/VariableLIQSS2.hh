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

	// Order of Method
	int
	order() const
	{
		return 2;
	}

	// Continuous Value at Time t
	Value
	x( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Continuous Numeric Differenentiation Value at Time t: Allow t Outside of [tX,tE]
	Value
	xn( Time const t ) const
	{
		Time const tDel( t - tX );
		return x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
	}

	// Continuous First Derivative at Time t
	Value
	x1( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		return x_1_ + ( two * x_2_ * ( t - tX ) );
	}

	// Continuous Second Derivative at Time t
	Value
	x2( Time const t ) const
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		return two * x_2_;
	}

	// Quantized Value at Time t
	Value
	q( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		return q_0_ + ( q_1_ * ( t - tQ ) );
	}

	// Quantized Numeric Differenentiation Value at Time t: Allow t Outside of [tQ,tE]
	Value
	qn( Time const t ) const
	{
		return q_0_ + ( q_1_ * ( t - tQ ) );
	}

	// Quantized First Derivative at Time t
	Value
	q1( Time const t ) const
	{
		assert( ( tQ <= t ) && ( t <= tE ) );
		(void)t; // Suppress unused parameter warning
		return q_1_;
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

	// Initialize Constant Term to Given Value
	void
	init0( Value const x )
	{
		x_0_ = qc_ = q_0_ = x;
		set_qTol();
	}

	// Initialize Linear Coefficient
	void
	init1()
	{
		self_observer = d_.finalize( this );
		shrink_observers(); // Optional
		// For self-observer this is a first pass to set a reasonable x_1_ = q_1_ for init2 calls
		// This avoids variable processing order dependency but not inconsistent rep usage
		x_1_ = q_1_ = d_.x( tQ ); // Continuous rep used to reduce impact of cyclic dependency
	}

	// Initialize Quadratic Coefficient in LIQSS Variable
	void
	init2_LIQSS()
	{ // Call before init2 since it alters q_0_
		if ( self_observer ) {
			advance_x(); // Continuous rep used to reduce impact of cyclic dependency
		} else {
			x_2_ = one_half * d_.x1( tQ ); // Continuous rep used to reduce impact of cyclic dependency
			q_0_ += signum( x_2_ ) * qTol;
		}
	}

	// Initialize Event in Queue
	void
	init_event()
	{
		set_tE_aligned();
		event( events.add( tE, this ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
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
		qc_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		set_qTol();
		if ( self_observer ) {
			x_0_ = qc_;
			advance_q();
			tX = tE;
		} else {
			q_0_ += signum( x_2_ ) * qTol;
			q_1_ = x_1_ + ( two * x_2_ * tDel );
		}
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "! " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
		advance_observers();
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 0
	void
	advance0()
	{
		Time const tDel( ( tQ = tE ) - tX );
		x_0_ = qc_ = q_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
		set_qTol();
		tX = tE;
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 1
	void
	advance1()
	{
		x_1_ = q_1_ = d_.x( tE ); // Continuous rep used to reduce impact of cyclic dependency: Neutral initialization
	}

	// Advance Simultaneous Trigger in LIQSS Variable to Time tE and Requantize: Step 2
	void
	advance2_LIQSS()
	{ // Call before advance2 since it alters q_0_
		if ( self_observer ) {
			advance_x(); // Continuous rep used to reduce impact of cyclic dependency
		} else {
			x_2_ = one_half * d_.x1( tE ); // Continuous rep used to reduce impact of cyclic dependency
			q_0_ += signum( x_2_ ) * qTol;
		}
	}

	// Advance Simultaneous Trigger to Time tE and Requantize: Step 2
	void
	advance2()
	{
		set_tE_aligned();
		event( events.shift( tE, event() ) );
		if ( diag ) std::cout << "= " << name << '(' << tQ << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
	}

	// Advance Observer to Time t
	void
	advance( Time const t )
	{
		assert( ( tX <= t ) && ( t <= tE ) );
		if ( tX < t ) { // Could observe multiple variables with simultaneous triggering
			Time const tDel( t - tX );
			x_0_ = x_0_ + ( ( x_1_ + ( x_2_ * tDel ) ) * tDel );
			x_1_ = d_.qs( t );
			x_2_ = one_half * d_.qf1( tX = t );
			set_tE_unaligned();
			event( events.shift( tE, event() ) );
			if ( diag ) std::cout << "  " << name << '(' << t << ')' << " = " << q_0_ << "+" << q_1_ << "*t quantized, " << x_0_ << "+" << x_1_ << "*t+" << x_2_ << "*t^2 internal   tE=" << tE << '\n';
		}
	}

private: // Methods

	// Set End Time: Quantized and Continuous Aligned
	void
	set_tE_aligned()
	{
		assert( tX <= tQ );
		tE = ( x_2_ != 0.0 ? tQ + std::sqrt( qTol / std::abs( x_2_ ) ) : infinity );
		if ( dt_max != infinity ) tE = std::min( tE, tQ + dt_max );
		if ( ( inflection_steps ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tQ < tI ) tE = std::min( tE, tI );
		}
	}

	// Set End Time: Quantized and Continuous Unaligned
	void
	set_tE_unaligned()
	{
		assert( tQ <= tX );
		Value const d0( x_0_ - ( qc_ + ( q_1_ * ( tX - tQ ) ) ) );
		Value const d1( x_1_ - q_1_ );
		if ( d1 >= 0.0 ) {
			Time const tPosQ( min_root_quadratic( x_2_, d1, d0 - qTol ) );
			if ( x_2_ >= 0.0 ) { // Only need to check +qTol
				tE = ( tPosQ == infinity ? infinity : tX + tPosQ );
			} else {
				Time const tNegQ( min_root_quadratic( x_2_, d1, d0 + qTol ) );
				Time const tMinQ( std::min( tPosQ, tNegQ ) );
				tE = ( tMinQ == infinity ? infinity : tX + tMinQ );
			}
		} else { // d1 < 0
			Time const tNegQ( min_root_quadratic( x_2_, d1, d0 + qTol ) );
			if ( x_2_ <= 0.0 ) { // Only need to check -qTol
				tE = ( tNegQ == infinity ? infinity : tX + tNegQ );
			} else {
				Time const tPosQ( min_root_quadratic( x_2_, d1, d0 - qTol ) );
				Time const tMinQ( std::min( tPosQ, tNegQ ) );
				tE = ( tMinQ == infinity ? infinity : tX + tMinQ );
			}
		}
		if ( dt_max != infinity ) tE = std::min( tE, tX + dt_max );
		if ( ( inflection_steps ) && ( x_2_ != 0.0 ) && ( signum( x_1_ ) != signum( x_2_ ) ) && ( signum( x_1_ ) == signum( q_1_ ) ) ) {
			Time const tI( tX - ( x_1_ / ( two * x_2_ ) ) );
			if ( tX < tI ) tE = std::min( tE, tI );
		}
	}

	// Advance Self-Observing Trigger using Quantized Derivative
	void
	advance_LIQSS( AdvanceSpecsLIQSS2 const & specs )
	{
		assert( qTol > 0.0 );
		assert( self_observer );

		// Set coefficients based on second derivative signs
		int const dls( signum( specs.l2 ) );
		int const dus( signum( specs.u2 ) );
		if ( ( dls == -1 ) && ( dus == -1 ) ) { // Downward curving trajectory
			q_0_ -= qTol;
			x_1_ = q_1_ = specs.l1;
			x_2_ = one_half * specs.l2;
		} else if ( ( dls == +1 ) && ( dus == +1 ) ) { // Upward curving trajectory
			q_0_ += qTol;
			x_1_ = q_1_ = specs.u1;
			x_2_ = one_half * specs.u2;
		} else { // Straight trajectory
			x_1_ = q_1_ = specs.z1;
			q_0_ = std::min( std::max( specs.z2, q_0_ - qTol ), q_0_ + qTol ); // Clipped in case of roundoff
			x_2_ = 0.0;
		}
	}

	// Advance Self-Observing Trigger using Quantized Derivative
	void
	advance_q()
	{
		assert( ( tQ == tE ) || ( tQ == 0.0 ) ); // Precondition: tQ set to trigger time
		advance_LIQSS( d_.qlu2( tQ, qTol ) ); // Quantized rep used for single trigger
	}

	// Advance Self-Observing Trigger using Continuous Derivative
	void
	advance_x()
	{
		assert( ( tQ == tE ) || ( tQ == 0.0 ) ); // Precondition: tQ set to trigger time
		advance_LIQSS( d_.xlu2( tQ, qTol ) ); // Continuous rep used for simultaneous triggers
	}

private: // Data

	Value x_0_{ 0.0 }, x_1_{ 0.0 }, x_2_{ 0.0 }; // Continuous rep coefficients
	Value qc_{ 0.0 }, q_0_{ 0.0 }, q_1_{ 0.0 }; // Quantized rep coefficients
	Derivative d_; // Derivative function

};

#endif

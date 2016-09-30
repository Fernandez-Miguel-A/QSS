#ifndef QSS_VariableQSS2_hh_INCLUDED
#define QSS_VariableQSS2_hh_INCLUDED

// QSS2 Variable

// QSS Headers
#include <QSS/Variable.hh>
#include <QSS/globals.hh>

// QSS2 Variable
template< template< typename > typename F >
class VariableQSS2 final : public Variable
{

public: // Types

	using Time = Variable::Time;
	template< typename V > using Function = F< V >;
	using Derivative = Function< Variable >;
	using Coefficient = typename Derivative::Coefficient;

public: // Creation

	// Constructor
	explicit
	VariableQSS2(
	 std::string const & name,
	 double const aTol = 1.0e-6,
	 double const rTol = 1.0e-6
	) :
	 Variable( name, aTol, rTol )
	{}

public: // Properties

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

	// Order of QSS Method
	int
	order() const
	{
		return 2;
	}

	// Continuous Value at Time t
	double
	x( Time const t ) const
	{
		assert( ( tCon <= t ) && ( t <= tEnd ) );
		Time const tDel( t - tCon );
		return x0_ + ( x1_ * tDel ) + ( x2_ * ( tDel * tDel ) );
	}

	// Quantized Value at Time tBeg
	double
	q() const
	{
		return q0_;
	}

	// Quantized First Derivative at Time tBeg
	double
	q1() const
	{
		return q1_;
	}

	// Quantized Value at Time t
	double
	q( Time const t ) const
	{
		assert( ( tBeg <= t ) && ( t <= tEnd ) );
		return q0_ + ( q1_ * ( t - tBeg ) );
	}

	// Quantized First Derivative at Time t
	double
	q1( Time const t ) const
	{
		assert( ( tBeg <= t ) && ( t <= tEnd ) );
		(void)t; // Suppress unused parameter warning
		return q1_;
	}

	// Next End Time on Trigger Update
	Time
	tEndTrigger() const
	{
		return ( x2_ != 0.0 ? tBeg + std::sqrt( qTol / std::abs( x2_ ) ) : infinity );
	}

	// Next End Time on Observer Update
	Time
	tEndObserver() const
	{
		if ( advanced ) {
			double const d0( x0_ - ( q0_ + ( q1_ * ( tCon - tBeg ) ) ) );
			if ( x1_ - q1_ >= 0.0 ) {
				Time const tPosQ( min_root_quadratic( x2_, x1_ - q1_, d0 - qTol ) );
				if ( x2_ >= 0.0 ) { // Only need to check +qTol
					return ( tPosQ == infinity ? infinity : tCon + tPosQ );
				} else {
					Time const tNegQ( min_root_quadratic( x2_, x1_ - q1_, d0 + qTol ) );
					Time const tMinQ( std::min( tPosQ, tNegQ ) );
					return ( tMinQ == infinity ? infinity : tCon + tMinQ );
				}
			} else { // x1 - q1 < 0
				Time const tNegQ( min_root_quadratic( x2_, x1_ - q1_, d0 + qTol ) );
				if ( x2_ <= 0.0 ) { // Only need to check -qTol
					return ( tNegQ == infinity ? infinity : tCon + tNegQ );
				} else {
					Time const tPosQ( min_root_quadratic( x2_, x1_ - q1_, d0 - qTol ) );
					Time const tMinQ( std::min( tPosQ, tNegQ ) );
					return ( tMinQ == infinity ? infinity : tCon + tMinQ );
				}
			}
		} else {
			assert( tBeg == tCon );
			assert( q0_ == x0_ );
			assert( q1_ == x1_ );
			return ( x2_ != 0.0 ? tBeg + std::sqrt( qTol / std::abs( x2_ ) ) : infinity );
		}
	}

public: // Methods

	// Finalize Derivative Function
	void
	finalize_der()
	{
		d_.finalize( this );
	}

	// Initialize Constant Term
	VariableQSS2 &
	init0( double const x )
	{
		x0_ = q0_ = x;
		set_qTol();
		return *this;
	}

	// Initialize Linear Coefficient
	void
	init1()
	{
		x1_ = q1_ = d_.q();
	}

	// Initialize Quadratic Coefficient
	void
	init2()
	{
		x2_ = one_half * d_.q1();
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
		qTol = std::max( aTol, rTol * std::abs( q0_ ) );
		assert( qTol > 0.0 );
	}

	// Advance Trigger to Time tEnd and Requantize
	void
	advance()
	{
		Time const tDel( tEnd - tCon );
		x0_ = q0_ = x0_ + ( x1_ * tDel ) + ( x2_ * ( tDel * tDel ) );
		x1_ = q1_ = d_.q( tBeg = tCon = tEnd );
		x2_ = one_half * d_.q1( tBeg );
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
		Time const tDel( tEnd - tCon );
		x0_ = q0_ = x0_ + ( x1_ * tDel ) + ( x2_ * ( tDel * tDel ) );
		set_qTol();
	}

	// Advance Simultaneous Trigger to Time tEnd and Requantize: Step 1
	void
	advance1()
	{
		x1_ = q1_ = d_.q( tBeg = tCon = tEnd );
	}

	// Advance Simultaneous Trigger to Time tEnd and Requantize: Step 2
	void
	advance2()
	{
		x2_ = one_half * d_.q1( tBeg );
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
			Time const tDel( t - tCon );
			x0_ = x0_ + ( x1_ * tDel ) + ( x2_ * ( tDel * tDel ) );
			x1_ = d_.q( t );
			x2_ = one_half * d_.q1( t );
			tCon = t;
			advanced = true;
			tEnd = tEndObserver();
			event( events.shift( tEnd, event() ) );
		}
	}

private: // Data

	Derivative d_; // Derivative function
	double x0_{ 0.0 }, x1_{ 0.0 }, x2_{ 0.0 }; // Continuous value coefficients for active time segment
	double q0_{ 0.0 }, q1_{ 0.0 }; // Quantized value coefficients for active time segment

};

#endif

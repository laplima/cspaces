#include "TupleSpaceI.h"
#include "TupleSpaceC.h"
#include "tao/Basic_Types.h"
#include "tao/Object.h"
#include "tao/Typecode_typesC.h"
#include <mutex>
#include <print>
#include <iostream>
#include <cstring>
#include <colibry/ORBManager.h>
#include <memory>
#include <spdlog/spdlog.h>

// #define DEBUGGING true
// #define DEBUGMSG(msg) if (DEBUGGING) cout << msg << endl

using namespace std;
namespace CS=CSpace;
using spdlog::error;
using spdlog::debug;

// namespace {
// 	mutex mtx;          // global mutex for tuplespace
// 	mutex mtx_pending;	// global mutex for pending requests
// }

namespace global {
	extern std::unique_ptr<colibry::ORBManager> orb;
}

// HELPER
ostream& operator<<(ostream& os, const CS::Tuple& t);

// -----------------------------------------------------
// TupleSpaceImpl
// -----------------------------------------------------

CS::TupleSpaceImpl::TupleSpaceImpl (string id)
	: m_id{std::move(id)}
{
}

template<typename T>
string str(const T& x) {
	ostringstream ss;
	ss << x;
	return ss.str();
}

// OUT
// -----------------------------------------------------
void CS::TupleSpaceImpl::out (const ::CSpace::Tuple & tp)
{
	debug("out {}", str(tp));

	// no wildcards allowed - tp must be a tuple and not an anti-tuple
	// items of kind tk_null are considered wildcards
	const CORBA::ULong N = tp.length();
	for (CORBA::ULong i=0; i<N; i++)
		if (tp[i].type()->kind() == CORBA::tk_null)
			throw CS::InvalidTuple();

	// tp doesn't contain any wildcards

	{
		lock_guard<mutex> g{mutex_};

		if (FindTuple(tp) != end(m_tuples)) {
			// found - error - no duplicate tuples are allowed
			debug("duplicate tuple!");
			throw CS::InvalidTuple();
		}
		// Tuple not found: Add tuple to space
		m_tuples.push_back(tp);
		debug("inserted");
	}

	// check if someone needs to be waken up
	// wake up all pending for that tuple
	while (true) {
		lock_guard<mutex> g{mutex_pending_};
		auto p = FindPendingTuple(tp);
		if (p == end(m_pending))
			break;	// no pending tuple for tp
		auto s = p->s;
		m_pending.erase(p); // remove from pending
		debug("wakeup pending");
		s->unlock();
	}
}

// IN
// -----------------------------------------------------
void CS::TupleSpaceImpl::in (::CSpace::Tuple & tp)
{
	debug("in {}", str(tp));
	
	// block if not available

	while (true) {
		{
			lock_guard<mutex> g{mutex_};
			auto t = FindTuple(tp);
			if (t != end(m_tuples)) {
				debug("found");
				tp = *t;
				m_tuples.erase(t);   // in operation is destructive
				break;
			}
		}
		debug("pending");
		{
			lock_guard<mutex> g{mutex_pending_};
			// add to pending list and block
			m_pending.push_back({&tp, make_shared<mutex>()});
			m_pending.back().s->lock(); // needed, so next will sleep
		}
		debug("    locking");
		m_pending.back().s->lock(); // = sleep on the semaphore in the vector
		// out will remove from pending
	}
}

// RD
// -----------------------------------------------------
void CS::TupleSpaceImpl::rd (::CSpace::Tuple & tp)
{
	debug("rd {}", str(tp));

	while (true) {
		{
			lock_guard<mutex> g{mutex_};
			auto t = FindTuple(tp);
			if (t != end(m_tuples)) {
				debug("found");
				tp = *t;
				break;
			}
		}
		// add to pending list and block
		debug("pending");
		{
			lock_guard<mutex> g{mutex_pending_};
			m_pending.push_back({&tp, make_shared<mutex>()});
			m_pending.back().s->lock(); // needed, so next will sleep
		}
		debug("locking");
		m_pending.back().s->lock(); // = sleep on the semaphore in the vector
		// out will remove from pending
	}
}

// INP
// -----------------------------------------------------
::CORBA::Boolean CS::TupleSpaceImpl::inp (::CSpace::Tuple & tp)
{
	debug("inp {}", str(tp));

	lock_guard<mutex> g{mutex_};	
	auto t = FindTuple(tp);
	if (t != end(m_tuples)) {
		tp = *t;
		m_tuples.erase(t); // remove from space
		return true;
	}
	return false;
}

// RDP
// -----------------------------------------------------
::CORBA::Boolean CS::TupleSpaceImpl::rdp (::CSpace::Tuple & tp)
{
	debug("rdp {}", str(tp));

	lock_guard<mutex> g{mutex_};	
	auto t = FindTuple(tp);
	if (t != end(m_tuples)) {
		tp = *t;
		return true;
	}
	return false;
}

// HELPERS
// -----------------------------------------------------

bool CS::TupleSpaceImpl::IsEqual(const ::CORBA::Any& t1,
				 const ::CORBA::Any& t2)
{
	if (t1.type()->kind() == CORBA::tk_null || t2.type()->kind() == CORBA::tk_null) {
		// one of them is a wild card, so it matches
		return true;
	}
	
	// Check type - TO TEST: I think it's checking data also...
	// equivalent: subtype equivalence
	// equal: absolute equality
	if (!t1.type()->equal(t2.type()))
		return false;    // different types

	// Check data
	// Compare basic types only
	// (yet incomplete)
	switch (t1.type()->kind()) {
	case CORBA::tk_short: {          // Short
		CORBA::Short x1, x2;
		t1 >>= x1;
		t2 >>= x2;
		return (bool)(x1 == x2); }
	case CORBA::tk_long: {           // Long
		CORBA::Long x1, x2;
		t1 >>= x1;
		t2 >>= x2;
		return (bool)(x1 == x2); }
	case CORBA::tk_ushort: {          // UShort
		CORBA::UShort x1, x2;
		t1 >>= x1;
		t2 >>= x2;
		return (bool)(x1 == x2); }
	case CORBA::tk_ulong: {           // ULong
		CORBA::ULong x1, x2;
		t1 >>= x1;
		t2 >>= x2;
		return (bool)(x1 == x2); }
	case CORBA::tk_float: {          // Float
		CORBA::Float x1, x2;
		t1 >>= x1;
		t2 >>= x2;
		return (bool)(x1 == x2); }
	case CORBA::tk_double: {         // Double
		CORBA::Double x1, x2;
		t1 >>= x1;
		t2 >>= x2;
		return (bool)(x1 == x2); }
	case CORBA::tk_string: {         // String
		const char *x1;
		const char *x2;
		t1 >>= x1;
		t2 >>= x2;
		return (bool)(strcmp(x1,x2)==0); }
	case CORBA::tk_boolean: {        // Boolean
		// boolean, octet and char are not distinguishable, so the need of to_
		CORBA::Boolean x1,x2;
		t1 >>= CORBA::Any::to_boolean(x1);
		t2 >>= CORBA::Any::to_boolean(x2);
		return (bool)(x1 == x2); }
	case CORBA::tk_octet: {          // Octet
		CORBA::Octet x1,x2;
		t1 >>= CORBA::Any::to_octet(x1);
		t2 >>= CORBA::Any::to_octet(x2);
		return (bool)(x1 == x2); }
	case CORBA::tk_char: {           // Char
		CORBA::Char x1,x2;
		t1 >>= CORBA::Any::to_char(x1);
		t2 >>= CORBA::Any::to_char(x2);
		return (bool)(x1 == x2); }
	default:
		return false;
	}

	return true;
}


CS::TVIterator CS::TupleSpaceImpl::FindTuple(const ::CSpace::Tuple & antituple)
{
	// find 1st tuple that match non-wildcard fields

	// find and do something in an atomic way. So, no locking here!
	// lock_guard<mutex> g{mutex_};
	for (auto ti = m_tuples.begin(); ti != m_tuples.end(); ++ti) {
		CORBA::ULong tsz = ti->length();
		if (tsz != antituple.length()) // tuples of diferent sizes are not equal
			continue;
		CORBA::ULong j = 0;
		for (; j<tsz; ++j) {
			if (antituple[j].type()->kind() != CORBA::tk_null) {
				// this is not a wildcard - check type + value
				if (!IsEqual((*ti)[j],antituple[j]))
					break;
			}
		}
		if (j == tsz)
			return ti;
	}
	
	// if (!found)
	// 	throw NotFoundExc();
	
	return m_tuples.end();
}

CS::PVIterator CS::TupleSpaceImpl::FindPendingTuple(const CS::Tuple& tuple)
{
	// tuple doesn't contain any wildcards
	// find and do something in an atomic way. So, no locking here!
	// lock_guard<mutex> g{mutex_pending_};
	for (auto ti = m_pending.begin(); ti != m_pending.end(); ++ti) {
		CORBA::ULong tsz = ti->t->length();
		if (tsz != tuple.length())
			continue; 	// skip comparison of tuples of different sizes
		CORBA::ULong j=0;
		for (; j<tsz; ++j)
			if ((*ti->t)[j].type()->kind() != CORBA::tk_null)
				if (!IsEqual((*(ti->t))[j],tuple[j]))
					break;
		if (j == tsz)
			return ti;     // all elements are equal
	}

	// if (!found)
	// 	throw NotFoundExc();

	return m_pending.end();
}

//
// FACTORY
//

CS::TupleSpaceFactoryImpl::TupleSpaceFactoryImpl (PortableServer::POA_ptr poa)
	: m_poa(PortableServer::POA::_duplicate(poa))
{
}

::CSpace::TupleSpace_ptr CS::TupleSpaceFactoryImpl::get (const char * id)
{
	// Create or return existing TupleSpace
	PortableServer::ObjectId_var oid = PortableServer::string_to_ObjectId(id);

	CORBA::Object_ptr tmp_ref = CORBA::Object::_nil();
	CS::TupleSpace_ptr tspace = CS::TupleSpace::_nil();

	try {
		// Tenta achar conta no Active Object Map do poa
		tmp_ref = m_poa->id_to_reference(oid.in());
		// cout << "Objeto existe!" << endl;
	} catch (PortableServer::POA::ObjectNotActive&) {
		// Objeto nao encontrado no Object Active Map - cria novo
		auto* ts_i = new CS::TupleSpaceImpl(id);
		m_poa->activate_object_with_id(oid.in(),ts_i);
		tmp_ref = m_poa->id_to_reference(oid.in());
		// cout << "Objeto criado!" << endl;
	}

	tspace = CS::TupleSpace::_narrow(tmp_ref);
	return CS::TupleSpace::_duplicate(tspace);
}

void CS::TupleSpaceFactoryImpl::shutdown ()
{
	global::orb->shutdown();
}

PortableServer::POA_ptr CS::TupleSpaceFactoryImpl::_default_POA()
{
	return PortableServer::POA::_duplicate(m_poa.in());
}

ostream& operator<<(ostream& os, const CS::Tuple& t)
{
	os << "<";
	CORBA::ULong len = t.length();
	for (CORBA::ULong i=0; i<len; i++) {
		switch (t[i].type()->kind()) {
		case CORBA::tk_null:  // wildcard
			os << "*";
			break;
		case CORBA::tk_short: // Short
			CORBA::Short x;
			t[i] >>= x;
			os << x;
			break;
		case CORBA::tk_float: // Float
			CORBA::Float y;
			t[i] >>= y;
			os << y;
			break;
		case CORBA::tk_string: // String
			const char* s;
			t[i] >>= s;
			os << "\"" << s << "\"";
			break;
		default:
			os << "unknown";
		}
		if (i < len-1) // not the last?
			os << ",";
	}    
	os << ">";
	return os;
}

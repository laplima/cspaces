#ifndef TUPLESPACEIMPL_H
#define TUPLESPACEIMPL_H

// #include <ace/Thread_Mutex.h>
#include <mutex>
#include "TupleSpaceS.h"
#include <vector>
#include <string>
#include <memory>

namespace CSpace {

	class NotFoundExc {};

	using TupleVec = std::vector<::CSpace::Tuple>;
	using TVIterator = TupleVec::iterator;

	struct PendingItem {
		Tuple* t;
		std::shared_ptr<std::mutex> s;	// unique_ptr doesn't work
	};

	using PendingVec = std::vector<PendingItem>;
	using PVIterator = PendingVec::iterator;

	class  TupleSpaceImpl : public virtual POA_CSpace::TupleSpace {
	public:
		TupleSpaceImpl (std::string id);
		~TupleSpaceImpl() override = default;
		// IDL operations
		void in (::CSpace::Tuple & tp) override;
		void out (const ::CSpace::Tuple & tp) override;
		void rd (::CSpace::Tuple & tp) override;
		::CORBA::Boolean inp (::CSpace::Tuple & tp) override;  
		::CORBA::Boolean rdp (::CSpace::Tuple & tp) override;
	protected:
		TVIterator FindTuple(const ::CSpace::Tuple & antituple);
		PVIterator FindPendingTuple(const Tuple& antituple);
		static bool IsEqual(const ::CORBA::Any& ti1,
				const ::CORBA::Any& ti2);
	private:
		std::string m_id;
		TupleVec    m_tuples;
		PendingVec  m_pending;   	// pending tuples + semaphores
		std::mutex mutex_;			// controls access to m_tuples
		std::mutex mutex_pending_;	// controls access to m_pending
	};


	class  TupleSpaceFactoryImpl : public virtual POA_CSpace::TupleSpaceFactory {
	public:
		TupleSpaceFactoryImpl (PortableServer::POA_ptr poa);
		~TupleSpaceFactoryImpl () override = default;
		PortableServer::POA_ptr _default_POA() override;
		// IDL
		::CSpace::TupleSpace_ptr get (const char * id) override;  
		void shutdown() override;
	private:
		PortableServer::POA_var m_poa;
	};

}; // namespace


#endif /* TUPLESPACEIMPL_H_  */

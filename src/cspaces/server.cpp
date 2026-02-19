#include <iostream>
#include <print>
#include <format>
#include <orbsvcs/CosNamingC.h>
#include <signal.h>
#include <spdlog/common.h>
#include <string>
#include <cstdlib>
#include <colibry/TextTools.h>
#include <colibry/ORBManager.h>
#include <colibry/NameServer.h>
#include <span>
#include <spdlog/spdlog.h>
#include <memory>
#include "ThreadPool.h"
#include "TupleSpaceI.h"

using namespace std;
using namespace CSpace;
using namespace colibry;
using spdlog::error;

constexpr size_t THREADPOOLSZ = 10;        // number of concurrent threads
constexpr const char * const DFLT_FACNAME = "TSFac";

namespace global {
	unique_ptr<ORBManager> orb;
	unique_ptr<NameServer> ns;
}

void TerminationHandler(int nsig);

constexpr int  DFLT_TAB = 40;
/*#define ABSTAB(t)                               \
    (char)0x1b << (char)0x5b << 80 << 'D'    \
    << (char)0x1b << (char)0x5b << t << 'C'*/

int main(int argc, char* argv[])
{
	// spdlog::set_level(spdlog::level::debug);
	spdlog::set_level(spdlog::level::info);

    println("CSPACES (C) Luiz Lima Jr.");

    // Parse command line parameters
    unsigned short nthreads = THREADPOOLSZ;
    const char* facname = nullptr;

    span args(argv, argc);
    for (unsigned short i=1; i<argc; i++) {
		if (args[i] == "-h"s) {
		    println("USAGE: {} [-h] [-t <N>] [<facname>]\n"
		    	"\t-h\t: help\n"
		    	"\t-t <N>\t: # of threads", args[0]);
		    return 0;
		}
		if (args[i] == "-t"s) {
		    if (args.size()<i+1)
				nthreads = stoi(args[i]);
		} else
		    facname = args[i];
    }
    if (facname == nullptr)
		facname = DFLT_FACNAME;

    try {

		print("* Starting up... ");

		global::orb = make_unique<ORBManager>(argc, argv);
		global::orb->activate_rootpoa();

		auto& rpoa = global::orb->rootpoa();
		auto child_poa = rpoa.create_child_poa("CPOA", {POAPolicy::USER_ID,
			POAPolicy::NO_IMPLICIT_ACTIVATION});

		// Signal handling
		for (int i=1; i<32; i++)
		    signal(i,TerminationHandler);

		println("{}[OK]", ABSTAB(DFLT_TAB));

		print("* Registering \"{}\" in NS... ", facname);

		// Instantiate factory servant
		TupleSpaceFactoryImpl fac_i(child_poa.poa());

		// Get IOR
		TupleSpaceFactory_var fac = global::orb->activate_object<TupleSpaceFactory>(fac_i);

		// Export IOR
		global::ns = make_unique<NameServer>(*global::orb);
		global::ns->rebind(facname, fac.in());

		println("{}[OK]", ABSTAB(DFLT_TAB));

		print("* Starting thread pool ({})... ", nthreads);
		ThreadPool tpool{*global::orb, static_cast<unsigned short>(nthreads-1)};
		println("{}[OK]", ABSTAB(DFLT_TAB));

		println("* Waiting for requests...");

		// run orb
		global::orb->run();

		print("* Waiting for termination of all threads... ");
		tpool.join();
		println("{}[OK]", ABSTAB(DFLT_TAB));

		// cleanup
		print("* Cleaning up... ");
		// child_poa->destroy(true,true);
		// root_poa->destroy(true,true);
		// global_orb->destroy();
		println("{}[OK]", ABSTAB(DFLT_TAB));

    } catch (CORBA::Exception& e) {
    	ostringstream ss;
    	ss << e;
		error("CORBA exception: {}", ss.str());
    }

    return 0;
}

void TerminationHandler(int nsig)
{
    // don´t know why it sometimes block...
    /*try {
      println("* Unbinding \"" << global_tsname[0].id << "\"... " << flush;
      global_ns->unbind(global_tsname);
      println(ABSTAB(DFLT_TAB) << "[OK]" << endl;
      } catch (CORBA::Exception&) {
      cerr << "ERROR" << endl;
      }*/
    global::orb->shutdown();
}

